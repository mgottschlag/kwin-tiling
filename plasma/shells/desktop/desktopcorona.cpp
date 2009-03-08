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
#include <QDir>
#include <QGraphicsLayout>
#include <QTimer>

#include <KDebug>
#include <KDialog>
#include <KGlobalSettings>
#include <KStandardDirs>
#include <KWindowSystem>

#include <Plasma/Containment>
#include <Plasma/DataEngineManager>

#include <kephal/screens.h>

#include "panelview.h"
#include "plasmaapp.h"
#include "plasma-shell-desktop.h"

DesktopCorona::DesktopCorona(QObject *parent)
    : Plasma::Corona(parent)
{
    init();
}

void DesktopCorona::init()
{
    Kephal::Screens *screens = Kephal::Screens::self();
    connect(screens, SIGNAL(screenAdded(Kephal::Screen *)), SLOT(screenAdded(Kephal::Screen *)));
    connect(KWindowSystem::self(), SIGNAL(workAreaChanged()), this, SIGNAL(availableScreenRegionChanged()));
}

void DesktopCorona::checkScreens()
{
    // quick sanity check to ensure we have containments for each screen
    int numScreens = Kephal::ScreenUtils::numScreens();
    for (int i = 0; i < numScreens; ++i) {
        checkScreen(i);
    }
}

void DesktopCorona::checkScreen(int screen, bool signalWhenExists)
{
    if (AppSettings::perVirtualDesktopViews()) {
        int numDesktops = KWindowSystem::numberOfDesktops();

        for (int j = 0; j < numDesktops; ++j) {
            Plasma::Containment *c = containmentForScreen(screen, j);

            if (!c) {
                addDesktopContainment(screen, j);
            } else if (signalWhenExists) {
                emit containmentAdded(c);
            }
        }
    } else {
        Plasma::Containment *c = containmentForScreen(screen);
        if (!containmentForScreen(screen)) {
            addDesktopContainment(screen);
        } else if (signalWhenExists) {
            emit containmentAdded(c);
        }
    }

    if (signalWhenExists) {
        foreach (Plasma::Containment * c, containments()) {
            if (c->screen() != screen) {
                continue;
            }

            Plasma::Containment::Type t = c->containmentType();
            if (t == Plasma::Containment::PanelContainment ||
                    t == Plasma::Containment::CustomPanelContainment) {
                emit containmentAdded(c);
            }
        }
    }
}

void DesktopCorona::addDesktopContainment(int screen, int desktop)
{
    kDebug() << screen << desktop;
    Plasma::Containment* c = addContainment("desktop");
    c->setScreen(screen, desktop);
    c->setFormFactor(Plasma::Planar);
    c->flushPendingConstraintsEvents();
    c->setActivity(i18n("Desktop"));
    emit containmentAdded(c);
}

int DesktopCorona::numScreens() const
{
    return Kephal::ScreenUtils::numScreens();
}

QRect DesktopCorona::screenGeometry(int id) const
{
    return Kephal::ScreenUtils::screenGeometry(id);
}

QRegion DesktopCorona::availableScreenRegion(int id) const
{
    QRegion r(screenGeometry(id));
    foreach (PanelView *view, PlasmaApp::self()->panelViews()) {
        if (view->screen() == id && view->visibilityMode() == PanelView::NormalPanel) {
            r = r.subtracted(view->geometry());
        }
    }

    return r;
}

