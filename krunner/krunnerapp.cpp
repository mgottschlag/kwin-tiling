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

#include <QTimer>

#include <kaction.h>
#include <kglobalaccel.h>
#include <kauthorized.h>
#include <kglobalsettings.h>
#include <kapplication.h>
#include <klocale.h>
#include <KActionCollection>
#include <KMessageBox>
#include <kprocess.h>
#include "../lib/kworkspace.h"
#include <QObject>
#include <QtDBus/QtDBus>

#include "interfaceadaptor.h"
#include "interface.h"

KRunnerApp::KRunnerApp(Display *display,
                       Qt::HANDLE visual,
                       Qt::HANDLE colormap)
    : RestartingApplication( display, visual, colormap )
{
    kDebug() << "new krunner app" << endl;
}

KRunnerApp::KRunnerApp()
    : RestartingApplication()
{
    kDebug() << "new simple krunner app" << endl;
}

KRunnerApp::~KRunnerApp()
{
    delete m_interface;
}

void KRunnerApp::initialize()
{
    m_interface = new Interface;

    // Global keys
    m_actionCollection = new KActionCollection( m_interface );
    QAction* a = 0;

    if ( KAuthorized::authorizeKAction( "run_command" ) ) {
        a = m_actionCollection->addAction( I18N_NOOP("Run Command") );
        a->setText( i18n( I18N_NOOP( "Run Command" ) ) );
        qobject_cast<KAction*>( a )->setGlobalShortcut(KShortcut(Qt::ALT+Qt::Key_F2));
        connect( a, SIGNAL(triggered(bool)), m_interface, SLOT(display()) );
/*
        QDialog* test = new QDialog;
        connect( a, SIGNAL(triggered(bool)), test, SLOT(show()) );
*/
    }

    a = m_actionCollection->addAction( I18N_NOOP( "Show Taskmanager" ) );
    a->setText( i18n( I18N_NOOP( "Show Taskmanager" ) ) );
    qobject_cast<KAction*>( a )->setGlobalShortcut( KShortcut( Qt::CTRL+Qt::Key_Escape ) );
    connect( a, SIGNAL(triggered(bool)), SLOT(showTaskManager()) );

/*
 * TODO: doesn't this belong in the window manager?
    a = m_actionCollection->addAction( I18N_NOOP( "Show Window List") );
    a->setText( i18n( I18N_NOOP( "Show Window List") ) );
    qobject_cast<KAction*>( a )->setGlobalShortcut( KShortcut( Qt::ALT+Qt::Key_F5 ) );
    connect( a, SIGNAL(triggered(bool)), SLOT(slotShowWindowList()) );
*/
    a = m_actionCollection->addAction( I18N_NOOP("Switch User") );
    a->setText( i18n( I18N_NOOP("Switch User") ) );
    qobject_cast<KAction*>( a )->setGlobalShortcut( KShortcut( Qt::ALT+Qt::CTRL+Qt::Key_Insert ) );
    connect(a, SIGNAL(triggered(bool)), SLOT(switchUser()));

    if ( KAuthorized::authorizeKAction( "lock_screen" ) ) {
        a = m_actionCollection->addAction( I18N_NOOP( "Lock Session" ) );
        a->setText( i18n( I18N_NOOP( "Lock Session" ) ) );
        qobject_cast<KAction*>( a )->setGlobalShortcut( KShortcut( Qt::ALT+Qt::CTRL+Qt::Key_L ) );
        connect( a, SIGNAL(triggered(bool)), &m_saver, SLOT(Lock()) );
    }

    if ( KAuthorized::authorizeKAction( "logout" ) ) {
        a = m_actionCollection->addAction( I18N_NOOP("Log Out") );
        a->setText( i18n(I18N_NOOP("Log Out")) );
        qobject_cast<KAction*>( a )->setGlobalShortcut(KShortcut(Qt::ALT+Qt::CTRL+Qt::Key_Delete));
        connect(a, SIGNAL(triggered(bool)), SLOT(logout()));

        a = m_actionCollection->addAction( I18N_NOOP("Log Out Without Confirmation") );
        a->setText( i18n(I18N_NOOP("Log Out Without Confirmation")) );
        qobject_cast<KAction*>( a )->setGlobalShortcut(KShortcut(Qt::ALT+Qt::CTRL+Qt::SHIFT+Qt::Key_Delete));
        connect(a, SIGNAL(triggered(bool)), SLOT(logoutWithoutConfirmation()));

        a = m_actionCollection->addAction( I18N_NOOP("Halt without Confirmation") );
        a->setText( i18n(I18N_NOOP("Halt without Confirmation")) );
        qobject_cast<KAction*>( a )->setGlobalShortcut(KShortcut(Qt::ALT+Qt::CTRL+Qt::SHIFT+Qt::Key_PageDown));
        connect(a, SIGNAL(triggered(bool)), SLOT(haltWithoutConfirmation()));

        a = m_actionCollection->addAction( I18N_NOOP("Reboot without Confirmation") );
        a->setText( i18n(I18N_NOOP("Reboot without Confirmation")) );
        qobject_cast<KAction*>( a )->setGlobalShortcut(KShortcut(Qt::ALT+Qt::CTRL+Qt::SHIFT+Qt::Key_PageUp));
        connect(a, SIGNAL(triggered(bool)), SLOT(rebootWithoutConfirmation()));
    }

    m_actionCollection->readSettings();
} // end void KRunnerApp::initializeBindings


