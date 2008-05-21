/*
 *   Copyright 2008 Aaron Seigo <aseigo@kde.org>
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

#include "desktopcorona.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QGraphicsLayout>

#include <KDebug>

#include <plasma/containment.h>

DesktopCorona::DesktopCorona(QObject *parent)
    : Plasma::Corona(parent)
{
    init();
}

void DesktopCorona::init()
{
    QDesktopWidget *desktop = QApplication::desktop();
    m_numScreens = desktop->numScreens();
    QObject::connect(desktop, SIGNAL(resized(int)), this, SLOT(screenResized(int)));
}

void DesktopCorona::checkScreens()
{
    // quick sanity check to ensure we have containments for each screen!
    int numScreens = QApplication::desktop()->numScreens();
    for (int i = 0; i < numScreens; ++i) {
        if (!containmentForScreen(i)) {
            //TODO: should we look for containments that aren't asigned but already exist?
            Plasma::Containment* c = addContainment("desktop");
            c->setScreen(i);
            c->setFormFactor(Plasma::Planar);
            c->flushPendingConstraintsEvents();
        } else if (i >= m_numScreens) {
            // now ensure that if our screen count changed we actually get views
            // for them, even if the Containment already existed for that screen
            // so we "lie" and emit a containmentAdded signal for every new screen
            // regardless of whether it actually already existed, or just got added
            // and therefore had this signal emitted. plasma can handle such
            // multiple emissions of the signal, and this is simply the most
            // straightforward way of accomplishing this
            kDebug() << "Notifying of new screen: " << i;
            emit containmentAdded(containmentForScreen(i));
        }
    }

    m_numScreens = numScreens;
}

void DesktopCorona::loadDefaultLayout()
{
    //FIXME: implement support for system-wide defaults
    QDesktopWidget *desktop = QApplication::desktop();
    int numScreens = desktop->numScreens();
    kDebug() << "number of screens is" << numScreens;
    int topLeftScreen = 0;
    QPoint topLeftCorner = desktop->screenGeometry(0).topLeft();

    // create a containment for each screen
    for (int i = 0; i < numScreens; ++i) {
        QRect g = desktop->screenGeometry(i);
        kDebug() << "     screen " << i << "geometry is" << g;
        Plasma::Containment* c = addContainment("desktop");
        c->setScreen(i);
        c->setFormFactor(Plasma::Planar);
        c->flushPendingConstraintsEvents();
        emit containmentAdded(c);

        if (g.x() <= topLeftCorner.x() && g.y() >= topLeftCorner.y()) {
            topLeftCorner = g.topLeft();
            topLeftScreen = i;
        }
    }

    // make a panel at the bottom
    Plasma::Containment* panel = addContainment("panel");
    panel->setScreen(topLeftScreen);
    panel->setLocation(Plasma::BottomEdge);
    panel->flushPendingConstraintsEvents();

    // some default applets to get a usable UI
    panel->addApplet("launcher");
    panel->addApplet("notifier");
    panel->addApplet("pager");
    panel->addApplet("tasks");
    panel->addApplet("digital-clock");
    panel->addApplet("systemtray");
}

void DesktopCorona::screenResized(int screen)
{
    int numScreens = QApplication::desktop()->numScreens();
    if (screen < numScreens) {
        foreach (Plasma::Containment *c, containments()) {
            if (c->screen() == screen) {
                // trigger a relayout
                c->setScreen(screen);
            }
        }

        checkScreens(); // ensure we have containments for every screen
    } else {
        m_numScreens = numScreens;
    }
}

#include "desktopcorona.moc"

