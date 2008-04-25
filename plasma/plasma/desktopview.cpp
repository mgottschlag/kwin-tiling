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
#include "plasma/corona.h"
#include "plasma/containment.h"
#include "plasma/svg.h"

#include "dashboardview.h"
#include "plasmaapp.h"

DesktopView::DesktopView(Plasma::Containment *containment, int id, QWidget *parent)
    : Plasma::View(containment, id, parent),
      m_zoomLevel(Plasma::DesktopZoom),
      m_dashboard(0)
{
    connectContainment(containment);
    if (containment) {
        containment->enableToolBoxTool("zoomIn", false);
        containment->enableToolBoxTool("addSiblingContainment", false);
    }
}

DesktopView::~DesktopView()
{
    delete m_dashboard;
}

void DesktopView::connectContainment(Plasma::Containment *containment)
{
    if (containment) {
        connect(containment, SIGNAL(zoomIn(Plasma::Containment *)), this, SLOT(zoomIn(Plasma::Containment *)));
        connect(containment, SIGNAL(zoomOut()), this, SLOT(zoomOut()));
        connect(containment, SIGNAL(showAddWidgetsInterface(QPointF)), this, SLOT(showAppletBrowser()));
        connect(containment, SIGNAL(addSiblingContainment(Plasma::Containment *)), this, SLOT(addContainment(Plasma::Containment *)));
    }
}

void DesktopView::toggleDashboard()
{
    if (!m_dashboard) {
        if (!containment()) {
            return;
        }

        m_dashboard = new DashboardView(containment(), 0);
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

void DesktopView::addContainment(Plasma::Containment *fromContainment)
{
    if (fromContainment) {
        Plasma::Corona *corona = fromContainment->corona();
        if (corona) {
            //make it the same type of containment
            Plasma::Containment *c = corona->addContainment(fromContainment->pluginName());
            //it doesn't know what size to be because it's never had a screen
            //c->resize(fromContainment->size());
            connectContainment(c); //TODO put this in our own setContainment
            //note: this will set a sane size too, assuming we have a screen
            setContainment(c);
            kDebug() << "containment added at" << c->geometry();
        }
    }
}

void DesktopView::zoomIn(Plasma::Containment *toContainment)
{
    kDebug();
    if (containment()) {
        containment()->enableToolBoxTool("zoomOut", true);
    }
    if (toContainment && containment() != toContainment) {
        setContainment(toContainment);
        if (m_dashboard) {
            m_dashboard->setContainment(toContainment);
        }
    }
    if (m_zoomLevel == Plasma::GroupZoom) {
        setDragMode(NoDrag);
        m_zoomLevel = Plasma::DesktopZoom;
        qreal factor = Plasma::scalingFactor(m_zoomLevel) / matrix().m11();
        scale(factor, factor);
        if (containment()) {
            //disconnect from other containments
            Plasma::Corona *corona = containment()->corona();
            if (corona) {
                QList<Plasma::Containment*> containments = corona->containments();
                foreach (Plasma::Containment *c, containments) {
                    if (c == containment() || c->containmentType() == Plasma::Containment::PanelContainment) {
                        continue;
                    }
                    disconnect(c, 0, this, 0);
                }
            }
            setSceneRect(containment()->geometry());
            containment()->closeToolBox();
            containment()->enableToolBoxTool("zoomIn", false);
            containment()->enableToolBoxTool("addSiblingContainment", false);
        }
    } else if (m_zoomLevel == Plasma::OverviewZoom) {
        m_zoomLevel = Plasma::GroupZoom;
        qreal factor = Plasma::scalingFactor(m_zoomLevel);
        factor = factor / matrix().m11();
        scale(factor, factor);
        setSceneRect(QRectF(0, 0, scene()->sceneRect().right(), scene()->sceneRect().bottom()));

        if (containment()) {
            containment()->enableToolBoxTool("zoomIn", true);
            containment()->enableToolBoxTool("addSiblingContainment", true);
            ensureVisible(containment()->sceneBoundingRect());
        }
    } else {
        setDragMode(NoDrag);
        if (containment()) {
            containment()->closeToolBox();
            containment()->enableToolBoxTool("zoomIn", false);
            containment()->enableToolBoxTool("addSiblingContainment", false);
        }
    }
}

void DesktopView::zoomOut()
{
    containment()->enableToolBoxTool("zoomIn", true);
    containment()->enableToolBoxTool("addSiblingContainment", true);
    if (m_zoomLevel == Plasma::DesktopZoom) {
        containment()->enableToolBoxTool("zoomOut", true);
        m_zoomLevel = Plasma::GroupZoom;
        //connect to other containments
        //FIXME if some other view is zoomed out, a little madness will ensue
        Plasma::Corona *corona = containment()->corona();
        if (corona) {
            QList<Plasma::Containment*> containments = corona->containments();
            foreach (Plasma::Containment *c, containments) {
                if (c == containment() || c->containmentType() == Plasma::Containment::PanelContainment || c->screen() > -1) {
                    continue;
                }
                connectContainment(c);
            }
        }
    } else if (m_zoomLevel == Plasma::GroupZoom) {
        containment()->enableToolBoxTool("zoomOut", false);
        m_zoomLevel = Plasma::OverviewZoom;
    } else {
        containment()->enableToolBoxTool("zoomOut", false);
        return;
    }

    setDragMode(ScrollHandDrag);
    qreal factor = Plasma::scalingFactor(m_zoomLevel);
    qreal s = factor / matrix().m11();
    scale(s, s);
    setSceneRect(QRectF(0, 0, scene()->sceneRect().right(), scene()->sceneRect().bottom()));

    if (containment()) {
        ensureVisible(containment()->sceneBoundingRect());
    }
}

void DesktopView::showAppletBrowser()
{
    if (m_dashboard && m_dashboard->isVisible()) {
        return;
    }

    PlasmaApp::self()->showAppletBrowser(containment());
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

