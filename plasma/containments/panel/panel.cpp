/*
*   Copyright 2007 by Alex Merry <huntedhacker@tiscali.co.uk>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Library General Public License version 2,
*   or (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details
*
*   You should have received a copy of the GNU Library General Public
*   License along with this program; if not, write to the
*   Free Software Foundation, Inc.,
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "panel.h"

#include <QApplication>
#include <QPainter>
#include <QDesktopWidget>
#include <QGridLayout>
#include <QLabel>
#include <QComboBox>
#include <QAction>

#include <KDebug>
#include <KIcon>
#include <KDialog>
#include <KIntNumInput>

#include <plasma/corona.h>
#include <plasma/layouts/layout.h>
#include <plasma/svgpanel.h>
#include <plasma/theme.h>

using namespace Plasma;

Panel::Panel(QObject *parent, const QVariantList &args)
    : Containment(parent, args),
      m_dialog(0),
      m_appletBrowserAction(0),
      m_configureAction(0)
{
    m_background = new Plasma::SvgPanel("widgets/panel-background", this);
    m_background->setBorderFlags(Plasma::SvgPanel::DrawAllBorders);
    connect(m_background, SIGNAL(repaintNeeded()), this, SLOT(backgroundChanged()));
    setZValue(150);
    setContainmentType(Containment::PanelContainment);

    connect(Plasma::Theme::self(), SIGNAL(changed()), this, SLOT(themeUpdated()));
    themeUpdated();
}

Panel::~Panel()
{
    delete m_dialog;
}

void Panel::init()
{
    KConfigGroup cg = config();
    QRectF geo = cg.readEntry("geometry", QRectF());
    Plasma::Location loc = (Plasma::Location)cg.readEntry("location", (int)Plasma::BottomEdge);
    //setFormFactor((Plasma::FormFactor)cg.readEntry("formfactor", (int)Plasma::Horizontal));
    int s = cg.readEntry("screen", 0);

    setScreen(s);
    setLocation(loc);
    if (geo.isValid()) {
        updateSize(geo.size());
    } else {
        updateSize(56);
    }
    Containment::init();
}

QList<QAction*> Panel::contextActions()
{
    if (!m_appletBrowserAction) {
        m_appletBrowserAction = new QAction(i18n("Add Widgets..."), this);
        m_appletBrowserAction->setIcon(KIcon("list-add"));
        m_appletBrowserAction->setVisible(!isImmutable());
        connect(m_appletBrowserAction, SIGNAL(triggered()), this, SIGNAL(showAddWidgets()));

        m_configureAction = new QAction(i18n("Panel Settings"), this);
        m_configureAction->setIcon(KIcon("configure"));
        connect(m_configureAction, SIGNAL(triggered()), this, SLOT(configure()));
    }

    QList<QAction*> actions;
    actions << m_configureAction << m_appletBrowserAction;
    return actions;
}

void Panel::backgroundChanged()
{
    constraintsUpdated(Plasma::LocationConstraint);
}

void Panel::updateBorders()
{
    Plasma::Location loc = location();
    SvgPanel::BorderFlags bFlags = SvgPanel::DrawAllBorders;

    int s = qMax(0, screen());
    QRect r = QApplication::desktop()->screenGeometry(s);
    //kDebug() << loc << s << formFactor() << geometry();

    qreal topHeight = m_background->marginSize(Plasma::TopMargin);
    qreal bottomHeight = m_background->marginSize(Plasma::BottomMargin);
    qreal leftWidth = m_background->marginSize(Plasma::LeftMargin);
    qreal rightWidth = m_background->marginSize(Plasma::RightMargin);

    //remove unwanted borders
    if (loc == BottomEdge || loc == TopEdge) {
        if (loc == BottomEdge) {
            bFlags ^= SvgPanel::DrawBottomBorder;
            bottomHeight = 0;
        } else {
            bFlags ^= SvgPanel::DrawTopBorder;
            topHeight = 0;
        }
        //FIXME I think these tests may always return true
        //but it won't really be tested until we have non-fullwidth panels
        //FIXME! yes, it *is* broken. just not visibly so, yet.
        if (geometry().x() <= r.x()) {
            bFlags ^= SvgPanel::DrawLeftBorder;
            leftWidth = 0;
        }
        if (geometry().right() >= r.right()) {
            bFlags ^= SvgPanel::DrawRightBorder;
            rightWidth = 0;
        }
        //kDebug() << "top/bottom: Width:" << width << ", height:" << height;
    } else if (loc == LeftEdge || loc == RightEdge) {
        if (loc == RightEdge) {
            bFlags ^= SvgPanel::DrawRightBorder;
            rightWidth = 0;
        } else {
            bFlags ^= SvgPanel::DrawLeftBorder;
            leftWidth = 0;
        }
        if (geometry().y() <= r.y()) {
            bFlags ^= SvgPanel::DrawTopBorder;
            topHeight = 0;
        }
        if (geometry().bottom() >= r.bottom()) {
            bFlags ^= SvgPanel::DrawBottomBorder;
            bottomHeight = 0;
        }
        //kDebug() << "left/right: Width:" << width << ", height:" << height;
    } else {
        kDebug() << "no location!?";
    }

    if (layout()) {
        layout()->setMargin(Plasma::TopMargin, topHeight);
        layout()->setMargin(Plasma::LeftMargin, leftWidth);
        layout()->setMargin(Plasma::RightMargin, rightWidth);
        layout()->setMargin(Plasma::BottomMargin, bottomHeight);
    }

    m_background->setBorderFlags(bFlags);
}

void Panel::checkForConflict()
{
    if (corona()) {
        foreach (Containment *c, corona()->containments()) {
            if (c->type() != PanelContainment || c == this) {
                continue;
            }

            if (c->geometry().intersects(geometry())) {
                //TODO: here is where we need to schedule a negotiation for where to show the
                //      panel on the scene
                //
                //      we also probably need to direct whether to allow this containment to
                //      be resized before moved, or moved only
                kDebug() << "conflict!";
            }
            kDebug() << "panel containment with geometry of" << c->geometry() << "but really" << c->transform().map(geometry());
        }
    }
}

void Panel::constraintsUpdated(Plasma::Constraints constraints)
{
    //kDebug() << "constraints updated with" << constraints << "!!!!!!";
/*
    if (constraints & Plasma::LocationConstraint) {
        setFormFactorFromLocation();
    }
*/
    if (constraints & Plasma::ScreenConstraint || constraints & Plasma::LocationConstraint || 
            constraints & Plasma::SizeConstraint) {
        updatePos();
        updateBorders();
        checkForConflict();
    }

    if (constraints & Plasma::SizeConstraint) {
        m_background->resize(size());
    }

    if (constraints & Plasma::ImmutableConstraint && m_appletBrowserAction) {
        // we need to update the menu items that have already been created
        bool locked = isImmutable();
        m_appletBrowserAction->setVisible(!locked);
    }
}

