/*****************************************************************

Copyright (c) 1996-2000 the kicker authors. See file AUTHORS.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/


#include <QBitmap>
#include <QCursor>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>
#include <QEvent>
#include <QFile>
#include <QFontMetrics>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QPixmap>
#include <QPoint>
#include <QResizeEvent>
#include <QStyle>
#include <QStyleOption>
#include <QToolTip>


#include <kapplication.h>
#include <kconfig.h>
#include <kcursor.h>
#include <kdialog.h>
#include <kdirwatch.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <kicontheme.h>
#include <kipc.h>
#include <kstandarddirs.h>
#include <klocale.h>

#include "utils.h"

#include "kshadowengine.h"
#include "kshadowsettings.h"

#include "kickerSettings.h"
#include "panelbutton.h"
#include "panelbutton.moc"

class PanelButton::Private
{
public:
    Private()
     : isValid(true),
       isLeftMouseButtonDown(false),
       drawArrow(false),
       highlight(false),
       changeCursorOverItem(true),
       hasAcceptedDrag(false),
       arrowDirection(Plasma::Up),
       popupDirection(Plasma::Up),
       orientation(Qt::Horizontal),
       size((K3Icon::StdSizes)-1),
       fontPercent(0.40)
    {}

    QPoint lastLeftMouseButtonPress;
    bool isValid;
    bool isLeftMouseButtonDown;
    bool drawArrow;
    bool highlight;
    bool changeCursorOverItem;
    bool hasAcceptedDrag;
    QColor textColor;
    QString buttonText;
    QString title;
    QString iconName;
    QString backingFile;
    QPixmap up;
    QPixmap down;
    QPixmap icon;
    QPixmap iconh; // hover
    QPixmap iconz; // mouse over
    Plasma::Position arrowDirection;
    Plasma::Position popupDirection;
    Qt::Orientation orientation;
    int size;
    double fontPercent;
};

// init static variable
KShadowEngine* PanelButton::s_textShadowEngine = 0L;

PanelButton::PanelButton( QWidget* parent, const char* name )
    : QAbstractButton(parent, name),
      d(new Private)
{
    KGlobal::locale()->insertCatalog("libkicker");
    calculateIconSize();
    setAcceptDrops(true);

    d->textColor = KGlobalSettings::textColor();

    updateSettings(KApplication::SETTINGS_MOUSE);

    kapp->addKipcEventMask(KIPC::SettingsChanged | KIPC::IconChanged);

    installEventFilter(KickerTip::self());

    connect(kapp, SIGNAL(settingsChanged(int)), SLOT(updateSettings(int)));
    connect(kapp, SIGNAL(iconChanged(int)), SLOT(updateIcon(int)));
}

PanelButton::~PanelButton()
{
   delete d; 
}

void PanelButton::setDrawArrow(bool drawArrow)
{
    if (d->drawArrow == drawArrow)
    {
        return;
    }

    d->drawArrow = drawArrow;
    update();
}

void PanelButton::setEnabled(bool enable)
{
    QAbstractButton::setEnabled(enable);
    loadIcons();
    update();
}

void PanelButton::setPopupDirection(Plasma::Position popupDirection)
{
    d->popupDirection = popupDirection;
    setArrowDirection(popupDirection);
}

void PanelButton::setOrientation(Qt::Orientation o)
{
    d->orientation = o;
}

void PanelButton::updateIcon(int group)
{
    if (group != K3Icon::Panel)
    {
        return;
    }

    loadIcons();
    update();
}

void PanelButton::updateSettings(int category)
{
    if (category != KApplication::SETTINGS_MOUSE)
    {
        return;
    }

    d->changeCursorOverItem = KGlobalSettings::changeCursorOverIcon();

    if (d->changeCursorOverItem)
    {
        setCursor(KCursor::handCursor());
    }
    else
    {
        unsetCursor();
    }
}

void PanelButton::checkForDeletion(const QString& path)
{
    if (path == d->backingFile)
    {
        setEnabled(false);
        QTimer::singleShot(1000, this, SLOT(scheduleForRemoval()));
    }
}

bool PanelButton::checkForBackingFile()
{
    return QFile::exists(d->backingFile);
}

void PanelButton::scheduleForRemoval()
{
    static int timelapse = 1000;
    if (checkForBackingFile())
    {
        setEnabled(true);
        timelapse = 1000;
        emit hideme(false);
        return;
    }
    else if (KickerSettings::removeButtonsWhenBroken())
    {
        if (timelapse > 255*1000) // we'v given it ~8.5 minutes by this point
        {
            emit removeme();
            return;
        }

        if (timelapse > 3000 && isVisible())
        {
            emit hideme(true);
        }

        timelapse *= 2;
        QTimer::singleShot(timelapse, this, SLOT(scheduleForRemoval()));
    }
}

// return the dimension that the button wants to be for a given panel dimension (panelDim)
int PanelButton::preferredDimension(int panelDim) const
{
    // determine the upper limit on the size.  Normally, this is panelDim,
    // but if conserveSpace() is true, we restrict size to comfortably fit the icon
    if (KickerSettings::conserveSpace())
    {
        int newSize = preferredIconSize(panelDim);
        if (newSize > 0)
        {
            return qMin(panelDim, newSize + (KDialog::spacingHint() * 2));
        }
    }

    return panelDim;
}

int PanelButton::widthForHeight(int height) const
{
    int rc = preferredDimension(height);

    // we only paint the text when horizontal, so make sure we're horizontal
    // before adding the text in here
    if (orientation() == Qt::Horizontal && !d->buttonText.isEmpty())
    {
        QFont f(font());
        f.setPixelSize(qMin(height, qMax(int(float(height) * d->fontPercent), 16)));
        QFontMetrics fm(f);

        rc += fm.width(d->buttonText) + qMin(25, qMax(5, fm.width('m') / 2));
    }

    return rc;
}

int PanelButton::heightForWidth(int width) const
{
    return preferredDimension(width);
}

const QPixmap& PanelButton::labelIcon() const
{
    return d->highlight ? d->iconh : d->icon;
}

const QPixmap& PanelButton::zoomIcon() const
{
    return d->iconz;
}

bool PanelButton::isValid() const
{
    return d->isValid;
}

void PanelButton::setIsValid(bool valid)
{
   d->isValid = valid;
}

void PanelButton::setIconPixmap(const QPixmap &icon)
{
    d->icon = icon;
}

void PanelButton::setTitle(const QString& t)
{
    d->title = t;
}

void PanelButton::setIcon(const QString& icon)
{
    if (icon == d->iconName)
    {
        return;
    }

    d->iconName = icon;
    loadIcons();
    update();
    emit iconChanged();
}

QString PanelButton::icon() const
{
    return d->iconName;
}

bool PanelButton::hasText() const
{
    return !d->buttonText.isEmpty();
}

void PanelButton::setButtonText(const QString& text)
{
    d->buttonText = text;
    update();
}

QString PanelButton::buttonText() const
{
    return d->buttonText;
}

void PanelButton::setTextColor(const QColor& c)
{
    d->textColor = c;
}

QColor PanelButton::textColor() const
{
    return d->textColor;
}

void PanelButton::setFontPercent(double p)
{
    d->fontPercent = p;
}

double PanelButton::fontPercent() const
{
    return d->fontPercent;
}

Qt::Orientation PanelButton::orientation() const
{
    return d->orientation;
}

Plasma::Position PanelButton::popupDirection() const
{
    return d->popupDirection;
}

QPoint PanelButton::center() const
{
    return mapToGlobal(rect().center());
}

QString PanelButton::title() const
{
    return d->title;
}

void PanelButton::triggerDrag()
{
    setDown(false);

    startDrag();
}

void PanelButton::startDrag()
{
    emit dragme(d->icon);
}

void PanelButton::enterEvent(QEvent* e)
{
    if (!d->highlight)
    {
        d->highlight = true;
        repaint(false);
    }

    QAbstractButton::enterEvent(e);
}

void PanelButton::leaveEvent(QEvent* e)
{
    if (d->highlight)
    {
        d->highlight = false;
        repaint(false);
    }

    QAbstractButton::leaveEvent(e);
}

void PanelButton::dragEnterEvent(QDragEnterEvent* e)
{
    if (e->isAccepted())
    {
        d->hasAcceptedDrag = true;
    }

    update();
    QAbstractButton::dragEnterEvent( e );
}

void PanelButton::dragLeaveEvent(QDragLeaveEvent* e)
{
    d->hasAcceptedDrag = false;
    update();
    QAbstractButton::dragLeaveEvent( e );
}

void PanelButton::dropEvent(QDropEvent* e)
{
    d->hasAcceptedDrag = false;
    update();
    QAbstractButton::dropEvent( e );
}

void PanelButton::mouseMoveEvent(QMouseEvent *e)
{
    if (!d->isLeftMouseButtonDown || (e->state() & Qt::LeftButton) == 0)
    {
        return;
    }

    QPoint p(e->pos() - d->lastLeftMouseButtonPress);
    if (p.manhattanLength() <= 16)
    {
        // KGlobalSettings::dndEventDelay() is not enough!
        return;
    }

    d->isLeftMouseButtonDown = false;
    triggerDrag();
}

void PanelButton::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton)
    {
        d->lastLeftMouseButtonPress = e->pos();
        d->isLeftMouseButtonDown = true;
    }
    QAbstractButton::mousePressEvent(e);
}

void PanelButton::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton)
    {
        d->isLeftMouseButtonDown = false;
    }
    QAbstractButton::mouseReleaseEvent(e);
}

void PanelButton::resizeEvent(QResizeEvent*)
{
    if (calculateIconSize())
    {
        loadIcons();
    }
}

void PanelButton::paintEvent( QPaintEvent * )
{
    QPainter p(this);
    drawButton( &p );
}

void PanelButton::drawButton(QPainter *p)
{
    QStyleOptionFrame opt;
    opt.init(this);
    opt.lineWidth = 1;
    opt.midLineWidth = 0;
    opt.state = QStyle::State_Enabled;


    if (isDown() || isChecked())
    {
        // Draw shapes to indicate the down state.
        opt.state |= QStyle::State_Sunken;
        style()->drawPrimitive(QStyle::PE_Frame, &opt, p, this);
    }

    drawButtonLabel(p);

    if (hasFocus() || d->hasAcceptedDrag)
    {
        int x1, y1, x2, y2;
        rect().getCoords(&x1, &y1, &x2, &y2);
        QRect r(x1+2, y1+2, x2-x1-3, y2-y1-3);
        QStyleOptionFocusRect focusOpt;
        focusOpt.init(this);
        focusOpt.backgroundColor = palette().button();
        focusOpt.rect            = r;
        style()->drawPrimitive(QStyle::PE_FrameFocusRect, &focusOpt, p, this);
    }
}

void PanelButton::drawButtonLabel(QPainter *p)
{
    QPixmap icon = labelIcon();
    bool active = isDown() || isChecked();

    if (active)
    {
        icon = icon.toImage().scaled(icon.width() - 2, icon.height() - 2, 
			Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }

    if (!d->buttonText.isEmpty() && orientation() == Qt::Horizontal)
    {
        int h = height();
        int w = width();
        int y = (h - icon.height())/2;
        p->save();
        QFont f = font();

        double fontPercent = d->fontPercent;
        if (active)
        {
            fontPercent *= .8;
        }
        f.setPixelSize(qMin(h, qMax(int(float(h) * d->fontPercent), 16)));
        QFontMetrics fm(f);
        p->setFont(f);

        /* Draw shadowed text */
        Qt::LayoutDirection rtl = layoutDirection();
        bool reverse = (rtl == Qt::RightToLeft);

        if (!reverse && !icon.isNull())
        {
            /* Draw icon */
            p->drawPixmap(3, y, icon);
        }

        int tX = reverse ? 3 : icon.width() + qMin(25, qMax(5, fm.width('m') / 2));
        int tY = fm.ascent() + ((h - fm.height()) / 2);

        // get a transparent pixmap
        QPainter pixPainter;
        QPixmap textPixmap(w, h);

        textPixmap.fill(QColor(0,0,0));
        textPixmap.setMask(textPixmap.createHeuristicMask(true));

        // draw text
        pixPainter.begin(&textPixmap);
        pixPainter.setPen(m_textColor);
        pixPainter.setFont(p->font()); // get the font from the root painter
        pixPainter.drawText(tX, tY, d->buttonText, -1, rtl);
        pixPainter.end();

        if (!s_textShadowEngine)
        {
            KShadowSettings* shadset = new KShadowSettings();
            shadset->setOffsetX(0);
            shadset->setOffsetY(0);
            shadset->setThickness(1);
            shadset->setMaxOpacity(96);
            s_textShadowEngine = new KShadowEngine(shadset);
        }

        // draw shadow
        QColor shadCol = Plasma::shadowColor(d->textColor);
        QImage img = s_textShadowEngine->makeShadow(textPixmap, shadCol);
        p->drawImage(0, 0, img);
        p->save();
        pixPainter.setPen(m_textColor);
        p->drawText(tX, tY, d->buttonText, -1, rtl);
        p->restore();

        if (reverse && !icon.isNull())
        {
            p->drawPixmap(w - icon.width() - 3, y, icon);
        }

        p->restore();
    }
    else if (!icon.isNull())
    {
        int y = (height() - icon.height())/2;
        int x = (width()  - icon.width() )/2;
        p->drawPixmap(x, y, icon);
    }

    if (d->drawArrow)
    {
        QStyle::PrimitiveElement e = QStyle::PE_IndicatorArrowUp;
        QRect r(0, 0, 8, 8);

        switch (d->arrowDirection)
        {
            case Plasma::Top:
                e = QStyle::PE_IndicatorArrowUp;
                break;
            case Plasma::Bottom:
                e = QStyle::PE_IndicatorArrowDown;
                r.translate(0, height() - 8);
                break;
            case Plasma::Right:
                e = QStyle::PE_IndicatorArrowRight;
                r.translate(width() - 8 , 0);
                break;
            case Plasma::Left:
                e = QStyle::PE_IndicatorArrowLeft;
                break;
            case Plasma::Floating:
                if (orientation() == Qt::Horizontal)
                {
                    e = QStyle::PE_IndicatorArrowDown;
                }
                else if (QApplication::isRightToLeft())
                {
                    e = QStyle::PE_IndicatorArrowLeft;
                }
                else
                {
                    e = QStyle::PE_IndicatorArrowRight;
                }
                break;
        }

        QStyle::State flags = QStyle::State_Enabled;
        if (isDown() || isChecked())
        {
            flags |= QStyle::State_Sunken;
        }
        QStyleOption opt;
        opt.init(this);
        opt.rect  = r;
        opt.state = flags;
        style()->drawPrimitive(e, &opt, p, this);
    }
}

