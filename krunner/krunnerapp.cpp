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

#include "krunnerapp.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <QClipboard>
#include <QObject>
#include <QProcess>
#include <QTimer>
#include <QtDBus/QtDBus>
#include <QLineEdit>

#include <KAction>
#include <KActionCollection>
#include <KCrash>
#include <KDialog>
#include <KAuthorized>
#include <KGlobalAccel>
#include <KGlobalSettings>
#include <KLocale>
#include <KMessageBox>
#include <KWindowSystem>

#include <Plasma/RunnerManager>
#include <Plasma/AbstractRunner>

#include "kworkspace/kdisplaymanager.h"

#include "appadaptor.h"
#include "ksystemactivitydialog.h"
#include "interfaces/default/interface.h"
#include "interfaces/quicksand/qs_dialog.h"
#ifdef Q_WS_X11
#include "startupid.h"
#endif
#include "klaunchsettings.h"
#include "krunnersettings.h"

#ifdef Q_WS_X11
#include <X11/extensions/Xrender.h>
#endif

KRunnerApp* KRunnerApp::self()
{
    if (!kapp) {
        return new KRunnerApp();
    }

    return qobject_cast<KRunnerApp*>(kapp);
}

KRunnerApp::KRunnerApp()
    : KUniqueApplication(),
      m_interface(0),
      m_tasks(0),
      m_startupId(NULL),
      m_firstTime(true)
{
    initialize();
    connect(this, SIGNAL(aboutToQuit()), this, SLOT(cleanUp()));
}

KRunnerApp::~KRunnerApp()
{
}

void KRunnerApp::cleanUp()
{
    disconnect(KRunnerSettings::self(), SIGNAL(configChanged()), this, SLOT(reloadConfig()));
    kDebug() << "deleting interface";
    delete m_interface;
    m_interface = 0;
    delete m_runnerManager;
    m_runnerManager = 0;
#ifndef Q_WS_WIN
    delete m_tasks;
    m_tasks = 0;
#endif
    KGlobal::config()->sync();
}

KActionCollection* KRunnerApp::actionCollection()
{
    return m_actionCollection;
}

