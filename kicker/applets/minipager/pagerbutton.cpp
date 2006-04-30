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

#include <qdrawutil.h>

#include <QBitmap>
#include <QCursor>
#include <QDesktopWidget>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>
#include <QEvent>
#include <QResizeEvent>
#include <QLineEdit>
#include <QList>
#include <QMenu>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QPixmap>

#include <netwm.h>
#include <dcopclient.h>

#include <kwinmodule.h>
#include <ksharedpixmap.h>
#include <kpixmapeffect.h>
#include <kstringhandler.h>
#include <kiconloader.h>

#include "utils.h"
#include "kickertip.h"
#include "kickerSettings.h"
#include "kshadowengine.h"

#include "pagerapplet.h"
#include "pagerbutton.h"
#include "pagerbutton.moc"
#include "pagersettings.h"

#ifdef FocusOut
#undef FocusOut
#endif

KSharedPixmap* KMiniPagerButton::s_commonSharedPixmap;
KPixmap* KMiniPagerButton::s_commonBgPixmap;

KMiniPagerButton::KMiniPagerButton(int desk, KMiniPager *parent, const char *name)
    : QAbstractButton(parent, name, Qt::WNoAutoErase),
      m_pager(parent),
      m_desktop(desk),
      m_lineEdit(0),
      m_sharedPixmap(0),
      m_bgPixmap(0),
      m_isCommon(false),
      m_currentWindow(0)
{
    setCheckable(true);
    setAcceptDrops(true);

    //setBackgroundOrigin(AncestorOrigin);
    installEventFilter(KickerTip::self());

    m_desktopName = m_pager->kwin()->desktopName(m_desktop);

    connect(this, SIGNAL(clicked()), SLOT(slotClicked()));
    connect(this, SIGNAL(toggled(bool)), SLOT(slotToggled(bool)));
    connect(&m_dragSwitchTimer, SIGNAL(timeout()), this, SLOT(slotDragSwitch()));

    m_dragSwitchTimer.setSingleShot(true);

    if (m_pager->desktopPreview())
    {
        setMouseTracking(true);
    }
    loadBgPixmap();
}

KMiniPagerButton::~KMiniPagerButton()
{
    delete m_sharedPixmap;
    delete m_bgPixmap;
}

bool KMiniPagerButton::shouldPaintWindow( KWin::WindowInfo *info )
{
    if (!info)
      return false;

//  if (info->mappingState != NET::Visible)
//    return false;

    NET::WindowType type = info->windowType( NET::NormalMask | NET::DesktopMask
        | NET::DockMask | NET::ToolbarMask | NET::MenuMask | NET::DialogMask
        | NET::OverrideMask | NET::TopMenuMask | NET::UtilityMask | NET::SplashMask );

    if (type == NET::Desktop || type == NET::Dock || type == NET::TopMenu)
      return false;

    if (!info->isOnDesktop(m_desktop))
      return false;

    if (info->state() & NET::SkipPager || info->state() & NET::Shaded )
      return false;

    if (info->win() == m_pager->winId())
      return false;

    if ( info->isMinimized() )
      return false;

    return true;
}

void KMiniPagerButton::resizeEvent(QResizeEvent *ev)
{
    if (m_lineEdit)
    {
        m_lineEdit->setGeometry(rect());
    }

    delete m_bgPixmap;
    m_bgPixmap = 0;

    QAbstractButton::resizeEvent(ev);
}

void KMiniPagerButton::windowsChanged()
{
    m_currentWindow = 0;
    update();
}

void KMiniPagerButton::backgroundChanged()
{
    delete s_commonSharedPixmap;
    s_commonSharedPixmap = 0;
    delete s_commonBgPixmap;
    s_commonBgPixmap = 0;
    loadBgPixmap();
}

