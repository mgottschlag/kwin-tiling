/*
 *   Copyright 2007 Aaron Seigo <aseigo@kde.org>
 *   Copyright 2007 Matt Broadstone <mbroadst@gmail.com>
 *   Copyright 2007 André Duffeck <duffeck@kde.org>
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

#include "dashboardview.h"

#include <QDesktopWidget>
#include <QKeyEvent>
#include <QTimer>

#include <KWindowSystem>

#include "plasma/applet.h"
#include "plasma/corona.h"
#include "plasma/containment.h"
#include "plasma/svg.h"
#include "plasma/appletbrowser.h"
#include "plasma/widgets/pushbutton.h"

#include "plasmaapp.h"

static const int SUPPRESS_SHOW_TIMEOUT = 500; // Number of millis to prevent reshow of dashboard

DashboardView::DashboardView(int screen, QWidget *parent)
    : Plasma::View(screen, PlasmaApp::self()->corona(), parent),
      m_appletBrowser(0),
      m_suppressShow(false),
      m_zoomIn(false),
      m_zoomOut(false)
{
    //setContextMenuPolicy(Qt::NoContextMenu);
    setWindowFlags(Qt::FramelessWindowHint);
    if (!PlasmaApp::hasComposite()) {
        setAutoFillBackground(false);
        setAttribute(Qt::WA_NoSystemBackground);
    }

    QDesktopWidget *desktop = QApplication::desktop();
    setGeometry(desktop->screenGeometry(screen));

    setDrawWallpaper(!PlasmaApp::hasComposite());

    connect(scene(), SIGNAL(launchActivated()), SLOT(hideView()));
    connect(containment(), SIGNAL(showAddWidgets()), this, SLOT(showAppletBrowser()));
    Plasma::Widget *tool = containment()->addToolBoxTool("hideDashboard", "preferences-desktop-display", i18n("Hide Dashboard"));
    containment()->enableToolBoxTool("hideDashboard", false);
    connect(tool, SIGNAL(clicked()), this, SLOT(hideView()));

    installEventFilter(this);
}

DashboardView::~DashboardView()
{
    delete m_appletBrowser;
}

void DashboardView::drawBackground(QPainter * painter, const QRectF & rect)
{
    if (PlasmaApp::hasComposite()) {
        setDrawWallpaper(false);
        painter->setCompositionMode(QPainter::CompositionMode_Source);
        painter->fillRect(geometry(), QColor(0, 0, 0, 180));
    } else {
        setDrawWallpaper(true);
        Plasma::View::drawBackground(painter, rect);
    }
}

void DashboardView::showAppletBrowser()
{
    if (!m_appletBrowser) {
        m_appletBrowser = new Plasma::AppletBrowser(containment(), this, Qt::FramelessWindowHint );
        //TODO: make this proportional to the screen
        m_appletBrowser->setInitialSize(QSize(400, 400));
        m_appletBrowser->setApplication();
        m_appletBrowser->setWindowTitle(i18n("Add Widgets"));
        QPalette p = m_appletBrowser->palette();
        p.setBrush(QPalette::Background, QBrush(QColor(0, 0, 0, 180)));
        m_appletBrowser->setPalette(p);
        m_appletBrowser->setBackgroundRole(QPalette::Background);
        m_appletBrowser->setAutoFillBackground(true);
        KWindowSystem::setState(m_appletBrowser->winId(), NET::KeepAbove|NET::SkipTaskbar);
        m_appletBrowser->move(0, 0);
        m_appletBrowser->installEventFilter(this);
    }

    m_appletBrowser->setHidden(m_appletBrowser->isVisible());
}

void DashboardView::appletBrowserDestroyed()
{
    m_appletBrowser = 0;
}

bool DashboardView::eventFilter(QObject *watched, QEvent *event)
{
    if (watched != m_appletBrowser) {
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *me = static_cast<QMouseEvent *>(event);
            if (me->button() == Qt::LeftButton) {
                hideView();
            }
        }
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

void DashboardView::toggleVisibility()
{
    if (isHidden()) {
        if (m_suppressShow) {
            kDebug() << "DashboardView::toggleVisibility but show was suppressed";
            return;
        }

        setWindowState(Qt::WindowFullScreen);
        KWindowSystem::setOnAllDesktops(winId(), true);
        KWindowSystem::setState(winId(), NET::KeepAbove|NET::SkipTaskbar);

        m_zoomOut = containment()->isToolboxToolEnabled("zoomOut");
        m_zoomIn = containment()->isToolboxToolEnabled("zoomIn");

        containment()->enableToolBoxTool("hideDashboard", true);
        containment()->enableToolBoxTool("zoomOut", false);
        containment()->enableToolBoxTool("zoomIn", false);

        show();
        raise();

        m_suppressShow = true;
        QTimer::singleShot(SUPPRESS_SHOW_TIMEOUT, this, SLOT(suppressShowTimeout()));
        containment()->showToolbox();
    } else {
        hideView();
    }
}

void DashboardView::hideView()
{
    if (m_appletBrowser) {
        m_appletBrowser->hide();
    }

    disconnect(KWindowSystem::self(), SIGNAL(activeWindowChanged(WId)), this, SLOT(activeWindowChanged(WId)));

    containment()->hideToolbox();
    containment()->enableToolBoxTool("zoomOut", m_zoomOut);
    containment()->enableToolBoxTool("zoomIn", m_zoomIn);
    containment()->enableToolBoxTool("hideDashboard", false);
    hide();
}

void DashboardView::suppressShowTimeout()
{
    kDebug() << "DashboardView::suppressShowTimeout";
    m_suppressShow = false;
}

void DashboardView::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        hideView();
    }

    Plasma::View::keyPressEvent(event);
}

void DashboardView::activeWindowChanged(WId id)
{
    if (id != winId() &&
        (!m_appletBrowser || id != m_appletBrowser->winId())) {
        hideView();
    }
}

void DashboardView::showEvent(QShowEvent *event)
{
    KWindowSystem::setState(winId(), NET::SkipPager);
    connect(KWindowSystem::self(), SIGNAL(activeWindowChanged(WId)), this, SLOT(activeWindowChanged(WId)));
    Plasma::View::showEvent(event);
}

#include "dashboardview.moc"