// return the icon size that would be used if the panel were proposed_size
// if proposed_size==-1, use the current panel size instead
int PanelButton::preferredIconSize(int proposed_size) const
{
    // (re)calculates the icon sizes and report true if they have changed.
    // Get sizes from icontheme. We assume they are sorted.
    KIconTheme *ith = KGlobal::iconLoader()->theme();

    if (!ith)
    {
        return -1; // unknown icon size
    }

    QList<int> sizes = ith->querySizes(K3Icon::Panel);

    int sz = ith->defaultSize(K3Icon::Panel);

    if (proposed_size < 0)
    {
        proposed_size = (orientation() == Qt::Horizontal) ? height() : width();
    }

    // determine the upper limit on the size.  Normally, this is panelSize,
    // but if conserve space is requested, the max button size is used instead.
    int upperLimit = proposed_size;
    if (proposed_size > Plasma::maxButtonDim() &&
        KickerSettings::conserveSpace())
    {
        upperLimit = Plasma::maxButtonDim();
    }

    //kDebug()<<endl<<endl<<flush;
    QList<int>::const_iterator it = sizes.constBegin();
    QList<int>::const_iterator itEnd = sizes.constEnd();
    while (it != itEnd)
    {
        if ((*it) + (2 * KickerSettings::iconMargin()) > upperLimit)
        {
            break;
        }
        sz = *it;   // get the largest size under the limit
        ++it;
    }

    //kDebug()<<"Using icon sizes: "<<sz<<"  "<<zood->sz<<endl<<flush;
    return sz;
}