void KMiniPagerButton::loadBgPixmap()
{
    if (m_pager->bgType() != PagerSettings::EnumBackgroundType::BgLive)
        return; // not needed

    DCOPClient *client = kapp->dcopClient();
    if (!client->isAttached())
    {
        client->attach();
    }

    bool isCommon;
    DCOPReply result = DCOPRef("kdesktop", "KBackgroundIface").call("isCommon()");
    if (result.get(isCommon))
    {
        m_isCommon = isCommon;
    }

    if (m_isCommon)
    {
        if (s_commonBgPixmap)
        { // pixmap is already ready, just use it
            backgroundLoaded( true );
            return;
        }
        else if (s_commonSharedPixmap)
        { // other button is already fetching the pixmap
            connect(s_commonSharedPixmap, SIGNAL(done(bool)),
                    SLOT(backgroundLoaded(bool)));
            return;
        }
    }

    DCOPRef("kdesktop", "KBackgroundIface").send("setExport", 1);

    if (m_isCommon)
    {
        if (!s_commonSharedPixmap)
        {
            s_commonSharedPixmap = new KSharedPixmap;
            connect(s_commonSharedPixmap, SIGNAL(done(bool)),
                    SLOT(backgroundLoaded(bool)));
        }
        s_commonSharedPixmap->loadFromShared(QString("DESKTOP1"));
    }
    else
    {
        if (!m_sharedPixmap)
        {
            m_sharedPixmap = new KSharedPixmap;
            connect(m_sharedPixmap, SIGNAL(done(bool)),
                    SLOT(backgroundLoaded(bool)));
        }
        m_sharedPixmap->loadFromShared(QString("DESKTOP%1").arg(m_desktop));
    }
}

