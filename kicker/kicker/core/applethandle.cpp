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

#include <QCursor>
#include <QBoxLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QPixmapCache>
#include <QStyle>
#include <QStyleOption>
#include <QTimer>
#include <QToolTip>

#include <kpushbutton.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kauthorized.h>

#include "container_applet.h"
#include "kickerSettings.h"

#include "applethandle.h"

static const char* const up_xpm[]={
"5 5 2 1",
"# c black",
". c None",
".....",
"..#..",
".###.",
"#####",
"....."};

static const char* const down_xpm[]={
"5 5 2 1",
"# c black",
". c None",
".....",
"#####",
".###.",
"..#..",
"....."};

static const char* const left_xpm[]={
"5 5 2 1",
"# c black",
". c None",
"...#.",
"..##.",
".###.",
"..##.",
"...#."};

static const char* const right_xpm[]={
"5 5 2 1",
"# c black",
". c None",
".#...",
".##..",
".###.",
".##..",
".#..."};

AppletHandle::AppletHandle(AppletContainer* parent)
    : QWidget(parent),
      m_applet(parent),
      m_menuButton(0),
      m_drawHandle(false),
      m_popupDirection(Plasma::Up),
      m_handleHoverTimer(0)
{
    setBackgroundOrigin(AncestorOrigin);
    setMinimumSize(widthForHeight(0), heightForWidth(0));
    m_layout = new QBoxLayout(this, QBoxLayout::BottomToTop, 0, 0);

    m_dragBar = new AppletHandleDrag(this);
    m_dragBar->installEventFilter(this);
    m_layout->addWidget(m_dragBar);

    if (KAuthorized::authorizeKAction("kicker_rmb"))
    {
        m_menuButton = new AppletHandleButton( this );
        m_menuButton->setPixmap(xpmPixmap(up_xpm, "up"));
        m_menuButton->installEventFilter(this);
        m_layout->addWidget(m_menuButton);

        connect(m_menuButton, SIGNAL(pressed()),
                this, SLOT(menuButtonPressed()));
        m_menuButton->setToolTip( i18n("%1 menu", parent->info().name()));
    }

    this->setToolTip( i18n("%1 applet handle", parent->info().name()));
    resetLayout();
}

int AppletHandle::heightForWidth( int /* w */ ) const
{
    return style()->pixelMetric(QStyle::PM_ToolBarHandleExtent, 0, this);
}

int AppletHandle::widthForHeight( int /* h */ ) const
{
    return style()->pixelMetric(QStyle::PM_ToolBarHandleExtent, 0, this);
}

void AppletHandle::setPopupDirection(Plasma::Position d)
{
    if (d == m_popupDirection || !m_menuButton)
    {
        return;
    }

    m_popupDirection = d;

    switch (m_popupDirection)
    {
        case Plasma::Up:
            m_layout->setDirection(QBoxLayout::BottomToTop);
            m_menuButton->setPixmap(xpmPixmap(up_xpm, "up"));
            break;
        case Plasma::Down:
            m_layout->setDirection(QBoxLayout::TopToBottom);
            m_menuButton->setPixmap(xpmPixmap(down_xpm, "down"));
            break;
        case Plasma::Left:
            m_layout->setDirection(QBoxLayout::RightToLeft);
            m_menuButton->setPixmap(xpmPixmap(left_xpm, "left"));
            break;
        case Plasma::Right:
            m_layout->setDirection(QBoxLayout::LeftToRight);
            m_menuButton->setPixmap(xpmPixmap(right_xpm, "right"));
            break;
	case Plasma::Floating:
	    m_layout->setDirection(QBoxLayout::BottomToTop);
	    m_menuButton->setPixmap(xpmPixmap(up_xpm, "up"));
	    break;
    }

    m_layout->activate();
}

void AppletHandle::resetLayout()
{
    if (m_handleHoverTimer && !m_drawHandle)
    {
        m_dragBar->hide();

        if (m_menuButton)
        {
            m_menuButton->hide();
        }
    }
    else
    {
        m_dragBar->show();

        if (m_menuButton)
        {
            m_menuButton->show();
        }
    }
}

void AppletHandle::setFadeOutHandle(bool fadeOut)
{
    if (fadeOut)
    {
        if (!m_handleHoverTimer)
        {
            m_handleHoverTimer = new QTimer(this);
            connect(m_handleHoverTimer, SIGNAL(timeout()),
                    this, SLOT(checkHandleHover()));
            m_applet->installEventFilter(this);
        }
    }
    else
    {
        delete m_handleHoverTimer;
        m_handleHoverTimer = 0;
        m_applet->removeEventFilter(this);
    }

    resetLayout();
}

