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
    kDebug();
    QDesktopWidget *desktop = QApplication::desktop();
    m_numScreens = desktop->numScreens();
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
    int numScreens = desktop->numScreens();
    kDebug() << "number of screens is" << numScreens;
    int topLeftScreen = 0;
    QPoint topLeftCorner = desktop->screenGeometry(0).topLeft();

    // create a containment for each screen
    for (int i = 0; i < 1; ++i) {
        QRect g = desktop->screenGeometry(i);
        kDebug() << "     screen " << i << "geometry is" << g;
        Plasma::Containment* c = addContainment("saverdesktop");
        c->setScreen(i);
        c->setFormFactor(Plasma::Planar);
        c->flushPendingConstraintsEvents();

        // put a folder view on the first screen
        if (i == 0) {
            Plasma::Applet *folderView =  Plasma::Applet::load("clock", c->id() + 1);
            c->addApplet(folderView, QPointF(KDialog::spacingHint(), KDialog::spacingHint()), true);
            folderView->init();
            folderView->flushPendingConstraintsEvents();
        }

        if (g.x() <= topLeftCorner.x() && g.y() >= topLeftCorner.y()) {
            topLeftCorner = g.topLeft();
            topLeftScreen = i;
        }

        emit containmentAdded(c);
    }

}

#include "savercorona.moc"

