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

#include "savercorona.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QGraphicsLayout>

#include <KDebug>
#include <KDialog>
#include <KStandardDirs>

#include <plasma/containment.h>

SaverCorona::SaverCorona(QObject *parent)
    : Plasma::Corona(parent)
{
    init();
}

void SaverCorona::init()
{
    QDesktopWidget *desktop = QApplication::desktop();
    m_numScreens = desktop->numScreens();
    if (m_numScreens > 1) {
        kDebug() << "maybe someone should implement multiple screen support";
    }
}

void SaverCorona::loadDefaultLayout()
{
    kDebug();
    QString defaultConfig = KStandardDirs::locate("appdata", "plasma-overlay-default-layoutrc");
    if (!defaultConfig.isEmpty()) {
        kDebug() << "attempting to load the default layout from:" << defaultConfig;
        loadLayout(defaultConfig);
        return;
    }

    QDesktopWidget *desktop = QApplication::desktop();

    // create a containment for the screen
    QRect g = desktop->screenGeometry(0);
    kDebug() << "     screen geometry is" << g;
    Plasma::Containment *c = addContainment("saverdesktop");
    c->setScreen(0);
    c->setFormFactor(Plasma::Planar);
    c->flushPendingConstraintsEvents();

    // a default clock
    Plasma::Applet *clock =  Plasma::Applet::load("clock", c->id() + 1);
    c->addApplet(clock, QPointF(KDialog::spacingHint(), KDialog::spacingHint()), true);
    clock->init();
    clock->flushPendingConstraintsEvents();

    emit containmentAdded(c);

}

#include "savercorona.moc"

