/*
 *   Copyright 2007 Aaron Seigo <aseigo@kde.org>
 *   Copyright 2007 Matt Broadstone <mbroadst@gmail.com>
 *   Copyright 2007 André Duffeck <duffeck@kde.org>
 *   Copyright 2008 Chani Armitage <chanika@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
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

#include "saverview.h"

#include <QAction>
#include <QDesktopWidget>
#include <QKeyEvent>
#include <QTimer>

//#include <KWindowSystem>

#include "plasma/applet.h"
#include "plasma/corona.h"
#include "plasma/containment.h"
#include "plasma/svg.h"
#include "plasma/appletbrowser.h"
#include "plasmaapp.h"

static const int SUPPRESS_SHOW_TIMEOUT = 500; // Number of millis to prevent reshow of dashboard

SaverView::SaverView(Plasma::Containment *containment, QWidget *parent)
    : Plasma::View(containment, parent),
      m_appletBrowser(0),
      m_suppressShow(false)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint |
            Qt::X11BypassWindowManagerHint);
    if (!PlasmaApp::hasComposite()) {
        setAutoFillBackground(false);
        setAttribute(Qt::WA_NoSystemBackground);
    }

    //app is doing this for us - if needed
    //QDesktopWidget *desktop = QApplication::desktop();
    //setGeometry(desktop->screenGeometry(containment->screen()));

    //TODO the ctmt should only offer wallpaper to the user if there's no composite
    //and we'll be waiting on pluggable backgrounds even for that.
    setWallpaperEnabled(!PlasmaApp::hasComposite());

    //TODO 'always show widgets' option will change this.
    //also, need a way to be sure the screensaver's shown if we're going byebye
    //connect(scene(), SIGNAL(releaseVisualFocus()), SLOT(hideView()));

    //I suppose it doesn't hurt to leave this in
    m_hideAction = new QAction(i18n("Hide Widgets"), this);
    m_hideAction->setIcon(KIcon("preferences-desktop-display"));
    m_hideAction->setEnabled(false);
    containment->addToolBoxTool(m_hideAction);
    connect(m_hideAction, SIGNAL(triggered()), this, SLOT(hideView()));

    installEventFilter(this);
}

SaverView::~SaverView()
{
    delete m_appletBrowser;
}

void SaverView::drawBackground(QPainter * painter, const QRectF & rect)
{
    if (PlasmaApp::hasComposite()) {
        setWallpaperEnabled(false);
        painter->setCompositionMode(QPainter::CompositionMode_Source);
        if (PlasmaApp::self()->cheatsEnabled()) {
            painter->fillRect(geometry(), QColor(0, 0, 0, 180));
        } else {
            painter->fillRect(geometry(), QColor(0, 0, 0, 0));
        }
        //FIXME kwin's shadow effect is getting drawn behind me. do not want.
    } else {
        setWallpaperEnabled(true);
        Plasma::View::drawBackground(painter, rect);
    }
}

void SaverView::showAppletBrowser()
{
    if (!m_appletBrowser) {
        m_appletBrowser = new Plasma::AppletBrowser(this, Qt::FramelessWindowHint );
        m_appletBrowser->setContainment(containment());
        //TODO: make this proportional to the screen
        m_appletBrowser->setInitialSize(QSize(400, 400));
        m_appletBrowser->setApplication();
        m_appletBrowser->setWindowTitle(i18n("Add Widgets"));
        QPalette p = m_appletBrowser->palette();
        p.setBrush(QPalette::Background, QBrush(QColor(0, 0, 0, 180)));
        m_appletBrowser->setPalette(p);
        m_appletBrowser->setBackgroundRole(QPalette::Background);
        m_appletBrowser->setAutoFillBackground(true);
        m_appletBrowser->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint |
                Qt::X11BypassWindowManagerHint);
        //KWindowSystem::setState(m_appletBrowser->winId(), NET::KeepAbove|NET::SkipTaskbar);
        m_appletBrowser->move(0, 0);
        m_appletBrowser->installEventFilter(this);
    }

    kDebug();
    m_appletBrowser->setHidden(m_appletBrowser->isVisible());
}

void SaverView::appletBrowserDestroyed()
{
    m_appletBrowser = 0;
}

bool SaverView::eventFilter(QObject *watched, QEvent *event)
{
    if (watched != m_appletBrowser) {
        /*if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *me = static_cast<QMouseEvent *>(event);
            if (me->button() == Qt::LeftButton) {
                hideView();
            }
        }*/
        return false;
    }

    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *me = static_cast<QMouseEvent *>(event);
        m_appletBrowserDragStart = me->globalPos();
    } else if (event->type() == QEvent::MouseMove && m_appletBrowserDragStart != QPoint()) {
        QMouseEvent *me = static_cast<QMouseEvent *>(event);
        QPoint newPos = me->globalPos();
        QPoint curPos = m_appletBrowser->pos();
        int x = curPos.x();
        int y = curPos.y();

        if (curPos.y() == 0 || curPos.y() + m_appletBrowser->height() >= height()) {
           x = curPos.x() + (newPos.x() - m_appletBrowserDragStart.x());
           if (x < 0) {
               x = 0;
           } else if (x + m_appletBrowser->width() > width()) {
               x = width() - m_appletBrowser->width();
           }
        }

        if (x == 0 || x + m_appletBrowser->width() >= width()) {
            y = m_appletBrowser->y() + (newPos.y() - m_appletBrowserDragStart.y());

            if (y < 0) {
                y = 0;
            } else if (y + m_appletBrowser->height() > height()) {
                y = height() - m_appletBrowser->height();
            }
        }
        m_appletBrowser->move(x, y);
        m_appletBrowserDragStart = newPos;
    } else if (event->type() == QEvent::MouseButtonRelease) {
        m_appletBrowserDragStart = QPoint();
    }

    return false;
}