bool AppletHandle::eventFilter(QObject *o, QEvent *e)
{
    if (o == parent())
    {
        switch (e->type())
        {
            case QEvent::Enter:
            {
                m_drawHandle = true;
                resetLayout();

                if (m_handleHoverTimer)
                {
                    m_handleHoverTimer->start(250);
                }
                break;
            }

            case QEvent::Leave:
            {
                if (m_menuButton && m_menuButton->isChecked())
                {
                    break;
                }

                QWidget* w = dynamic_cast<QWidget*>(o);

                bool nowDrawIt = false;
                if (w)
                {
                    // a hack for applets that have out-of-process
                    // elements (e.g the systray) so that the handle
                    // doesn't flicker when moving over those elements
                    if (w->rect().contains(w->mapFromGlobal(QCursor::pos())))
                    {
                        nowDrawIt = true;
                    }
                }

                if (nowDrawIt != m_drawHandle)
                {
                    if (m_handleHoverTimer)
                    {
                        m_handleHoverTimer->stop();
                    }

                    m_drawHandle = nowDrawIt;
                    resetLayout();
                }
                break;
            }

            default:
                break;
        }

        return QWidget::eventFilter( o, e );
    }
    else if (o == m_dragBar)
    {
        if (e->type() == QEvent::MouseButtonPress)
        {
            QMouseEvent* ev = static_cast<QMouseEvent*>(e);
            if (ev->button() == Qt::LeftButton || ev->button() == Qt::MidButton)
            {
                emit moveApplet(m_applet->mapFromGlobal(ev->globalPos()));
            }
        }
    }

    if (m_menuButton && e->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent* ev = static_cast<QMouseEvent*>(e);
        if (ev->button() == Qt::RightButton)
        {
            if (!m_menuButton->isDown())
            {
                m_menuButton->setDown(true);
                menuButtonPressed();
            }

            return true;
        }
    }

    return QWidget::eventFilter(o, e);    // standard event processing
}

void AppletHandle::menuButtonPressed()
{
    if (!KAuthorized::authorizeKAction("kicker_rmb"))
    {
        return;
    }

    m_menuButton->setChecked(true);
    emit showAppletMenu();

    if (!onMenuButton(QCursor::pos()))
    {
        toggleMenuButtonOff();
    }
}

void AppletHandle::checkHandleHover()
{
    if (!m_handleHoverTimer ||
        (m_menuButton && m_menuButton->isChecked()) ||
        m_applet->geometry().contains(m_applet->mapToParent(
                                      m_applet->mapFromGlobal(QCursor::pos()))))
    {
        return;
    }

    m_handleHoverTimer->stop();
    m_drawHandle = false;
    resetLayout();
}

bool AppletHandle::onMenuButton(const QPoint& point) const
{
    return m_menuButton && (childAt(mapFromGlobal(point)) == m_menuButton);
}

void AppletHandle::toggleMenuButtonOff()
{
    if (!m_menuButton)
    {
        return;
    }

    m_menuButton->setChecked(false);
    m_menuButton->setDown(false);
}

QPixmap AppletHandle::xpmPixmap( const char* const xpm[], const char* _key )
{
   QString key = QString("$kde_kicker_applethandle_") + _key;
   if (QPixmap* pm = QPixmapCache::find(key))
   {
       return *pm;
   }

   QPixmap pm(const_cast< const char** >(xpm));
   QPixmapCache::insert(key, pm);
   return pm;
}

AppletHandleDrag::AppletHandleDrag(AppletHandle* parent)
    : QWidget(parent),
      m_parent(parent)
{
   setBackgroundOrigin( AncestorOrigin );
}

QSize AppletHandleDrag::minimumSizeHint() const
{
    int wh = style()->pixelMetric(QStyle::PM_ToolBarHandleExtent, 0, this);

    if (m_parent->orientation() == Qt::Horizontal)
    {
        return QSize(wh, 0);
    }

    return QSize(0, wh);
}

QSizePolicy AppletHandleDrag::sizePolicy() const
{
    if (m_parent->orientation() == Qt::Horizontal)
    {
        return QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Preferred );
    }

    return QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
}

