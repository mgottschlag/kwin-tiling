/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
                 2007       Troy Unrau <troy.unrau@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

//#include <krootwm.h>
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

#include "bindings.h"


void KRunnerBindings::setBindings() {

  // Global keys
  KActionCollection* actionCollection = m_actionCollection = new KActionCollection( this );
  // (void) new KRootWm( this );
  QAction* a = 0L;

//#include "kdesktopbindings.cpp"
  a = actionCollection->addAction( "Program:krunner" );
  a->setText( i18n("Runner") );
  
  if (KAuthorized::authorizeKAction("run_command"))
	{
   a = actionCollection->addAction( I18N_NOOP("Run Command") );                        
   a->setText( i18n(I18N_NOOP("Run Command")) );                                       
   qobject_cast<KAction*>( a )->setGlobalShortcut(KShortcut(Qt::ALT+Qt::Key_F2)); 
   connect(a, SIGNAL(triggered(bool)), SLOT(slotExecuteCommand())); // TODO: needs to be Interface.display() slot
	}

   a = actionCollection->addAction( I18N_NOOP("Show Taskmanager") );                        
   a->setText( i18n(I18N_NOOP("Show Taskmanager")) );                                       
   qobject_cast<KAction*>( a )->setGlobalShortcut(KShortcut(Qt::CTRL+Qt::Key_Escape)); 
   connect(a, SIGNAL(triggered(bool)), SLOT(slotShowTaskManager()));

   a = actionCollection->addAction( I18N_NOOP("Show Window List") );                        
   a->setText( i18n(I18N_NOOP("Show Window List")) );                                       
   qobject_cast<KAction*>( a )->setGlobalShortcut(KShortcut(Qt::ALT+Qt::Key_F5)); 
   connect(a, SIGNAL(triggered(bool)), SLOT(slotShowWindowList()));

   a = actionCollection->addAction( I18N_NOOP("Switch User") );                        
   a->setText( i18n(I18N_NOOP("Switch User")) );                                       
   qobject_cast<KAction*>( a )->setGlobalShortcut(KShortcut(Qt::ALT+Qt::CTRL+Qt::Key_Insert)); 
   connect(a, SIGNAL(triggered(bool)), SLOT(slotSwitchUser()));

/*  if (KAuthorized::authorizeKAction("lock_screen"))
	{
   a = actionCollection->addAction(I18N_NOOP("Lock Session"));                        
   a->setText( i18n(I18N_NOOP("Lock Session")) );                                       
   qobject_cast<KAction*>( a )->setGlobalShortcut(KShortcut(Qt::ALT+Qt::CTRL+Qt::Key_L)); 
   connect(a, SIGNAL(triggered(bool)), KRootWm::self(), slotLock())
	}
*/
  if (KAuthorized::authorizeKAction("logout"))
	{
   a = actionCollection->addAction( I18N_NOOP("Log Out") );                        
   a->setText( i18n(I18N_NOOP("Log Out")) );                                       
   qobject_cast<KAction*>( a )->setGlobalShortcut(KShortcut(Qt::ALT+Qt::CTRL+Qt::Key_Delete)); 
   connect(a, SIGNAL(triggered(bool)), SLOT(slotLogout()));

   a = actionCollection->addAction( I18N_NOOP("Log Out Without Confirmation") );                        
   a->setText( i18n(I18N_NOOP("Log Out Without Confirmation")) );                                       
   qobject_cast<KAction*>( a )->setGlobalShortcut(KShortcut(Qt::ALT+Qt::CTRL+Qt::SHIFT+Qt::Key_Delete)); 
   connect(a, SIGNAL(triggered(bool)), SLOT(slotLogoutNoCnf()));

   a = actionCollection->addAction( I18N_NOOP("Halt without Confirmation") );                        
   a->setText( i18n(I18N_NOOP("Halt without Confirmation")) );                                       
   qobject_cast<KAction*>( a )->setGlobalShortcut(KShortcut(Qt::ALT+Qt::CTRL+Qt::SHIFT+Qt::Key_PageDown)); 
   connect(a, SIGNAL(triggered(bool)), SLOT(slotHaltNoCnf()));

   a = actionCollection->addAction( I18N_NOOP("Reboot without Confirmation") );                        
   a->setText( i18n(I18N_NOOP("Reboot without Confirmation")) );                                       
   qobject_cast<KAction*>( a )->setGlobalShortcut(KShortcut(Qt::ALT+Qt::CTRL+Qt::SHIFT+Qt::Key_PageUp)); 
   connect(a, SIGNAL(triggered(bool)), SLOT(slotRebootNoCnf()));
	}
  m_actionCollection->readSettings();

} // end void KRunnerBindings::setBindings

void KRunnerBindings::slotSwitchUser()
{
     //TODO: fixme
     //KRootWm::self()->slotSwitchUser();
}

void KRunnerBindings::slotShowWindowList()
{
     //TODO: fixme
     //KRootWm::self()->slotWindowList();
}

void KRunnerBindings::slotShowTaskManager()
{
    //kDebug(1204) << "Launching KSysGuard..." << endl;
    KProcess* p = new KProcess;
    Q_CHECK_PTR(p);

    *p << "ksysguard";
    *p << "--showprocesses";

    p->start(KProcess::DontCare);

    delete p;
}

void KRunnerBindings::slotLogout()
{
    logout( KWorkSpace::ShutdownConfirmDefault,
            KWorkSpace::ShutdownTypeDefault );
}

void KRunnerBindings::slotLogoutNoCnf()
{
    logout( KWorkSpace::ShutdownConfirmNo,
            KWorkSpace::ShutdownTypeNone );
}

void KRunnerBindings::slotHaltNoCnf()
{
    logout( KWorkSpace::ShutdownConfirmNo,
            KWorkSpace::ShutdownTypeHalt );
}

void KRunnerBindings::slotRebootNoCnf()
{
    logout( KWorkSpace::ShutdownConfirmNo,
            KWorkSpace::ShutdownTypeReboot );
}

// For the dbus interface [maybe the dbus interface should have those extra args?]
void KRunnerBindings::logout()
{
    logout( KWorkSpace::ShutdownConfirmDefault,
            KWorkSpace::ShutdownTypeNone );
}

void KRunnerBindings::logout( KWorkSpace::ShutdownConfirm confirm,
                       KWorkSpace::ShutdownType sdtype )
{
    if( !KWorkSpace::requestShutDown( confirm, sdtype ) )
        // this i18n string is also in kicker/applets/run/runapplet
        KMessageBox::error( this, i18n("Could not log out properly.\nThe session manager cannot "
                                        "be contacted. You can try to force a shutdown by pressing "
                                        "Ctrl+Alt+Backspace; note, however, that your current session "
                                        "will not be saved with a forced shutdown." ) );
}

