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
#include "scripting/scriptengine.h"

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

void DesktopCorona::checkScreens(bool signalWhenExists)
{
    // quick sanity check to ensure we have containments for each screen
    int numScreens = Kephal::ScreenUtils::numScreens();
    for (int i = 0; i < numScreens; ++i) {
        checkScreen(i, signalWhenExists);
    }
}

void DesktopCorona::checkScreen(int screen, bool signalWhenExists)
{
    if (AppSettings::perVirtualDesktopViews()) {
        int numDesktops = KWindowSystem::numberOfDesktops();

        for (int j = 0; j < numDesktops; ++j) {
            Plasma::Containment *c = containmentForScreen(screen, j);

            kDebug() << screen << j << (QObject*)c;
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

Plasma::Containment *DesktopCorona::findFreeContainment() const
{
    foreach (Plasma::Containment *cont, containments()) {
        if ((cont->containmentType() == Plasma::Containment::DesktopContainment ||
            cont->containmentType() == Plasma::Containment::CustomContainment) &&
            cont->screen() == -1 && !offscreenWidgets().contains(cont)) {
            return cont;
        }
    }

    return 0;
}

void DesktopCorona::addDesktopContainment(int screen, int desktop)
{
    kDebug() << screen << desktop;

    Plasma::Containment* c = findFreeContainment();
    if (!c) {
        // first try for "desktop", if it doesn't exist then we try for any 
        // desktopy containment
        c = addContainment("desktop");

        if (!c) {
            KPluginInfo::List desktopPlugins = Plasma::Containment::listContainmentsOfType("desktop");

            if (!desktopPlugins.isEmpty()) {
                c = addContainment(desktopPlugins.first().pluginName());
            }
        }

        if (!c) {
            kWarning() << "complete failure to load a desktop containment!";
            return;
        }

        c->setActivity(i18n("Desktop"));
    } else {
        kDebug() << "found an existing containment:" << c->screen() << c->desktop();
    }

    c->setScreen(screen, desktop);
    c->setFormFactor(Plasma::Planar);
    c->flushPendingConstraintsEvents();
    emit containmentAdded(c);
    requestConfigSync();
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

void DesktopCorona::processUpdateScripts()
{
    QStringList scripts = KGlobal::dirs()->findAllResources("data", "plasma-desktop/updates/*.js");
    if (scripts.isEmpty()) {
        //kDebug() << "no update scripts";
        return ;
    }

    KConfigGroup cg(KGlobal::config(), "Updates");
    QStringList performed = cg.readEntry("performed", QStringList());
    const QString localDir = KGlobal::dirs()->localkdedir();
    const QString localXdgDir = KGlobal::dirs()->localxdgdatadir();

    QMultiMap<QString, QString> scriptPaths;
    foreach (const QString &script, scripts) {
        if (performed.contains(script)) {
            continue;
        }

        if (script.startsWith(localDir) || script.startsWith(localXdgDir)) {
            kDebug() << "skipping user local script: " << script;
            continue;
        }

        QFileInfo f(script);
        QString filename = f.fileName();
        scriptPaths.insert(filename, script);
        performed.append(script);
    }

    evaluateScripts(scriptPaths);
    cg.writeEntry("performed", performed);
    KGlobal::config()->sync();
}

bool DesktopCorona::loadDefaultLayoutScripts()
{
    QStringList scripts = KGlobal::dirs()->findAllResources("data", "plasma-desktop/init/*.js");
    if (scripts.isEmpty()) {
        //kDebug() << "no javascript based layouts";
        return false;
    }

    const QString localDir = KGlobal::dirs()->localkdedir();
    const QString localXdgDir = KGlobal::dirs()->localxdgdatadir();

    QMap<QString, QString> scriptPaths;
    foreach (const QString &script, scripts) {
        if (script.startsWith(localDir) || script.startsWith(localXdgDir)) {
            kDebug() << "skipping user local script: " << script;
            continue;
        }

        QFileInfo f(script);
        QString filename = f.fileName();
        if (!scriptPaths.contains(filename)) {
            scriptPaths.insert(filename, script);
        }
    }

    evaluateScripts(scriptPaths);
    return !containments().isEmpty();
}

void DesktopCorona::evaluateScripts(QMap<QString, QString> scripts)
{
    ScriptEngine scriptEngine(this);
    connect(&scriptEngine, SIGNAL(printError(QString)), this, SLOT(printScriptError(QString)));
    connect(&scriptEngine, SIGNAL(print(QString)), this, SLOT(printScriptMessage(QString)));

    foreach (const QString &script, scripts) {
        QFile file(script);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text) ) {
            QString code = file.readAll();
            kDebug() << "evaluating startup script:" << script;
            scriptEngine.evaluateScript(code);
        }
    }
}

void DesktopCorona::printScriptError(const QString &error)
{
    kWarning() << "Startup script errror:" << error;
}

void DesktopCorona::printScriptMessage(const QString &error)
{
    kDebug() << "Startup script: " << error;
}

void DesktopCorona::loadDefaultLayout()
{
    if (loadDefaultLayoutScripts()) {
        return;
    }

    QString defaultConfig = KStandardDirs::locate("appdata", "plasma-default-layoutrc");
    if (!defaultConfig.isEmpty()) {
        kDebug() << "attempting to load the default layout from:" << defaultConfig;
        loadLayout(defaultConfig);

        if (!containments().isEmpty()) {
            return;
        }
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
        c->setScreen(i, -1);
        c->resize(screenGeometry(i).size());
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

    const int newHeight = 28;
    panel->resize(QSize((int)panel->size().width(), newHeight));
    panel->setMinimumSize(QSize((int)panel->minimumSize().width(), newHeight));
    panel->setMaximumSize(QSize((int)panel->maximumSize().width(), newHeight));

    panel->updateConstraints(Plasma::StartupCompletedConstraint);
    panel->flushPendingConstraintsEvents();

    // some default applets to get a usable UI
    Plasma::Applet *applet = loadDefaultApplet("launcher", panel);
    if (applet) {
        applet->setGlobalShortcut(KShortcut("Alt+F1"));
    }

    loadDefaultApplet("pager", panel);
    loadDefaultApplet("tasks", panel);
    Plasma::Applet *sysTray = loadDefaultApplet("systemtray", panel);

    if (sysTray) {
        QAction *addDefaultApplets = sysTray->action("add default applets");
        if (addDefaultApplets) {
            addDefaultApplets->trigger();
        }
    }

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
            applet->updateConstraints(Plasma::AllConstraints | Plasma::StartupCompletedConstraint);
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

