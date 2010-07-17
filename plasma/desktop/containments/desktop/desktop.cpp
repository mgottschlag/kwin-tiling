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

#include <KDebug>

#include <Plasma/Corona>

using namespace Plasma;

DefaultDesktop::DefaultDesktop(QObject *parent, const QVariantList &args)
    : Containment(parent, args),
      dropping(false),
      m_startupCompleted(false)
{
    qRegisterMetaType<QImage>("QImage");
    qRegisterMetaType<QPersistentModelIndex>("QPersistentModelIndex");

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

        connect(this, SIGNAL(appletAdded(Plasma::Applet *, const QPointF &)),
                this, SLOT(onAppletAdded(Plasma::Applet *, const QPointF &)));
        connect(this, SIGNAL(appletRemoved(Plasma::Applet *)),
                this, SLOT(onAppletRemoved(Plasma::Applet *)));

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
    m_layout->itemTransformed((Applet *)sender(), DesktopLayout::ItemTransformUser);
    m_layout->adjustPhysicalPositions();
}

void DefaultDesktop::onAppletTransformedItself()
{
    m_layout->itemTransformed((Applet *)sender(), DesktopLayout::ItemTransformSelf);
    m_layout->adjustPhysicalPositions();
}

void DefaultDesktop::refreshWorkingArea()
{
    Corona *c = corona();
    if (!c) {
        //kDebug() << "no corona?!";
        QTimer::singleShot(100, this, SLOT(refreshWorkingArea()));
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
        workingGeom = geometry();
        workingGeom = mapFromScene(workingGeom).boundingRect();
        //kDebug() << "defaults due to no screen; got:" << workingGeom;
    }

    if (workingGeom.isValid()) {
        //kDebug() << "!!!!!!!!!!!!! workingGeom is" << workingGeom;
        m_layout->setWorkingArea(workingGeom);
        m_layout->adjustPhysicalPositions();
    } else {
        QTimer::singleShot(100, this, SLOT(refreshWorkingArea()));
    }
}

void DefaultDesktop::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    dropping = true;
    Containment::dropEvent(event);
    dropping = false;
}

K_EXPORT_PLASMA_APPLET(desktop, DefaultDesktop)

#include "desktop.moc"