void KRunnerApp::initialize()
{
    setWindowIcon(KIcon(QLatin1String("system-run")));

    setQuitOnLastWindowClosed(false);
    KCrash::setFlags(KCrash::AutoRestart);
    initializeStartupNotification();

    connect(KRunnerSettings::self(), SIGNAL(configChanged()), this, SLOT(reloadConfig()));

    m_runnerManager = new Plasma::RunnerManager;

    new AppAdaptor(this);
    QDBusConnection::sessionBus().registerObject(QLatin1String("/App"), this);

    // Global keys
    m_actionCollection = new KActionCollection(this);
    KAction* a = 0;

    if (KAuthorized::authorize(QLatin1String("run_command"))) {
        a = m_actionCollection->addAction(QLatin1String("Run Command"));
        a->setText(i18n("Run Command"));
        a->setGlobalShortcut(KShortcut(Qt::ALT+Qt::Key_F2));
        connect(a, SIGNAL(triggered(bool)), SLOT(displayOrHide()));

        a = m_actionCollection->addAction(QLatin1String("Run Command on clipboard contents"));
        a->setText(i18n("Run Command on clipboard contents"));
        a->setGlobalShortcut(KShortcut(Qt::ALT+Qt::SHIFT+Qt::Key_F2));
        connect(a, SIGNAL(triggered(bool)), SLOT(displayWithClipboardContents()));
    }

    a = m_actionCollection->addAction(QLatin1String("Show System Activity"));
    a->setText(i18n("Show System Activity"));
    a->setGlobalShortcut(KShortcut(Qt::CTRL+Qt::Key_Escape));
    connect(a, SIGNAL(triggered(bool)), SLOT(showTaskManager()));

    if (KAuthorized::authorize(QLatin1String("switch_user"))) {
        a = m_actionCollection->addAction(QLatin1String("Switch User"));
        a->setText(i18n("Switch User"));
        a->setGlobalShortcut(KShortcut(Qt::ALT+Qt::CTRL+Qt::Key_Insert));
        connect(a, SIGNAL(triggered(bool)), SLOT(switchUser()));
    }

    //FIXME: lock/logout should be in the session management runner which also provides similar
    // functions
#ifdef Q_WS_X11
    if (KAuthorized::authorize(QLatin1String("lock_screen"))) {
        a = m_actionCollection->addAction(QLatin1String("Lock Session"));
        a->setText(i18n("Lock Session"));
        a->setGlobalShortcut(KShortcut(Qt::ALT+Qt::CTRL+Qt::Key_L));
        connect(a, SIGNAL(triggered(bool)), &m_saver, SLOT(Lock()));
    }
#endif

    //Setup the interface after we have set up the actions
    //TODO: if !KAuthorized::authorize("run_comand") (and !"switch_user" i suppose?)
    //      then we probably don't need the interface at all. would be another place
    //      for some small improvements in footprint in that case
    switch (KRunnerSettings::interface()) {
        default:
        case KRunnerSettings::EnumInterface::CommandOriented:
            m_interface = new Interface(m_runnerManager);
            break;
        case KRunnerSettings::EnumInterface::TaskOriented:
            m_interface = new QsDialog(m_runnerManager);
            break;
    }

#ifdef Q_WS_X11
    //FIXME: if argb visuals enabled Qt will always set WM_CLASS as "qt-subapplication" no matter what
    //the application name is we set the proper XClassHint here, hopefully won't be necessary anymore when
    //qapplication will manage apps with argvisuals in a better way
    XClassHint classHint;
    classHint.res_name = const_cast<char*>("krunner");
    classHint.res_class = const_cast<char*>("krunner");
    XSetClassHint(QX11Info::display(), m_interface->winId(), &classHint);
#endif


    m_actionCollection->readSettings();
    if (KAuthorized::authorize(QLatin1String("run_command"))) {
        //m_runnerManager->setAllowedRunners(QStringList() << "shell");
        m_runnerManager->reloadConfiguration(); // pre-load the runners

        // Single runner mode actions shortcuts

        foreach (const QString &runnerId, m_runnerManager->singleModeAdvertisedRunnerIds()) {
            a = m_actionCollection->addAction(runnerId);
            a->setText(i18nc("Run krunner restricting the search only to runner %1", "Run Command (runner \"%1\" only)",
                       m_runnerManager->runnerName(runnerId)));
            a->setGlobalShortcut(KShortcut());
            connect(a, SIGNAL(triggered(bool)), SLOT(singleRunnerModeActionTriggered()));
        }
    }
}

void KRunnerApp::singleRunnerModeActionTriggered()
{
    KAction * action = qobject_cast<KAction*>(sender());
    if (action) {
        displaySingleRunner(action->objectName());
    }
}

void KRunnerApp::querySingleRunner(const QString& runnerId, const QString &term)
{
    if (!KAuthorized::authorize(QLatin1String("run_command"))) {
        return;
    }

    m_runnerManager->setSingleModeRunnerId(runnerId);
    m_runnerManager->setSingleMode(!runnerId.isEmpty());

    if (m_runnerManager->singleMode()) {
        m_interface->display(term);
    }
}

QStringList KRunnerApp::singleModeAdvertisedRunnerIds() const
{
    return m_runnerManager->singleModeAdvertisedRunnerIds();
}

void KRunnerApp::initializeStartupNotification()
{
    // Startup notification
    KLaunchSettings::self()->readConfig();
#ifdef Q_WS_X11
    if (!KLaunchSettings::busyCursor()) {
        delete m_startupId;
        m_startupId = NULL;
    } else {
        if (m_startupId == NULL ) {
            m_startupId = new StartupId;
        }

        m_startupId->configure();
    }
#endif
}

void KRunnerApp::showTaskManager()
{
    showTaskManagerWithFilter(QString());
}

void KRunnerApp::showTaskManagerWithFilter(const QString &filterText)
{
#ifndef Q_WS_WIN
    //kDebug(1204) << "Launching KSysGuard...";
    if (!m_tasks) {
        m_tasks = new KSystemActivityDialog;
        connect(m_tasks, SIGNAL(finished()),
                this, SLOT(taskDialogFinished()));
    } else if ((filterText.isEmpty() || m_tasks->filterText() == filterText) &&
               KWindowSystem::activeWindow() == m_tasks->winId()) {
        m_tasks->hide();
        return;
    }

    m_tasks->run();
    m_tasks->setFilterText(filterText);
#endif
}