void KRunnerApp::switchUser()
{
    //TODO: use the SessionRunner
}

/*TODO: fixme - move to kwin
void KRunnerApp::showWindowList()
{
     //KRootWm::self()->slotWindowList();
}
*/

void KRunnerApp::showTaskManager()
{
    //kDebug(1204) << "Launching KSysGuard..." << endl;
    KProcess* p = new KProcess;
    Q_CHECK_PTR(p);

    *p << "ksysguard";
    *p << "--showprocesses";

    p->start(KProcess::DontCare);

    delete p;
}

void KRunnerApp::logout()
{
    logout( KWorkSpace::ShutdownConfirmDefault,
            KWorkSpace::ShutdownTypeDefault );
}

void KRunnerApp::logoutWithoutConfirmation()
{
    logout( KWorkSpace::ShutdownConfirmNo,
            KWorkSpace::ShutdownTypeNone );
}

void KRunnerApp::haltWithoutConfirmation()
{
    logout( KWorkSpace::ShutdownConfirmNo,
            KWorkSpace::ShutdownTypeHalt );
}

void KRunnerApp::rebootWithoutConfirmation()
{
    logout( KWorkSpace::ShutdownConfirmNo,
            KWorkSpace::ShutdownTypeReboot );
}

void KRunnerApp::logout( KWorkSpace::ShutdownConfirm confirm,
                       KWorkSpace::ShutdownType sdtype )
{
    if ( !KWorkSpace::requestShutDown( confirm, sdtype ) ) {
        // TODO: should we show these errors in Interface?
        KMessageBox::error( 0, i18n("Could not log out properly.\nThe session manager cannot "
                                    "be contacted. You can try to force a shutdown by pressing "
                                    "Ctrl+Alt+Backspace; note, however, that your current session "
                                    "will not be saved with a forced shutdown." ) );
    }
}

#include <KStartupInfo>
int KRunnerApp::newInstance()
{
    static bool firstTime = true;
    if ( firstTime ) {
        // App startup: do nothing
        firstTime = false;
    } else {
        KStartupInfo::setNewStartupId( m_interface, KStartupInfo::createNewStartupId() );
        m_interface->display();
        kDebug() << "startup id is " << startupId() << endl;
    }

    return 0;
}

#include "krunnerapp.moc"