void AppletHandleDrag::paintEvent(QPaintEvent *)
{
    QPainter p(this);

    QStyle::SFlags flags = QStyle::State_Default | QStyle::State_Enabled;
    if (m_parent->orientation() == Qt::Horizontal)
    {
        flags |= QStyle::State_Horizontal;
    }

    QRect r = rect();

    QStyleOptionFrame opt;
    opt.init(this);
    opt.lineWidth = 1;
    opt.midLineWidth = 0;
    opt.state = flags;

    if (m_parent->orientation() == Qt::Horizontal)
    {
        if (QApplication::isRightToLeft())
        {
            // paint it on the right-hand side
            opt.rect = QRect(width() - 2, 0, 2, height());
            style()->drawPrimitive(QStyle::PE_FrameDockWidget, &opt, &p, this);
        }
        else
        {
            // paint it on the left-hand side
            opt.rect = QRect(width() - 2, 0, 2, height());
            style()->drawPrimitive(QStyle::PE_FrameDockWidget, &opt, &p, this);
        }
    }
    else
    {
        //vertical, paint it on top
        opt.rect = QRect(0, 0, width(), 2);
        style()->drawPrimitive(QStyle::PE_FrameDockWidget, &opt, &p, this);
        r.rTop() += 2;
    }

    QStyleOption opt2;
    opt2.init(this);
    opt2.state = flags;
    opt2.rect  = r;
    style()->drawPrimitive(QStyle::PE_IndicatorToolBarHandle, &opt2, &p, this);

    p.end();
}

AppletHandleButton::AppletHandleButton(AppletHandle *parent)
  : QPushButton(parent),
    m_parent(parent)
{
    m_moveMouse = false;
    setFocusPolicy(Qt::NoFocus);
    setCheckable(true);
    setCheckable(true);
}

AppletHandleButton::~AppletHandleButton()
{
}

QSize AppletHandleButton::minimumSizeHint() const
{
    int height = style()->pixelMetric(QStyle::PM_ToolBarHandleExtent, 0, this);
    int width = height;

    if (m_parent->orientation() == Qt::Horizontal)
    {
        return QSize(width, height);
    }

    return QSize(height, width);
}

QSizePolicy AppletHandleButton::sizePolicy() const
{
    return QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

void AppletHandleButton::setPixmap(const QPixmap& pixmap)
{
     m_pixmap = pixmap;
}

const QPixmap& AppletHandleButton::pixmap()
{
    return m_pixmap;
}

void AppletHandleButton::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.fillRect(rect(), palette().brush(backgroundRole()));

    QStyleOptionFrame opt;
    opt.init(this);
    opt.lineWidth = 1;
    opt.midLineWidth = 0;
    opt.state = QStyle::State_Horizontal;

    if (m_parent->orientation() == Qt::Horizontal)
    {
        if (layoutDirection() == Qt::RightToLeft)
        {
             // paint it on the right-hand side
             opt.rect = QRect(width() - 2, 0, 2, height());
             style()->drawPrimitive(QStyle::PE_FrameDockWidget, &opt, &p, this);
        }
        else
        {
             opt.rect = QRect(0, 0, 2, height());
             style()->drawPrimitive(QStyle::PE_FrameDockWidget, &opt, &p, this);
        }
    }
    else
    {
       //vertical, paint it on top
       opt.rect = QRect( 0, 0, width(), 2 );
       style()->drawPrimitive( QStyle::PE_FrameDockWidget, &opt, &p, this );
    }

    int w = width();
    int h = height();
    if (m_parent->orientation() == Qt::Horizontal)
    {
        if (kapp->layoutDirection() != Qt::RightToLeft)
        {
            p.translate(2, 0);
        }

        w -= 2;
    }
    else
    {
        p.translate(0, 2);
        h -= 2;
    }

    p.drawPixmap((w - pixmap().width()) / 2,
                 (h - pixmap().height()) / 2,
                 pixmap());

    --w;
    --h;

    if (m_moveMouse && !isDown())
    {
        p.setPen(Qt::white);
        p.drawLine(0, h, 0, 0);
        p.drawLine(0, 0, w, 0);

        p.setPen(palette().dark());
        p.drawLine(w, 0, w, h);
        p.drawLine(w, h, 0, h);
    }

    if (isChecked() || isDown())
    {
        p.setPen(palette().dark());
        p.drawLine(0, h, 0, 0);
        p.drawLine(0, 0, w, 0);

        p.setPen(Qt::white);
        p.drawLine(w, 0, w, h);
        p.drawLine(w, h, 0, h);
    }
}

void AppletHandleButton::enterEvent(QEvent*)
{
    m_moveMouse = true;
    repaint();
}

void AppletHandleButton::leaveEvent(QEvent*)
{
    m_moveMouse = false;
    repaint();
}

#include "applethandle.moc"
