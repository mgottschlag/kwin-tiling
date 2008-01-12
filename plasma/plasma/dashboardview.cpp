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
      m_appletBrowserWidget(0),
      m_suppressShow(false),
      m_zoomIn(false),
      m_zoomOut(false)
{
    setContextMenuPolicy(Qt::NoContextMenu);
    setWindowFlags(Qt::FramelessWindowHint);
    if (!PlasmaApp::hasComposite()) {
        setAutoFillBackground(false);
        setAttribute(Qt::WA_NoSystemBackground);
    }

    QDesktopWidget *desktop = QApplication::desktop();
    setGeometry(desktop->screenGeometry(screen));

    setDrawWallpaper(!PlasmaApp::hasComposite());
    hide();

    connect(scene(), SIGNAL(launchActivated()), SLOT(hideView()));
    connect(containment(), SIGNAL(showAddWidgets()), this, SLOT(showAppletBrowser()));
    Plasma::Widget *tool = containment()->addToolBoxTool("hideDashboard", "preferences-desktop-display", i18n("Hide Dashboard"));
    containment()->enableToolBoxTool("hideDashboard", false);
    connect(tool, SIGNAL(clicked()), this, SLOT(hideView()));
}

DashboardView::~DashboardView()
{
    delete m_appletBrowserWidget;
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
    if (!m_appletBrowserWidget) {
        m_appletBrowserWidget = new Plasma::AppletBrowserWidget(containment(), true, this, Qt::FramelessWindowHint );
        m_appletBrowserWidget->setApplication();
        m_appletBrowserWidget->setWindowTitle(i18n("Add Widgets"));
        QPalette p = m_appletBrowserWidget->palette();
        p.setBrush(QPalette::Background, QBrush(QColor(0, 0, 0, 180)));
        m_appletBrowserWidget->setPalette(p);
        m_appletBrowserWidget->setBackgroundRole(QPalette::Background);
        m_appletBrowserWidget->setAutoFillBackground(true);
        KWindowSystem::setState(m_appletBrowserWidget->winId(), NET::KeepAbove|NET::SkipTaskbar);
        m_appletBrowserWidget->move(0, 0);
        m_appletBrowserWidget->installEventFilter(this);
    }

    m_appletBrowserWidget->setHidden(m_appletBrowserWidget->isVisible());
}

void DashboardView::appletBrowserDestroyed()
{
    m_appletBrowserWidget = 0;
}

bool DashboardView::eventFilter(QObject *watched, QEvent *event)
{
    if (watched != m_appletBrowserWidget) {
        return false;
    }

    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *me = static_cast<QMouseEvent *>(event);
        m_appletBrowserDragStart = me->globalPos();
    } else if (event->type() == QEvent::MouseMove && m_appletBrowserDragStart != QPoint()) {
        QMouseEvent *me = static_cast<QMouseEvent *>(event);
        QPoint newPos = me->globalPos();
        QPoint curPos = m_appletBrowserWidget->pos();
        int x = curPos.x();
        int y = curPos.y();

        if (curPos.y() == 0 || curPos.y() + m_appletBrowserWidget->height() >= height()) {
           x = curPos.x() + (newPos.x() - m_appletBrowserDragStart.x());
           if (x < 0) {
               x = 0;
           } else if (x + m_appletBrowserWidget->width() > width()) {
               x = width() - m_appletBrowserWidget->width();
           }
        }

        if (x == 0 || x + m_appletBrowserWidget->width() >= width()) {
            y = m_appletBrowserWidget->y() + (newPos.y() - m_appletBrowserDragStart.y());

            if (y < 0) {
                y = 0;
            } else if (y + m_appletBrowserWidget->height() > height()) {
                y = height() - m_appletBrowserWidget->height();
            }
        }
        m_appletBrowserWidget->move(x, y);
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
    } else {
        hideView();
    }
}

void DashboardView::hideView()
{
    if (m_appletBrowserWidget) {
        m_appletBrowserWidget->hide();
    }

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

void DashboardView::showEvent(QShowEvent *event)
{
    KWindowSystem::setState(winId(), NET::SkipPager);

    Plasma::View::showEvent(event);
}

#include "dashboardview.moc"