void PanelButton::backedByFile(const QString& localFilePath)
{
    d->backingFile = localFilePath;

    if (d->backingFile.isEmpty())
    {
        return;
    }

    // avoid multiple connections
    disconnect(KDirWatch::self(), SIGNAL(deleted(const QString&)),
               this, SLOT(checkForDeletion(const QString&)));

    if (!KDirWatch::self()->contains(d->backingFile))
    {
        KDirWatch::self()->addFile(d->backingFile);
    }

    connect(KDirWatch::self(), SIGNAL(deleted(const QString&)),
            this, SLOT(checkForDeletion(const QString&)));

}

void PanelButton::setArrowDirection(Plasma::Position dir)
{
    if (d->arrowDirection != dir)
    {
        d->arrowDirection = dir;
        update();
    }
}

void PanelButton::loadIcons()
{
    KIconLoader * ldr = KGlobal::iconLoader();
    QString nm = d->iconName;
    K3Icon::States defaultState = isEnabled() ? K3Icon::DefaultState :
                                               K3Icon::DisabledState;
    d->icon = ldr->loadIcon(nm, K3Icon::Panel, d->size, defaultState, 0L, true);

    if (d->icon.isNull())
    {
        nm = defaultIcon();
        d->icon = ldr->loadIcon(nm, K3Icon::Panel, d->size, defaultState);
    }

    if (!isEnabled())
    {
        d->iconh = d->icon;
    }
    else
    {
        d->iconh = ldr->loadIcon(nm, K3Icon::Panel, d->size,
                                K3Icon::ActiveState, 0L, true);
    }

    d->iconz = ldr->loadIcon(nm, K3Icon::Panel, K3Icon::SizeHuge,
                            defaultState, 0L, true );
}