void Panel::themeUpdated()
{
    //if the theme is changed all the calculations needs to be done again
    //TODO resize based on the change in theme bordersize
    constraintsUpdated(Plasma::LocationConstraint);
}

Qt::Orientations Panel::expandingDirections() const
{
    if (formFactor() == Plasma::Horizontal) {
        return Qt::Horizontal;
    } else {
        return Qt::Vertical;
    }
}

void Panel::paintInterface(QPainter *painter,
                           const QStyleOptionGraphicsItem *,
                           const QRect& contentsRect)
{
    //FIXME: this background drawing is bad and ugly =)
    // draw the background untransformed (saves lots of per-pixel-math)
    painter->save();
    painter->resetTransform();

    // blit the background (saves all the per-pixel-products that blending does)
    painter->setCompositionMode(QPainter::CompositionMode_Source);
    painter->setRenderHint(QPainter::Antialiasing);

    m_background->paint(painter, contentsRect);

    // restore transformation and composition mode
    painter->restore();
}

void Panel::configure()
{
    if (! m_dialog) {
        m_dialog = new KDialog();
        m_dialog->setCaption( i18nc("@title:window","Configure Panel") );
        m_dialog->setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Apply );
        connect(m_dialog, SIGNAL(applyClicked()), this, SLOT(applyConfig()));
        connect(m_dialog, SIGNAL(okClicked()), this, SLOT(applyConfig()));

        QWidget *p = m_dialog->mainWidget();
        QGridLayout *l = new QGridLayout(p);
        p->setLayout(l);

        QLabel *sizeLabel = new QLabel(i18n("Size:"), p);
        l->addWidget(sizeLabel, 0, 0);
        m_sizeCombo = new QComboBox(p);
        sizeLabel->setBuddy(m_sizeCombo);
        l->addWidget(m_sizeCombo, 0, 1);
        m_sizeCombo->addItem(i18n("Tiny"), QVariant(32));
        m_sizeCombo->addItem(i18n("Small"), QVariant(40));
        m_sizeCombo->addItem(i18n("Normal"), QVariant(56));
        m_sizeCombo->addItem(i18n("Large"), QVariant(72));
        m_sizeCombo->addItem(i18n("Custom"));
        m_sizeEdit = new KIntNumInput(p);
        m_sizeEdit->setRange(16, 256);
        l->addWidget(m_sizeEdit, 1, 1);
        l->setColumnStretch(1,1);
        connect(m_sizeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(sizeComboChanged()));

        QLabel *locationLabel = new QLabel(i18n("Location:"), p);
        l->addWidget(locationLabel, 2, 0);
        m_locationCombo = new QComboBox(p);
        locationLabel->setBuddy(m_locationCombo);
        l->addWidget(m_locationCombo, 2, 1);
        m_locationCombo->addItem(i18n("Bottom"), Plasma::BottomEdge);
        m_locationCombo->addItem(i18n("Top"), Plasma::TopEdge);
        m_locationCombo->addItem(i18n("Right"), Plasma::RightEdge);
        m_locationCombo->addItem(i18n("Left"), Plasma::LeftEdge);
    }

    int panelSize = (formFactor() == Plasma::Horizontal) ? size().height() : size().width();
    int idx = m_sizeCombo->count() - 1;
    for (int i = 0; i <= m_sizeCombo->count() - 2; ++i) {
        if (m_sizeCombo->itemData(i).toInt() == panelSize) {
            idx = i;
            break;
        }
    }
    m_sizeCombo->setCurrentIndex(idx);
    m_sizeEdit->setValue(panelSize);
    sizeComboChanged();
    idx = 0;
    for (int i = 0; i < m_locationCombo->count(); i++) {
        if (m_locationCombo->itemData(i).toInt() == location()) {
            idx = i;
            break;
        }
    }
    m_locationCombo->setCurrentIndex(idx);

    m_dialog->show();
}