void KRunnerApp::display()
{
    if (!KAuthorized::authorize(QLatin1String("run_command"))) {
        return;
    }

    m_runnerManager->setSingleMode(false);
    m_interface->display();
}

void KRunnerApp::displaySingleRunner(const QString &runnerId)
{
    if (!KAuthorized::authorize(QLatin1String("run_command"))) {
        return;
    }

    m_runnerManager->setSingleModeRunnerId(runnerId);
    m_runnerManager->setSingleMode(!runnerId.isEmpty());
    m_interface->display();
}

void KRunnerApp::displayOrHide()
{
    if (!KAuthorized::authorize(QLatin1String("run_command"))) {
        m_interface->hide();
        return;
    }

    if (!m_interface->isVisible()) {
        m_runnerManager->setSingleMode(false);
    }

    if (m_interface->freeFloating()) {
        if (m_interface->isVisible()) {
            m_interface->hide();
        } else {
            m_interface->display();
        }
    } else if (m_interface->isActiveWindow()) {
        m_interface->hide();
    } else {
        m_interface->display();
    }
}

void KRunnerApp::query(const QString &term)
{
    if (!KAuthorized::authorize(QLatin1String("run_command"))) {
        return;
    }

    m_interface->display(term);
}

void KRunnerApp::displayWithClipboardContents()
{
    if (!KAuthorized::authorize(QLatin1String("run_command"))) {
        return;
    }

    QString clipboardData = QApplication::clipboard()->text(QClipboard::Selection);
    m_interface->display(clipboardData);
}

void KRunnerApp::switchUser()
{
    const KService::Ptr service = KService::serviceByStorageId(QLatin1String("plasma-runner-sessions.desktop"));
    KPluginInfo info(service);

    if (info.isValid()) {
        SessList sessions;
        KDisplayManager dm;
        dm.localSessions(sessions);

        if (sessions.isEmpty()) {
            // no sessions to switch between, let's just start up another session directly
            Plasma::AbstractRunner *sessionRunner = m_runnerManager->runner(info.pluginName());
            if (sessionRunner) {
                Plasma::QueryMatch switcher(sessionRunner);
                sessionRunner->run(*m_runnerManager->searchContext(), switcher);
            }
        } else {
            m_runnerManager->setSingleModeRunnerId(info.pluginName());
            m_runnerManager->setSingleMode(true);
            m_interface->display();
            //TODO: ugh, magic strings. See sessions/sessionrunner.cpp
            m_runnerManager->launchQuery(QLatin1String("SESSIONS"), info.pluginName());
        }
    }
}

void KRunnerApp::clearHistory()
{
    m_interface->clearHistory();
}

void KRunnerApp::taskDialogFinished()
{
#ifndef Q_WS_WIN
    m_tasks->deleteLater();
    m_tasks = 0;
#endif
}

int KRunnerApp::newInstance()
{
    if (m_firstTime) {
        m_firstTime = false;
    } else {
        display();
    }

    return KUniqueApplication::newInstance();
    //return 0;
}

void KRunnerApp::reloadConfig()
{
    //Prevent Interface destructor from triggering this method
    disconnect(KRunnerSettings::self(), SIGNAL(configChanged()), this, SLOT(reloadConfig()));

    const int interface = KRunnerSettings::interface();
    if (!qobject_cast<QsDialog*>(m_interface) &&
        interface == KRunnerSettings::EnumInterface::TaskOriented) {
        m_interface->deleteLater();
        m_interface = new QsDialog(m_runnerManager);
    } else if (!qobject_cast<Interface*>(m_interface) &&
               interface == KRunnerSettings::EnumInterface::CommandOriented) {
        m_interface->deleteLater();
        m_interface = new Interface(m_runnerManager);
    }

    m_interface->setFreeFloating(KRunnerSettings::freeFloating());
    connect(KRunnerSettings::self(), SIGNAL(configChanged()), this, SLOT(reloadConfig()));
    display();
}

#include "krunnerapp.moc"