// (re)calculates the icon sizes and report true if they have changed.
//      (false if we don't know, because theme couldn't be loaded?)
bool PanelButton::calculateIconSize()
{
    int size = preferredIconSize();

    if (size < 0)
    {
        // size unknown
        return false;
    }

    if (d->size != size)
    {
        // Size has changed, update
        d->size = size;
        return true;
    }

    return false;
}

void PanelButton::updateTipData(KickerTip::Data& data)
{
    data.message = title();
    data.subtext = toolTip();
    data.icon = zoomIcon();
    data.direction = popupDirection();
}

//
// PanelPopupButton class
//

class PanelPopupButton::Private
{
public:
    Private()
      : popup(0),
        pressedDuringPopup(false),
        initialized(false)
    {}

    QMenu *popup;
    bool pressedDuringPopup;
    bool initialized;
};

PanelPopupButton::PanelPopupButton(QWidget *parent, const char *name)
  : PanelButton(parent, name),
    d(new Private)
{
    setDrawArrow(true);
    connect(this, SIGNAL(pressed()), SLOT(slotExecMenu()));
}

PanelPopupButton::~PanelPopupButton()
{
    delete d;
}

void PanelPopupButton::setPopup(QMenu *popup)
{
    if (d->popup)
    {
        d->popup->removeEventFilter(this);
    }

    d->popup = popup;

    if (popup)
    {
        d->popup->installEventFilter(this);
    }
}

