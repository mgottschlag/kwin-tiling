/*
 *   Copyright (C) 2006 Aaron Seigo <aseigo@kde.org>
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

#include <unistd.h>

#include <QTimer>

#include <kcrash.h>

#include "desktop.h"
#include "plasmaapp.h"
#include "plasmaapp.moc"

PlasmaApp* PlasmaApp::self()
{
    return qobject_cast<PlasmaApp*>(kapp);
}

PlasmaApp::PlasmaApp()
    : KUniqueApplication(),
      m_engineManager(0)
{
    notifyStartup(false);
    if (KCrash::crashHandler() == 0 )
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

/*
    dcopClient()->send("ksplash", "", "upAndRunning(QString)",
                       QString::fromLocal8Bit(KCmdLineArgs::appName()));
*/

    m_interface = this;
    m_engineManager = new DataEngineManager;

    m_desktop = new Desktop;
    m_desktop->show();

    notifyStartup(true);
}

PlasmaApp::~PlasmaApp()
{
}

void PlasmaApp::setCrashHandler()
{
//    KCrash::setEmergencySaveFunction(Kicker::crashHandler);
}

void PlasmaApp::crashHandler(int signal)
{
    Q_UNUSED(signal);

    fprintf(stderr, "Plasma crashed, attempting to automatically recover\n");

//    DCOPClient::emergencyClose();
    sleep(1);
    system("plasma --nocrashhandler &"); // try to restart
}

bool PlasmaApp::loadDataEngine(const QString& name)
{
    if (!m_engineManager)
        return false;

    return m_engineManager->loadDataEngine(name);
}

void PlasmaApp::unloadDataEngine(const QString& name)
{
    if (!m_engineManager)
        return;

    m_engineManager->unloadDataEngine(name);
}

void PlasmaApp::notifyStartup(bool completed)
{
/*
    const QString startupID("workspace desktop");
    DCOPClient cl;
    cl.attach();
    DCOPRef r("ksmserver", "ksmserver");
    r.setDCOPClient(&cl);

    if (completed)
    {
        r.send("resumeStartup", startupID);
    }
    else
    {
        r.send("suspendStartup", startupID);
    }
*/
}