void Panel::applyConfig()
{
    qreal newSize = m_sizeCombo->itemData(m_sizeCombo->currentIndex()).toInt();
    if (newSize == 0) {
        newSize = m_sizeEdit->value();
    }
    Plasma::Location newLoc = (Plasma::Location)(m_locationCombo->itemData(m_locationCombo->currentIndex()).toInt());

    qreal oldSize = (formFactor() == Plasma::Horizontal) ? size().height() : size().width();
    if (newLoc != location() || newSize != oldSize) {
        setLocation(newLoc);
        updateSize(newSize);
    }
}

void Panel::setFormFactorFromLocation() {
    switch (location()) {
    case BottomEdge:
    case TopEdge:
        setFormFactor(Plasma::Horizontal);
        break;
    case RightEdge:
    case LeftEdge:
        setFormFactor(Plasma::Vertical);
        break;
    default:
        kDebug() << "invalid location!!";
    }
}

//assumes location is an edge and screen is valid
//TODO handle floating location too
void Panel::updateSize(qreal newSize)
{
    QRect screenRect = QApplication::desktop()->screenGeometry(screen());
    QSizeF s;
    switch (location()) {
    case BottomEdge:
    case TopEdge:
        //FIXME: don't hardcode full width/height
        s.setWidth(screenRect.width());
        s.setHeight(newSize);
        break;
    case RightEdge:
    case LeftEdge:
        s.setWidth(newSize);
        s.setHeight(screenRect.height());
    default:
        kDebug() << "can't happen!";
    }
    updateSize(s);
}

void Panel::updateSize(QSizeF newSize)
{
    //formfactor *must* be set first. why? beats me...
    setFormFactorFromLocation();
    // Lock the size so that stray applets don't cause the panel to grow
    // or the removal of applets to cause the panel to shrink
    //TODO change this once panels aren't fullwidth
    setMinimumSize(newSize);
    setMaximumSize(newSize);
}

void Panel::updatePos()
{
    //the view actually ignores our pos() and handles positioning itself.
    //we just have to give it a kick here so it updates
    //TODO change this when we get floating panels
    emit geometryChanged();
    /*
    QPointF pos;
    switch (location()) {
    case BottomEdge:
        //pos.setY(screenRect.height() - size().height());
        //note: screenRect.height() == (screenRect.bottom() + 1) (if screen is at 0,0)
        break;
    case RightEdge:
        //pos.setX(screenRect.width() - size().width());
        break;
    default:
        break;
    }
    */
}

void Panel::sizeComboChanged()
{
    QVariant v = m_sizeCombo->itemData(m_sizeCombo->currentIndex());
    m_sizeEdit->setEnabled(v.isNull());
}

K_EXPORT_PLASMA_APPLET(panel, Panel)

#include "panel.moc"