QMenu *PanelPopupButton::popup() const
{
    return d->popup;
}

bool PanelPopupButton::eventFilter(QObject *, QEvent *e)
{
    if (e->type() == QEvent::MouseMove)
    {
        QMouseEvent *me = static_cast<QMouseEvent *>(e);
        if (rect().contains(mapFromGlobal(me->globalPos())) &&
            ((me->state() & Qt::ControlModifier) != 0 ||
             (me->state() & Qt::ShiftModifier) != 0))
        {
            PanelButton::mouseMoveEvent(me);
            return true;
        }
    }
    else if (e->type() == QEvent::MouseButtonPress ||
             e->type() == QEvent::MouseButtonDblClick)
    {
        QMouseEvent *me = static_cast<QMouseEvent *>(e);
        if (rect().contains(mapFromGlobal(me->globalPos())))
        {
            d->pressedDuringPopup = true;
            return true;
        }
    }
    else if (e->type() == QEvent::MouseButtonRelease)
    {
        QMouseEvent *me = static_cast<QMouseEvent *>(e);
        if (rect().contains(mapFromGlobal(me->globalPos())))
        {
            if (d->pressedDuringPopup && d->popup)
            {
                d->popup->hide();
            }
            return true;
        }
    }
    return false;
}

void PanelPopupButton::slotExecMenu()
{
    if (!d->popup)
    {
        return;
    }

    d->pressedDuringPopup = false;
    KickerTip::enableTipping(false);
    kapp->syncX();
    kapp->processEvents();

    if (!d->initialized)
    {
        initPopup();
    }

    d->popup->adjustSize();
    d->popup->exec(Plasma::popupPosition(popupDirection(), d->popup, this));
    setDown(false);
    KickerTip::enableTipping(true);
}

void PanelPopupButton::triggerDrag()
{
    if (d->popup)
    {
        d->popup->hide();
    }

    PanelButton::triggerDrag();
}

void PanelPopupButton::setInitialized(bool initialized)
{
    d->initialized = initialized;
}