void SaverView::showView()
{
    if (isHidden()) {
        if (m_suppressShow) {
            kDebug() << "show was suppressed";
            return;
        }

        setWindowState(Qt::WindowFullScreen);
        //KWindowSystem::setOnAllDesktops(winId(), true);
        //KWindowSystem::setState(winId(), NET::KeepAbove|NET::SkipTaskbar);

        m_hideAction->setEnabled(true);

        show();
        raise();

        m_suppressShow = true;
        QTimer::singleShot(SUPPRESS_SHOW_TIMEOUT, this, SLOT(suppressShowTimeout()));
        containment()->openToolBox();
    }
}

void SaverView::setContainment(Plasma::Containment *newContainment)
{
    if (newContainment == containment()) {
        return;
    }

    containment()->removeToolBoxTool(m_hideAction);
    newContainment->addToolBoxTool(m_hideAction);

    if (isVisible()) {
        disconnect(containment(), SIGNAL(showAddWidgetsInterface(QPointF)), this, SLOT(showAppletBrowser()));
        containment()->closeToolBox();

        connect(newContainment, SIGNAL(showAddWidgetsInterface(QPointF)), this, SLOT(showAppletBrowser()));
        newContainment->openToolBox();
    }

    if (m_appletBrowser) {
        m_appletBrowser->setContainment(newContainment);
    }

    View::setContainment(newContainment);
}

void SaverView::hideView()
{
    if (isHidden()) {
        return;
    }
    if (m_appletBrowser) {
        m_appletBrowser->hide();
    }

    disconnect(containment(), SIGNAL(showAddWidgetsInterface(QPointF)), this, SLOT(showAppletBrowser()));

    containment()->closeToolBox();
    m_hideAction->setEnabled(false);
    hide();
    //let the lockprocess know
    emit hidden();
}

void SaverView::suppressShowTimeout()
{
    kDebug() << "SaverView::suppressShowTimeout";
    m_suppressShow = false;
}

void SaverView::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        hideView();
        event->accept();
        return;
    }

    kDebug() << event->key() << event->spontaneous();
    Plasma::View::keyPressEvent(event);
}

//eeeeew. why did dashboard ever have this? wtf!
void SaverView::showEvent(QShowEvent *event)
{
    //KWindowSystem::setState(winId(), NET::SkipPager);
    connect(containment(), SIGNAL(showAddWidgetsInterface(QPointF)), this, SLOT(showAppletBrowser()));
    Plasma::View::showEvent(event);
}

#include "saverview.moc"

