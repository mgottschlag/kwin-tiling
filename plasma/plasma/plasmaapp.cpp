/*
 *   Copyright 2006 Aaron Seigo <aseigo@kde.org>
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

// plasma.loadEngine("hardware")
// LineGraph graph
// plasma.connect(graph, "hardware", "cpu");

#include "plasmaapp.h"

#include <unistd.h>

#include <QPixmapCache>
#include <QTimer>
#include <QtDBus/QtDBus>

#include <KCrash>
#include <KDebug>
#include <KCmdLineArgs>

#include <ksmserver_interface.h>

#include <plasma/corona.h>
#include <plasma/widgets/layout.h>

#include "appadaptor.h"
#include "rootwidget.h"
#include "desktopview.h"
#include "panel.h"

// testing
#include <KIcon>
#include <plasma/widgets/icon.h>

PlasmaApp* PlasmaApp::self()
{
    return qobject_cast<PlasmaApp*>(kapp);
}

PlasmaApp::PlasmaApp()
    : m_root(0),
      m_corona(0)
{
    new AppAdaptor(this); 
    QDBusConnection::sessionBus().registerObject("/App", this);
    notifyStartup(false);


    // this same pattern is in KRunner (see workspace/krunner/restartingapplication.h)
    // would be interesting to see if this could be shared.
    if (!KCrash::crashHandler())
    {
        // this means we've most likely crashed once. so let's see if we
        // stay up for more than 2 minutes time, and if so reset the
        // crash handler since the crash isn't a frequent offender
        QTimer::singleShot(120000, this, SLOT(setCrashHandler()));
    }
    else
    {
        // See if a crash handler was installed. It was if the -nocrashhandler
        // argument was given, but the app eats the kde options so we can't
        // check that directly. If it wasn't, don't install our handler either.
        setCrashHandler();
    }

    // enlarge application pixmap cache
    // TODO: make this dependand on system
    // memory and screen resolution. 8MB is ok for caching the background up
    // to 1600x1200 resolution
    if (QPixmapCache::cacheLimit() < 8192) {
        QPixmapCache::setCacheLimit(8192);
    }

    m_root = new RootWidget();
    //TODO: the line below is just inane. get rid of it before release. seriously.
    m_root->setAsDesktop(KCmdLineArgs::parsedArgs()->isSet("desktop"));

    m_root->show();
    connect(this, SIGNAL(aboutToQuit()), corona(), SLOT(saveApplets()));

    notifyStartup(true);

    createDefaultPanels();
}

PlasmaApp::~PlasmaApp()
{
    delete m_root;
    qDeleteAll(m_panels);
}

void PlasmaApp::initializeWallpaper()
{
    if (!m_root) {
        return;
    }

    //FIXME this has moved to Containment, so .....
    //m_root->desktop()->initializeWallpaper();
}

void PlasmaApp::setCrashHandler()
{
    KCrash::setEmergencySaveFunction(PlasmaApp::crashHandler);
}

void PlasmaApp::crashHandler(int signal)
{
    Q_UNUSED(signal);

    fprintf(stderr, "Plasma crashed, attempting to automatically recover\n");

    sleep(1);
    system("plasma --nocrashhandler &"); // try to restart
}

Plasma::Corona* PlasmaApp::corona()
{
    if (!m_corona) {
        m_corona = new Plasma::Corona(this);
        m_corona->setItemIndexMethod(QGraphicsScene::NoIndex);
        //TODO: Figure out a way to use rubberband and ScrollHandDrag
        m_corona->loadApplets();
    }

    return m_corona;
}

void PlasmaApp::notifyStartup(bool completed)
{
    org::kde::KSMServerInterface ksmserver("org.kde.ksmserver", "/KSMServer", QDBusConnection::sessionBus());

    const QString startupID("workspace desktop");
    if (completed) {
        ksmserver.resumeStartup(startupID);
    } else {
        ksmserver.suspendStartup(startupID);
    }
}

// for testing purposes
// this layout should be saved and loaded to a config source
// soon
void PlasmaApp::createDefaultPanels()
{
    return;
    Plasma::Panel *defaultPanel = new Plasma::Panel;
    Plasma::Corona *panelScene = new Plasma::Corona;
    defaultPanel->setCorona(panelScene);

    // place-holder for the launcher menu
    Plasma::Icon *launcherPlaceholder = new Plasma::Icon(KIcon("kmenu"),QString());
    panelScene->addItem(launcherPlaceholder);
    defaultPanel->layout()->addItem(launcherPlaceholder);

    // some default applets to get a usable UI
    QList<Plasma::Applet*> applets;
    Plasma::Applet *tasksApplet = panelScene->addApplet("tasks");
    Plasma::Applet *systemTrayApplet = panelScene->addApplet("systemtray");
    Plasma::Applet *clockApplet = panelScene->addApplet("digital-clock");

    applets << tasksApplet << systemTrayApplet << clockApplet;

    foreach (Plasma::Applet* applet , applets) {
        applet->setDrawStandardBackground(false);
        defaultPanel->layout()->addItem(applet);
    }

    defaultPanel->setLocation(Plasma::BottomEdge);
    defaultPanel->show();
    m_panels << defaultPanel;
}

#include "plasmaapp.moc"
