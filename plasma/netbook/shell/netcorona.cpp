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

#include "netcorona.h"
#include "netdialogmanager.h"

#include <QAction>
#include <QApplication>
#include <QDesktopWidget>
#include <QDir>
#include <QGraphicsLayout>

#include <KCmdLineArgs>
#include <KDebug>
#include <KDialog>
#include <KGlobalSettings>
#include <KStandardDirs>
#include <KWindowSystem>

#include <kephal/screens.h>

#include <Plasma/Containment>
#include <Plasma/DataEngineManager>

#include "plasmaapp.h"
#include "netview.h"
#include <plasma/containmentactionspluginsconfig.h>
#include <netbookscriptengine.h>

NetCorona::NetCorona(QObject *parent)
    : Plasma::Corona(parent)
{
    init();
}

void NetCorona::init()
{
    QDesktopWidget *desktop = QApplication::desktop();
    QObject::connect(desktop, SIGNAL(resized(int)), this, SLOT(screenResized(int)));
    connect(PlasmaApp::self(), SIGNAL(controlBarChanged()), this, SIGNAL(availableScreenRegionChanged()));

    connect(this, SIGNAL(containmentAdded(Plasma::Containment*)), this, SLOT(containmentAdded(Plasma::Containment*)));

    Plasma::ContainmentActionsPluginsConfig desktopPlugins;
    desktopPlugins.addPlugin(Qt::NoModifier, Qt::MidButton, "paste");
    desktopPlugins.addPlugin(Qt::NoModifier, Qt::RightButton, "contextmenu");
    Plasma::ContainmentActionsPluginsConfig panelPlugins;
    panelPlugins.addPlugin(Qt::NoModifier, Qt::RightButton, "contextmenu");

    setContainmentActionsDefaults(Plasma::Containment::DesktopContainment, desktopPlugins);
    setContainmentActionsDefaults(Plasma::Containment::CustomContainment, desktopPlugins);
    setContainmentActionsDefaults(Plasma::Containment::PanelContainment, panelPlugins);
    setContainmentActionsDefaults(Plasma::Containment::CustomPanelContainment, panelPlugins);

    enableAction("lock widgets", false);
    setDialogManager(new NetDialogManager(this));
}

void NetCorona::loadDefaultLayout()
{
    evaluateScripts(WorkspaceScripting::NetbookScriptEngine::defaultLayoutScripts());
    if (!containments().isEmpty()) {
        return;
    }

    QString defaultConfig = KStandardDirs::locate("appdata", "plasma-default-layoutrc");
    if (!defaultConfig.isEmpty()) {
        kDebug() << "attempting to load the default layout from:" << defaultConfig;
        importLayout(KConfig(defaultConfig));

        return;
    }
}

Plasma::Applet *NetCorona::loadDefaultApplet(const QString &pluginName, Plasma::Containment *c)
{
    QVariantList args;
    Plasma::Applet *applet = Plasma::Applet::load(pluginName, 0, args);

    if (applet) {
        c->addApplet(applet);
    }

    return applet;
}

void NetCorona::containmentAdded(Plasma::Containment *cont)
{
    if (cont->pluginName() == "sal") {
        QAction *a = cont->action("remove");
        cont->removeAction(a);
        delete a;
    }
}

Plasma::Containment *NetCorona::findFreeContainment() const
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

void NetCorona::screenResized(int screen)
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

int NetCorona::numScreens() const
{
    return Kephal::ScreenUtils::numScreens();
}

QRect NetCorona::screenGeometry(int id) const
{
    return Kephal::ScreenUtils::screenGeometry(id);
}

QRegion NetCorona::availableScreenRegion(int id) const
{
    QRegion r(screenGeometry(id));
    NetView *view = PlasmaApp::self()->controlBar();
    if (view) {
        r = r.subtracted(view->geometry());
    }
    QWidget *explorer = PlasmaApp::self()->widgetExplorer();
    if (explorer) {
        r = r.subtracted(explorer->geometry());
    }

    return r;
}

void NetCorona::processUpdateScripts()
{
    evaluateScripts(WorkspaceScripting::NetbookScriptEngine::pendingUpdateScripts());
}

void NetCorona::evaluateScripts(const QStringList &scripts)
{
    foreach (const QString &script, scripts) {
        WorkspaceScripting::NetbookScriptEngine scriptEngine(this);
        connect(&scriptEngine, SIGNAL(printError(QString)), this, SLOT(printScriptError(QString)));
        connect(&scriptEngine, SIGNAL(print(QString)), this, SLOT(printScriptMessage(QString)));
        connect(&scriptEngine, SIGNAL(createPendingPanelViews()), PlasmaApp::self(), SLOT(createWaitingPanels()));

        QFile file(script);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text) ) {
            QString code = file.readAll();
            kDebug() << "evaluating startup script:" << script;
            scriptEngine.evaluateScript(code);
        }
    }
}

void NetCorona::printScriptError(const QString &error)
{
    kWarning() << "Startup script errror:" << error;
}

void NetCorona::printScriptMessage(const QString &error)
{
    kDebug() << "Startup script: " << error;
}


#include "netcorona.moc"

