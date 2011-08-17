/*
*   Copyright 2007 by Aaron Seigo <aseigo@kde.org>
*   Copyright 2008 by Alexis Ménard <darktears31@gmail.com>
*
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

#include "desktop.h"

#include "krunner_interface.h"

#include <KDebug>

#include <Plasma/Corona>

#include <QTimer>

using namespace Plasma;

DefaultDesktop::DefaultDesktop(QObject *parent, const QVariantList &args)
    : Containment(parent, args),
      m_refreshFails(0),
      dropping(false),
      m_startupCompleted(false)
{
    qRegisterMetaType<QImage>("QImage");
    qRegisterMetaType<QPersistentModelIndex>("QPersistentModelIndex");

    m_delayedRefreshTimer = new QTimer(this);
    m_delayedRefreshTimer->setSingleShot(true);

    m_layout = new DesktopLayout;
    m_layout->setAlignment(Qt::AlignTop|Qt::AlignLeft);
    m_layout->setPlacementSpacing(20);
    m_layout->setScreenSpacing(0);
    m_layout->setShiftingSpacing(0);
    m_layout->setTemporaryPlacement(true);
    m_layout->setVisibilityTolerance(0.5);

    resize(800, 600);

    setHasConfigurationInterface(true);
    //kDebug() << "!!! loading desktop";
}

DefaultDesktop::~DefaultDesktop()
{
}

void DefaultDesktop::constraintsEvent(Plasma::Constraints constraints)
{
    if (constraints & Plasma::StartupCompletedConstraint) {
        if (m_startupCompleted) {
            return;
        }

        m_startupCompleted = true;

        connect(corona(), SIGNAL(availableScreenRegionChanged()),
                this, SLOT(refreshWorkingArea()));
        refreshWorkingArea();

        connect(this, SIGNAL(appletAdded(Plasma::Applet*,QPointF)),
                this, SLOT(onAppletAdded(Plasma::Applet*,QPointF)));
        connect(this, SIGNAL(appletRemoved(Plasma::Applet*)),
                this, SLOT(onAppletRemoved(Plasma::Applet*)));

        foreach (Applet *applet, applets()) {
            m_layout->addItem(applet, true, false);
            connect(applet, SIGNAL(appletTransformedByUser()), this, SLOT(onAppletTransformedByUser()));
            connect(applet, SIGNAL(appletTransformedItself()), this, SLOT(onAppletTransformedItself()));
        }

        m_layout->adjustPhysicalPositions();
    } else if ((constraints & Plasma::SizeConstraint) || (constraints & Plasma::ScreenConstraint)) {
        refreshWorkingArea();
    }
}

void DefaultDesktop::onAppletAdded(Plasma::Applet *applet, const QPointF &pos)
{
    if (dropping || pos != QPointF(-1,-1) || applet->geometry().topLeft() != QPointF(0,0)) {
        // add item to the layout using the current position
        m_layout->addItem(applet, true, false);
    } else {
        // auto-position
        m_layout->addItem(applet, true, true);
    }

    m_layout->adjustPhysicalPositions();

    connect(applet, SIGNAL(appletTransformedByUser()), this, SLOT(onAppletTransformedByUser()));
    connect(applet, SIGNAL(appletTransformedItself()), this, SLOT(onAppletTransformedItself()));
}

void DefaultDesktop::onAppletRemoved(Plasma::Applet *applet)
{
    for (int i=0; i < m_layout->count(); i++) {
        if (applet == m_layout->itemAt(i)) {
            m_layout->removeAt(i);
            m_layout->adjustPhysicalPositions();
            return;
        }
    }
}

void DefaultDesktop::onAppletTransformedByUser()
{
    Plasma::Applet *applet = static_cast<Plasma::Applet *>(sender());
    m_layout->itemTransformed(applet, DesktopLayout::ItemTransformUser);
    m_layout->adjustPhysicalPositions(applet);
}

void DefaultDesktop::onAppletTransformedItself()
{
    Plasma::Applet *applet = static_cast<Plasma::Applet *>(sender());
    m_layout->itemTransformed(applet, DesktopLayout::ItemTransformUser);
    m_layout->adjustPhysicalPositions(applet);
}

void DefaultDesktop::refreshWorkingArea()
{
    m_delayedRefreshTimer->stop();

    Corona *c = corona();
    if (!c) {
        //kDebug() << "no corona?!";
        m_delayedRefreshTimer->start(DELAYED_REFRESH_WAIT);
        return;
    }

    QRectF workingGeom;
    if (screen() != -1 && screen() < c->numScreens()) {
        // we are associated with a screen, make sure not to overlap panels
        workingGeom = c->availableScreenRegion(screen()).boundingRect();
        //kDebug() << "got" << workingGeom;
        // From screen coordinates to containment coordinates
        workingGeom.translate(-c->screenGeometry(screen()).topLeft());
    } else {
        workingGeom = mapFromScene(geometry()).boundingRect();
        //kDebug() << "defaults due to no screen; got:" << workingGeom;
    }

    if (workingGeom.isValid()) {
        //kDebug() << "!!!!!!!!!!!!! workingGeom is" << workingGeom;
        m_refreshFails = 0;
        m_layout->setWorkingArea(workingGeom);
        m_layout->adjustPhysicalPositions();
    } else if (m_refreshFails < MAX_REFRESH_FAILS) {
        ++m_refreshFails;
        m_delayedRefreshTimer->start(DELAYED_REFRESH_WAIT);
    }
}

void DefaultDesktop::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    dropping = true;
    Containment::dropEvent(event);
    dropping = false;
}

void DefaultDesktop::keyPressEvent(QKeyEvent *event)
{
    if (focusItem() == this && !event->text().isEmpty()) {
        const QString interface("org.kde.krunner");
        org::kde::krunner::App krunner(interface, "/App", QDBusConnection::sessionBus());
        krunner.query(event->text());
        event->accept();
    } else {
        event->ignore();
    }
}

K_EXPORT_PLASMA_APPLET(desktop, DefaultDesktop)

#include "desktop.moc"
