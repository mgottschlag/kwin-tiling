/*
*   Copyright 2007 by Aaron Seigo <aseigo@kde.org>
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

#include <QAction>

#include <KAuthorized>
#include <KDebug>
#include <KRun>

#include "plasma/appletbrowser.h"
#include "workspace/kworkspace.h"

#include "krunner_interface.h"
#include "ksmserver_interface.h"
#include "screensaver_interface.h"

using namespace Plasma;

DefaultDesktop::DefaultDesktop(QObject *parent, const QVariantList &args)
    : Containment(parent, args),
      m_engineExplorerAction(0),
      m_appletBrowserAction(0),
      m_runCommandAction(0),
      m_lockAction(0),
      m_logoutAction(0),
      m_appletBrowser(0)
{
}

DefaultDesktop::~DefaultDesktop()
{
}

void DefaultDesktop::launchExplorer()
{
    KRun::run("plasmaAppletExplorer", KUrl::List(), 0);
}

void DefaultDesktop::launchAppletBrowser()
{
    if (!m_appletBrowser) {
        //TODO: should we delete this after some point, so as to conserve memory
        //      and any possible processing tha tmight end up in AppletBrowser?
        m_appletBrowser = new Plasma::AppletBrowser(this);
    }

    m_appletBrowser->show();
}

void DefaultDesktop::runCommand()
{
    if (!KAuthorized::authorizeKAction("run_command")) {
        return;
    }

    QString interface("org.kde.krunner");
    org::kde::krunner::Interface krunner(interface, "/Interface",
                                         QDBusConnection::sessionBus());
    if (krunner.isValid()) {
        krunner.display();
    }
}

void DefaultDesktop::lockScreen()
{
    if (!KAuthorized::authorizeKAction("lock_screen")) {
        return;
    }

    QString interface("org.freedesktop.ScreenSaver");
    org::freedesktop::ScreenSaver screensaver(interface, "/ScreenSaver",
                                              QDBusConnection::sessionBus());
    if (screensaver.isValid()) {
        screensaver.Lock();
    }
}

QList<QAction*> DefaultDesktop::contextActions()
{
    //FIXME: several items here ... probably all junior jobs =)
    //  - pretty up the menu with separators
    //  - should we offer "Switch User" here?

    if (!m_appletBrowserAction) {
        m_engineExplorerAction = new QAction(i18n("Engine Explorer"), this);
        connect(m_engineExplorerAction, SIGNAL(triggered(bool)), this, SLOT(launchExplorer()));

        m_appletBrowserAction = new QAction(i18n("Add applet"), this);
        connect(m_appletBrowserAction, SIGNAL(triggered(bool)), this, SLOT(launchAppletBrowser()));

        m_runCommandAction = new QAction(i18n("Run Command..."), this);
        connect(m_runCommandAction, SIGNAL(triggered(bool)), this, SLOT(runCommand()));

        m_lockAction = new QAction(i18n("Lock Screen"), this);
        m_lockAction->setIcon(KIcon("system-lock-screen"));
        connect(m_lockAction, SIGNAL(triggered(bool)), this, SLOT(lockScreen()));

        m_logoutAction = new QAction(i18n("Logout"), this);
        m_logoutAction->setIcon(KIcon("system-log-out"));
        connect(m_logoutAction, SIGNAL(triggered(bool)), this, SLOT(logout()));
    }

    QList<QAction*> actions;

    actions.append(m_engineExplorerAction);
    actions.append(m_appletBrowserAction);

    if (KAuthorized::authorizeKAction("run_command")) {
        actions.append(m_runCommandAction);
    }

    if (KAuthorized::authorizeKAction("lock_screen")) {
        actions.append(m_lockAction);
    }

    if (KAuthorized::authorizeKAction("logout")) {
        actions.append(m_logoutAction);
    }

    return actions;
}

void DefaultDesktop::logout()
{
    if (!KAuthorized::authorizeKAction("logout")) {
        return;
    }

    QString interface("org.kde.ksmserver");
    org::kde::KSMServerInterface smserver(interface, "/KSMServer",
                                          QDBusConnection::sessionBus());
    if (smserver.isValid()) {
        smserver.logout(KWorkSpace::ShutdownConfirmDefault,
                        KWorkSpace::ShutdownTypeDefault,
                        KWorkSpace::ShutdownModeDefault);
    }
}

K_EXPORT_PLASMA_APPLET(desktop, DefaultDesktop)

#include "desktop.moc"