void DesktopCorona::loadDefaultLayout()
{
    QString defaultConfig = KStandardDirs::locate("appdata", "plasma-default-layoutrc");
    if (!defaultConfig.isEmpty()) {
        kDebug() << "attempting to load the default layout from:" << defaultConfig;
        loadLayout(defaultConfig);
        return;
    }

    kDebug() << "number of screens is" << Kephal::ScreenUtils::numScreens();
    int topLeftScreen = 0;
    QPoint topLeftCorner = Kephal::ScreenUtils::screenGeometry(0).topLeft();

    // find our "top left" screen, use it as the primary
    for (int i = 0; i < Kephal::ScreenUtils::numScreens(); ++i) {
        QRect g = Kephal::ScreenUtils::screenGeometry(i);
        kDebug() << "     screen " << i << "geometry is" << g;

        if (g.x() <= topLeftCorner.x() && g.y() >= topLeftCorner.y()) {
            topLeftCorner = g.topLeft();
            topLeftScreen = i;
        }
    }

    // create a containment for each screen
    for (int i = 0; i < Kephal::ScreenUtils::numScreens(); ++i) {
        // passing in an empty string will get us whatever the default
        // containment type is!
        Plasma::Containment* c = addContainmentDelayed(QString());

        if (!c) {
            continue;
        }

        c->init();
        c->setScreen(i, 0);
        c->setWallpaper("image", "SingleImage");
        c->setFormFactor(Plasma::Planar);
        c->updateConstraints(Plasma::StartupCompletedConstraint);
        c->flushPendingConstraintsEvents();

        // put a folder view on the first screen
        if (i == topLeftScreen) {
            QString desktopPath = KGlobalSettings::desktopPath();
            QDir desktopFolder(desktopPath);
            if (desktopPath != QDir::homePath() && desktopFolder.exists()) {
                Plasma::Applet *folderView =  Plasma::Applet::load("folderview", c->id() + 1);
                if (folderView) {
                    c->addApplet(folderView, QPointF(KDialog::spacingHint(), KDialog::spacingHint()), true);
                    KConfigGroup config = folderView->config();
                    config.writeEntry("url", "desktop:/");
                }
            }
        }

        emit containmentAdded(c);
    }

    // make a panel at the bottom
    Plasma::Containment *panel = addContainmentDelayed("panel");

    if (!panel) {
        return;
    }

    panel->init();
    panel->setScreen(topLeftScreen);
    panel->setLocation(Plasma::BottomEdge);
    panel->updateConstraints(Plasma::StartupCompletedConstraint);
    panel->flushPendingConstraintsEvents();

    // some default applets to get a usable UI
    Plasma::Applet *applet = loadDefaultApplet("launcher", panel);
    if (applet) {
        applet->setGlobalShortcut(KShortcut("Alt+F1"));
    }

    loadDefaultApplet("notifier", panel);
    loadDefaultApplet("pager", panel);
    loadDefaultApplet("tasks", panel);
    loadDefaultApplet("systemtray", panel);

    Plasma::DataEngineManager *engines = Plasma::DataEngineManager::self();
    Plasma::DataEngine *power = engines->loadEngine("powermanagement");
    if (power) {
        const QStringList &batteries = power->query("Battery")["sources"].toStringList();
        if (!batteries.isEmpty()) {
            loadDefaultApplet("battery", panel);
        }
    }
    engines->unloadEngine("powermanagement");

    loadDefaultApplet("digital-clock", panel);
    emit containmentAdded(panel);

    QTimer::singleShot(1000, this, SLOT(saveDefaultSetup()));
}

void DesktopCorona::saveDefaultSetup()
{
    // a "null" KConfigGroup is used to force a save into the config file
    KConfigGroup invalidConfig;

    foreach (Plasma::Containment *containment, containments()) {
        containment->save(invalidConfig);

        foreach (Plasma::Applet* applet, containment->applets()) {
            applet->init();
            applet->flushPendingConstraintsEvents();
            applet->save(invalidConfig);
        }
    }

    requestConfigSync();
}

Plasma::Applet *DesktopCorona::loadDefaultApplet(const QString &pluginName, Plasma::Containment *c)
{
    QVariantList args;
    Plasma::Applet *applet = Plasma::Applet::load(pluginName, 0, args);

    if (applet) {
        c->addApplet(applet);
    }

    return applet;
}

void DesktopCorona::screenAdded(Kephal::Screen *s)
{
    kDebug() << s->id();
    checkScreen(s->id(), true);
}

#include "desktopcorona.moc"

