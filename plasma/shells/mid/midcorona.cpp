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

#include "midcorona.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QDir>
#include <QGraphicsLayout>

#include <KCmdLineArgs>
#include <KDebug>
#include <KDialog>
#include <KGlobalSettings>
#include <KStandardDirs>

#include <plasma/containment.h>
#include <plasma/dataenginemanager.h>

MidCorona::MidCorona(QObject *parent)
    : Plasma::Corona(parent)
{
    init();
}

void MidCorona::init()
{
    QDesktopWidget *desktop = QApplication::desktop();
    QObject::connect(desktop, SIGNAL(resized(int)), this, SLOT(screenResized(int)));
}

void MidCorona::checkScreens()
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

void MidCorona::loadDefaultLayout()
{
    QString defaultConfig = KStandardDirs::locate("appdata", "plasma-default-layoutrc");
    if (!defaultConfig.isEmpty()) {
        kDebug() << "attempting to load the default layout from:" << defaultConfig;
        loadLayout(defaultConfig);
        return;
    }

    QDesktopWidget *desktop = QApplication::desktop();

    // find our "top left" screen, use it as the primary

    // used to force a save into the config file
    KConfigGroup invalidConfig;

    // FIXME: need to load the MID-specific containment
    // passing in an empty string will get us whatever the default
    // containment type is!
    Plasma::Containment* c = addContainmentDelayed(QString());

    if (!c) {
        return;
    }

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    bool isDesktop = args->isSet("desktop");

    c->init();

    if (isDesktop) {
        c->setScreen(0);
    } else {
        int width = qMax(400, args->getOption("width").toInt());
        int height = qMax(200, args->getOption("height").toInt());
        c->resize(width, height);
    }

    c->setWallpaper("image", "SingleImage");
    c->setFormFactor(Plasma::Planar);
    c->updateConstraints(Plasma::StartupCompletedConstraint);
    c->flushPendingConstraintsEvents();
    c->save(invalidConfig);

    emit containmentAdded(c);

    /*
    todo: replace with an applet layout at the top, perhaps reserve a WM strut while we're at it?

    loadDefaultApplet("systemtray", panel);

    foreach (Plasma::Applet* applet, panel->applets()) {
        applet->init();
        applet->flushPendingConstraintsEvents();
        applet->save(invalidConfig);
    }
    */

    requestConfigSync();
    /*
    foreach (Plasma::Containment *c, containments()) {
        kDebug() << "letting the world know about" << (QObject*)c;
        emit containmentAdded(c);
    }
    */
}

Plasma::Applet *MidCorona::loadDefaultApplet(const QString &pluginName, Plasma::Containment *c)
{
    QVariantList args;
    Plasma::Applet *applet = Plasma::Applet::load(pluginName, 0, args);

    if (applet) {
        c->addApplet(applet);
    }

    return applet;
}

void MidCorona::screenResized(int screen)
{
    int numScreens = QApplication::desktop()->numScreens();
    if (screen < numScreens) {
        foreach (Plasma::Containment *c, containments()) {
            if (c->screen() == screen) {
                // trigger a relayout
                c->setScreen(screen);
            }
        }
    }
}

#include "midcorona.moc"