static QPixmap scalePixmap(const QPixmap &pixmap, int width, int height)
{
    return pixmap.scaled(width, height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}

QPixmap fastScalePixmap(const QPixmap &pixmap, int width, int height)
{
    QMatrix m;
    m.scale( (width / (double)pixmap.width()), (height / (double)pixmap.height()) );
    return pixmap.transformed(m);
}

void KMiniPagerButton::backgroundLoaded( bool loaded )
{
    if (loaded)
    {
        if (!m_bgPixmap)
        {
            m_bgPixmap = new KPixmap;
        }
        if (m_isCommon)
        {
            if (!s_commonBgPixmap)
            {
                s_commonBgPixmap = new KPixmap;
                *s_commonBgPixmap = scalePixmap(s_commonSharedPixmap->pixmap(), width(), height());
                s_commonSharedPixmap->deleteLater(); // let others get the signal too
                s_commonSharedPixmap = 0;
            }
            *m_bgPixmap = *s_commonBgPixmap;
        }
        else
        {
            *m_bgPixmap = scalePixmap(m_sharedPixmap->pixmap(), width(), height());
            delete m_sharedPixmap;
            m_sharedPixmap = 0L;
        }

/*        delete m_sharedPixmap;
        m_sharedPixmap = 0L;
*/
        update();
    }
    else
    {
        kDebug() << "Error getting the background\n";
    }
}

void KMiniPagerButton::paintEvent(QPaintEvent *)
{
    int w = width();
    int h = height();
    bool on = isEnabled();
    bool down = isChecked();
    QPixmap buffer(w, h);
    QBitmap mask(w, h, true);
    QPainter bp(&buffer); //### copied attrs from this
    QPainter mp(&mask);

    QBrush background;

    bool liveBkgnd = m_pager->bgType() == PagerSettings::EnumBackgroundType::BgLive;
    bool transparent = m_pager->bgType() == PagerSettings::EnumBackgroundType::BgTransparent;

    // background...
    if (liveBkgnd)
    {
        if (m_bgPixmap && !m_bgPixmap->isNull())
        {
            if (on)
            {
                KPixmap tmp = *m_bgPixmap;
                KPixmapEffect::intensity(tmp, 0.33);
                bp.drawPixmap(0, 0, tmp);
            }
            else
            {
                bp.drawPixmap(0, 0, *m_bgPixmap);
            }
        }
        else
        {
            liveBkgnd = false;
        }

    }

    if (!liveBkgnd)
    {
        if (transparent)
        {
            // transparent windows get an 1 pixel frame...
            if (on)
            {
                bp.setPen(palette().color( QPalette::Midlight ) );
            }
            else if (down)
            {
                bp.setPen(Plasma::blendColors(palette().color( QPalette::Mid ),
                                              palette().color( QPalette::Midlight ) ) );
            }
            else
            {
                bp.setPen( palette().color( QPalette::Dark ) );
            }

            bp.drawRect( buffer.rect() );
            mp.setPen( Qt::color1 );
            mp.drawRect( buffer.rect() );
        }
        else
        {
            QBrush background;

            if (on)
            {
                background = palette().brush(QPalette::Midlight);
            }
            else if (down)
            {
                background = Plasma::blendColors(palette().color( QPalette::Mid ),
                                                 palette().color( QPalette::Midlight ) );
            }
            else
            {
                background = palette().brush(QPalette::Mid);
            }

            bp.fillRect(buffer.rect(), background);
        }
    }

    // window preview...
    if (m_pager->desktopPreview())
    {
        KWinModule* kwin = m_pager->kwin();
        KWin::WindowInfo *info = 0;
        int dw = QApplication::desktop()->width();
        int dh = QApplication::desktop()->height();

        QList<WId> windows = kwin->stackingOrder();
        QList<WId>::const_iterator itEnd = windows.constEnd();
        for (QList<WId>::ConstIterator it = windows.constBegin(); it != itEnd; ++it)
        {
            info = m_pager->info(*it);

            if (shouldPaintWindow(info))
            {
                QRect r =  info->frameGeometry();
                r = QRect(r.x() * width() / dw, 2 + r.y() * height() / dh,
                          r.width() * width() / dw, r.height() * height() / dh);

                if (kwin->activeWindow() == info->win())
                {
                    QBrush brush = palette().brush(QPalette::Highlight);
                    qDrawShadeRect(&bp, r, palette(), false, 1, 0, &brush);
                }
                else
                {
                    QBrush brush = palette().brush(QPalette::Button);

                    if (on)
                    {
                        brush.setColor(brush.color().light(120));
                    }

                    bp.fillRect(r, brush);
                    qDrawShadeRect(&bp, r, palette(), true, 1, 0);
                }

                if (transparent)
                {
                    mp.fillRect(r, Qt::color1);
                }

                if (m_pager->windowIcons() && r.width() > 15 && r.height() > 15)
                {
                    QPixmap icon = KWin::icon(*it, 16, 16, true);
                    if (!icon.isNull())
                    {
                        bp.drawPixmap(r.left() + ((r.width() - 16) / 2),
                                      r.top() + ((r.height() - 16) / 2),
                                      icon);
                    }
                }
            }
        }
    }

    if (liveBkgnd)
    {
        // draw a little border around the individual buttons
        // makes it look a bit more finished.
        if (on)
        {
            bp.setPen(palette().midlight());
        }
        else
        {
            bp.setPen(palette().mid());
        }

        bp.drawRect(0, 0, w, h);
    }

    mp.end();

    if (transparent)
    {
        bp.end();
        buffer.setMask(mask);
        erase();

        bp.begin(this);
        bp.drawPixmap(0, 0, buffer);
    }

    if (m_pager->labelType() != PagerSettings::EnumLabelType::LabelNone)
    {
        QString label = (m_pager->labelType() == PagerSettings::EnumLabelType::LabelNumber) ?
                            QString::number(m_desktop) : m_desktopName;
        QPainter tp;
        QPixmap textPixmap(width(), height());

        textPixmap.fill(QColor(0,0,0));
        textPixmap.setMask(textPixmap.createHeuristicMask(true));

        // draw text
        tp.begin(&textPixmap);
        tp.setPen(Qt::white);
        tp.setFont(font()); // get the font from the root painter
        tp.drawText(0, 0, w, h, Qt::AlignCenter, label);
        tp.end();

        // draw shadow
        QImage img = m_pager->shadowEngine()->makeShadow(textPixmap,
                                                         liveBkgnd ? Qt::black
                                                                   : Qt::white);

        bp.drawImage(0, 0, img);
        bp.drawText(0, 0, w, h, Qt::AlignCenter, label);
    }

    if (!transparent)
    {
        bp.end();
        bp.begin(this);
        bp.drawPixmap(0, 0, buffer);
    }

    bp.end();
}

void KMiniPagerButton::mousePressEvent(QMouseEvent * e)
{
    if (e->button() == Qt::RightButton)
    {
        // prevent LMB down -> RMB down -> LMB up sequence
        if ((e->state() & Qt::MouseButtonMask ) == Qt::NoButton)
        {
            emit showMenu(e->globalPos(), m_desktop);
            return;
        }
    }

    if (m_pager->desktopPreview())
    {
        m_pager->clickPos = e->pos();
    }

    QAbstractButton::mouseReleaseEvent(e);
}

void KMiniPagerButton::mouseReleaseEvent(QMouseEvent* e)
{
    m_pager->clickPos = QPoint();
    QAbstractButton::mouseReleaseEvent(e);
}

void KMiniPagerButton::mouseMoveEvent(QMouseEvent* e)
{
    if (!m_pager->desktopPreview())
    {
        return;
    }

    int dw = QApplication::desktop()->width();
    int dh = QApplication::desktop()->height();
    int w = width();
    int h = height();

    QPoint pos(m_pager->clickPos.isNull() ? mapFromGlobal(QCursor::pos()) : m_pager->clickPos);
    QPoint p(pos.x() * dw / w, pos.y() * dh / h);
    Task::TaskPtr wasWindow = m_currentWindow;
    m_currentWindow = TaskManager::self()->findTask(m_desktop, p);

    if (wasWindow != m_currentWindow)
    {
        KickerTip::Client::updateTip();
    }

    if (m_currentWindow && !m_pager->clickPos.isNull() &&
        (m_pager->clickPos - e->pos()).manhattanLength() > KGlobalSettings::dndEventDelay())
    {
        QRect r = m_currentWindow->geometry();

        // preview window height, window width
        int ww = r.width() * w / dw;
        int wh = r.height() * h / dh;
        QPixmap windowImage(ww, wh);
        QPainter bp(&windowImage); //### copied attributes from this

        bp.setPen( palette().color( QPalette::Foreground ) );
        bp.drawRect(0, 0, ww, wh);
        bp.fillRect(1, 1, ww - 2, wh - 2, palette().color( QPalette::Background) );

        Task::List tasklist;
        tasklist.append(m_currentWindow);
        TaskDrag* drag = new TaskDrag(tasklist, this);
        QPoint offset(m_pager->clickPos.x() - (r.x() * w / dw),
                m_pager->clickPos.y() - (r.y() * h / dh));
        drag->setPixmap(windowImage);
        drag->setHotSpot(offset);
        drag->start();

        if (isDown())
        {
            setDown(false);
        }

        m_pager->clickPos = QPoint();
    }
}

void KMiniPagerButton::dragEnterEvent(QDragEnterEvent* e)
{
    kDebug() << "received drag " << e->format() << endl;
    if (TaskDrag::canDecode(e->mimeData()))
    {
        // if it's a task drag don't switch the desktop, just accept it
        e->accept();
        setDown(true);
    }
    else
    {
        // if a dragitem is held for over a pager button for two seconds,
        // activate corresponding desktop
        m_dragSwitchTimer.start(1000);
        QAbstractButton::dragEnterEvent( e );
    }
}

void KMiniPagerButton::dropEvent(QDropEvent* e)
{
    if (TaskDrag::canDecode(e->mimeData()))
    {
        e->accept();
        Task::List tasks(TaskDrag::decode(e->mimeData()));

        if (tasks.count() == 1)
        {
            Task::TaskPtr task = tasks[0];
            int dw = QApplication::desktop()->width();
            int dh = QApplication::desktop()->height();
            int w = width();
            int h = height();
            QRect location = task->geometry();
            location.translate((e->pos().x() - m_pager->clickPos.x()) * dw / w,
                            (e->pos().y() - m_pager->clickPos.y()) * dh / h);

            XMoveWindow(x11Display(), task->window(), location.x(), location.y());
            if ((e->source() != this || !task->isOnAllDesktops()) &&
                task->desktop() != m_desktop)
            {
                task->toDesktop(m_desktop);
            }
        }
        else
        {
            Task::List::iterator itEnd = tasks.end();
            for (Task::List::iterator it = tasks.begin(); it != itEnd; ++it)
            {
                (*it)->toDesktop(m_desktop);
            }
        }

        setDown(false);
    }

    QAbstractButton::dropEvent( e );
}

void KMiniPagerButton::enabledChange( bool oldEnabled )
{
    if (m_pager->bgType() == PagerSettings::EnumBackgroundType::BgLive)
    {
        m_pager->refresh();
    }

    QAbstractButton::enabledChange( oldEnabled );
}

void KMiniPagerButton::dragLeaveEvent( QDragLeaveEvent* e )
{
    m_dragSwitchTimer.stop();

    if (m_pager->kwin()->currentDesktop() != m_desktop)
    {
        setDown(false);
    }

    QAbstractButton::dragLeaveEvent( e );
}

void KMiniPagerButton::slotDragSwitch()
{
    emit buttonSelected(m_desktop);
}

void KMiniPagerButton::slotClicked()
{
    emit buttonSelected(m_desktop);
}

void KMiniPagerButton::rename()
{
  if ( !m_lineEdit ) {
    m_lineEdit = new QLineEdit( this );
    connect( m_lineEdit, SIGNAL( returnPressed() ), m_lineEdit, SLOT( hide() ) );
    m_lineEdit->installEventFilter( this );
  }
  m_lineEdit->setGeometry( rect() );
  m_lineEdit->setText(m_desktopName);
  m_lineEdit->show();
  m_lineEdit->setFocus();
  m_lineEdit->selectAll();
  m_pager->emitRequestFocus();
}

void KMiniPagerButton::slotToggled( bool b )
{
    if ( !b && m_lineEdit )
    {
        m_lineEdit->hide();
    }
}

bool KMiniPagerButton::eventFilter( QObject *o, QEvent * e)
{
    if (o && o == m_lineEdit &&
        (e->type() == QEvent::FocusOut || e->type() == QEvent::Hide))
    {
        m_pager->kwin()->setDesktopName( m_desktop, m_lineEdit->text() );
        m_desktopName = m_lineEdit->text();
        QTimer::singleShot( 0, m_lineEdit, SLOT( deleteLater() ) );
        m_lineEdit = 0;
        return true;
    }

    return QAbstractButton::eventFilter(o, e);
}

void KMiniPagerButton::updateTipData(KickerTip::Data &data)
{
    Task::Dict tasks = TaskManager::self()->tasks();
    Task::Dict::iterator taskEnd = tasks.end();
    uint taskCounter = 0;
    uint taskLimiter = 4;
    QString lastWindow;

    for (Task::Dict::iterator it = tasks.begin(); it != taskEnd; ++it)
    {
        if (it.value()->desktop() == m_desktop || it.value()->isOnAllDesktops())
        {
            taskCounter++;
            if (taskCounter > taskLimiter)
            {
                lastWindow = it.value()->visibleName();
                continue;
            }

            if (it.value() == m_currentWindow)
            {
                data.subtext.append("<br>&bull; <u>").append(it.value()->visibleName()).append("</u>");
            }
            else
            {
                data.subtext.append("<br>&bull; ").append(it.value()->visibleName());
            }
        }
    }

    if (taskCounter > taskLimiter)
    {
        if (taskCounter - taskLimiter == 1)
        {
            data.subtext.append("<br>&bull; ").append(lastWindow);
        }
        else
        {
            data.subtext.append("<br>&bull; <i>")
                        .append(i18n("and %1 others",
                                     taskCounter - taskLimiter))
                        .append("</i>");
        }
    }

    if (taskCounter > 0)
    {
        data.subtext.prepend(i18np("One window:",
                                  "%n Windows:",
                                  taskCounter));
    }

    data.icon = DesktopIcon("window_list", K3Icon::SizeMedium);
    data.message = m_desktopName;
    data.direction = m_pager->popupDirection();
}

