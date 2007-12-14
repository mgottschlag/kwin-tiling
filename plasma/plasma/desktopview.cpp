/*
 *   Copyright 2007 Aaron Seigo <aseigo@kde.org>
 *   Copyright 2007 Matt Broadstone <mbroadst@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
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

#include "desktopview.h"

#include <QAction>
#include <QDesktopWidget>
#include <QFile>
#include <QWheelEvent>
#include <QCoreApplication>

#include <KAuthorized>
#include <KMenu>
#include <KRun>
#include <KToggleAction>
#include <KWindowSystem>

#include "plasma/applet.h"
#include "plasma/appletbrowser.h"
#include "plasma/corona.h"
#include "plasma/containment.h"
#include "plasma/svg.h"

#include "dashboardview.h"
#include "plasmaapp.h"

DesktopView::DesktopView(int screen, QWidget *parent)
    : Plasma::View(screen, PlasmaApp::self()->corona(), parent),
      m_zoomLevel(Plasma::DesktopZoom),
      m_appletBrowser(0),
      m_dashboard(0)
{
    if (containment()) {
        connect(containment(), SIGNAL(zoomIn()), this, SLOT(zoomIn()));
        connect(containment(), SIGNAL(zoomOut()), this, SLOT(zoomOut()));
        connect(containment(), SIGNAL(showAddWidgets()), this, SLOT(showAppletBrowser()));
        containment()->enableToolBoxTool("zoomIn", false);
    }
}

DesktopView::~DesktopView()
{
    delete m_dashboard;
}

void DesktopView::toggleDashboard()
{
    if (!m_dashboard) {
        m_dashboard = new DashboardView(screen(), 0);
        m_dashboard->setGeometry(geometry());
    }

    m_dashboard->toggleVisibility();
    kDebug() << "toggling dashboard for screen" << screen() << m_dashboard->isVisible();
}

void DesktopView::adjustSize()
{
    // adapt to screen resolution changes
    QDesktopWidget *desktop = QApplication::desktop();
    QRect geom = desktop->screenGeometry(screen());
    setGeometry(geom);

    if (m_dashboard) {
        m_dashboard->setGeometry(geom);
    }
}

void DesktopView::zoomIn()
{
    containment()->enableToolBoxTool("zoomOut", true);
    qreal s = Plasma::scalingFactor(m_zoomLevel) / matrix().m11();
    if (m_zoomLevel == Plasma::GroupZoom) {
        containment()->enableToolBoxTool("zoomIn", false);
        m_zoomLevel = Plasma::DesktopZoom;
        s = Plasma::scalingFactor(m_zoomLevel) / matrix().m11();
        setSceneRect(geometry());
    } else if (m_zoomLevel == Plasma::OverviewZoom) {
        containment()->enableToolBoxTool("zoomIn", true);
        m_zoomLevel = Plasma::GroupZoom;
        qreal factor = Plasma::scalingFactor(m_zoomLevel);
        s = factor / matrix().m11();
        setSceneRect(QRectF(0, 0, width() * 1.0/factor, height() * 1.0/factor));
    } else {
        containment()->enableToolBoxTool("zoomIn", false);
        return;
    }

    scale(s, s);
}

void DesktopView::zoomOut()
{
    containment()->enableToolBoxTool("zoomIn", true);
    if (m_zoomLevel == Plasma::DesktopZoom) {
        containment()->enableToolBoxTool("zoomOut", true);
        m_zoomLevel = Plasma::GroupZoom;
    } else if (m_zoomLevel == Plasma::GroupZoom) {
        containment()->enableToolBoxTool("zoomOut", false);
        m_zoomLevel = Plasma::OverviewZoom;
    } else {
        containment()->enableToolBoxTool("zoomOut", false);
        return;
    }

    qreal factor = Plasma::scalingFactor(m_zoomLevel);
    qreal s = factor / matrix().m11();
    setSceneRect(QRectF(0, 0, width() * 1.0/factor, height() * 1.0/factor));
    scale(s, s);
}

void DesktopView::showAppletBrowser()
{
    if (m_dashboard && m_dashboard->isVisible()) {
        return;
    }

    if (!m_appletBrowser) {
        m_appletBrowser = new Plasma::AppletBrowser(containment(), this);
        m_appletBrowser->setApplication();
        m_appletBrowser->setAttribute(Qt::WA_DeleteOnClose);
        m_appletBrowser->setWindowTitle(i18n("Add Widgets"));
        m_appletBrowser->setWindowIcon(KIcon("plasmagik"));
        connect(m_appletBrowser, SIGNAL(destroyed()), this, SLOT(appletBrowserDestroyed()));
    }

    KWindowSystem::setOnDesktop(m_appletBrowser->winId(), KWindowSystem::currentDesktop());
    m_appletBrowser->show();
    KWindowSystem::activateWindow(m_appletBrowser->winId());
}

void DesktopView::appletBrowserDestroyed()
{
    m_appletBrowser = 0;
}

void DesktopView::wheelEvent(QWheelEvent* event)
{
    if (scene() && scene()->itemAt(event->pos())) {
        QGraphicsView::wheelEvent(event);
        return;
    }

    if (event->modifiers() & Qt::ControlModifier) {
        if (event->delta() < 0) {
            zoomOut();
        } else {
            zoomIn();
        }
    }
}

#include "desktopview.moc"

