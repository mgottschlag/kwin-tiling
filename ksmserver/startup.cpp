/*****************************************************************
ksmserver - the KDE session management server

Copyright (C) 2000 Matthias Ettrich <ettrich@kde.org>
Copyright (C) 2005 Lubos Lunak <l.lunak@kde.org>

relatively small extensions by Oswald Buddenhagen <ob6@inf.tu-dresden.de>

some code taken from the dcopserver (part of the KDE libraries), which is
Copyright (c) 1999 Matthias Ettrich <ettrich@kde.org>
Copyright (c) 1999 Preston Brown <pbrown@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <pwd.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#include <sys/socket.h>
#include <sys/un.h>

#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <assert.h>

#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif

#include <QFile>
#include <QTextStream>
#include <QDataStream>
#include <QPushButton>
#include <QMessageBox>
#include <QTimer>
#include <QDesktopWidget>
#include <QtDBus/QtDBus>

#include <klocale.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <unistd.h>
#include <kapplication.h>
#include <kstaticdeleter.h>
#include <ktempfile.h>
#include <kprocess.h>
#include <kwinmodule.h>
#include <knotifyclient.h>

#include "global.h"
#include "server.h"
#include "client.h"
#include <kdebug.h>

#include <QX11Info>
#include <QApplication>

/*!  Restores the previous session. Ensures the window manager is
  running (if specified).
 */
void KSMServer::restoreSession( QString sessionName )
{
    if( state != Idle )
        return;
    state = LaunchingWM;

    kDebug( 1218 ) << "KSMServer::restoreSession " << sessionName << endl;
    upAndRunning( "restore session");
    KConfig* config = KGlobal::config();

    sessionGroup = "Session: " + sessionName;

    config->setGroup( sessionGroup );
    int count =  config->readEntry( "count", 0 );
    appsToStart = count;

    QList<QStringList> wmCommands;
    if ( !wm.isEmpty() ) {
	for ( int i = 1; i <= count; i++ ) {
	    QString n = QString::number(i);
	    if ( wm == config->readEntry( QString("program")+n, QString() ) ) {
		wmCommands << config->readEntry( QString("restartCommand")+n, QStringList() );
	    }
	}
    }
    if ( wmCommands.isEmpty() )
        wmCommands << ( QStringList() << wm );

    publishProgress( appsToStart, true );
    connect( klauncherSignals, SIGNAL( autoStart0Done()), SLOT( autoStart0Done()));
    connect( klauncherSignals, SIGNAL( autoStart1Done()), SLOT( autoStart1Done()));
    connect( klauncherSignals, SIGNAL( autoStart2Done()), SLOT( autoStart2Done()));
    upAndRunning( "ksmserver" );

    if ( !wmCommands.isEmpty() ) {
        // when we have a window manager, we start it first and give
        // it some time before launching other processes. Results in a
        // visually more appealing startup.
        for (int i = 0; i < wmCommands.count(); i++)
            startApplication( wmCommands[i] );
        QTimer::singleShot( 4000, this, SLOT( autoStart0() ) );
    } else {
        autoStart0();
    }
}

/*!
  Starts the default session.

  Currently, that's the window manager only (if specified).
 */
void KSMServer::startDefaultSession()
{
    if( state != Idle )
        return;

    state = LaunchingWM;
    sessionGroup = "";
    publishProgress( 0, true );
    upAndRunning( "ksmserver" );
    connect( klauncherSignals, SIGNAL( autoStart0Done()), SLOT( autoStart0Done()));
    connect( klauncherSignals, SIGNAL( autoStart1Done()), SLOT( autoStart1Done()));
    connect( klauncherSignals, SIGNAL( autoStart2Done()), SLOT( autoStart2Done()));
    startApplication( QStringList() << wm );
    QTimer::singleShot( 4000, this, SLOT( autoStart0() ) );
}

void KSMServer::clientSetProgram( KSMClient* client )
{
    if ( !wm.isEmpty() && client->program() == wm )
        autoStart0();
}

void KSMServer::autoStart0()
{
    if( state != LaunchingWM )
        return;
    if( !checkStartupSuspend())
        return;
    state = AutoStart0;
    QDBusInterface klauncher( "org.kde.klauncher", "/KLauncher", "org.kde.KLauncher" );
    klauncher.call( "autoStart", (int) 0 );
}

void KSMServer::autoStart0Done()
{
    if( state != AutoStart0 )
        return;
    disconnect( klauncherSignals, SIGNAL( autoStart0Done()), this, SLOT( autoStart0Done()));
    if( !checkStartupSuspend())
        return;
    kDebug( 1218 ) << "Autostart 0 done" << endl;
    upAndRunning( "kdesktop" );
    upAndRunning( "kicker" );
    kcminitSignals = new QDBusInterface("org.kde.kcminit", "/kcminit", "org.kde.KCMInit", QDBusConnection::sessionBus(), this );
    if( !kcminitSignals->isValid())
        kWarning() << "kcminit not running?" << endl;
    connect( kcminitSignals, SIGNAL( phase1Done()), SLOT( kcmPhase1Done()));
    state = KcmInitPhase1;
    QTimer::singleShot( 10000, this, SLOT( kcmPhase1Timeout())); // protection
    QDBusInterface kcminit( "org.kde.kcminit", "/kcminit", "org.kde.KCMInit" );
    kcminit.call( "runPhase1" );
}

void KSMServer::kcmPhase1Done()
{
    if( state != KcmInitPhase1 )
        return;
    kDebug( 1218 ) << "Kcminit phase 1 done" << endl;
    disconnect( kcminitSignals, SIGNAL( phase1Done()), this, SLOT( kcmPhase1Done()));
    autoStart1();
}

void KSMServer::kcmPhase1Timeout()
{
    if( state != KcmInitPhase1 )
        return;
    kDebug( 1218 ) << "Kcminit phase 1 timeout" << endl;
    kcmPhase1Done();
}

void KSMServer::autoStart1()
{
    if( state != KcmInitPhase1 )
        return;
    state = AutoStart1;
    QDBusInterface klauncher( "org.kde.klauncher", "/KLauncher", "org.kde.KLauncher" );
    klauncher.call( "autoStart", (int) 1 );
}

void KSMServer::autoStart1Done()
{
    if( state != AutoStart1 )
        return;
    disconnect( klauncherSignals, SIGNAL( autoStart1Done()), this, SLOT( autoStart1Done()));
    if( !checkStartupSuspend())
        return;
    kDebug( 1218 ) << "Autostart 1 done" << endl;
    lastAppStarted = 0;
    lastIdStarted.clear();
    state = Restoring;
    if( defaultSession()) {
        autoStart2();
        return;
    }
    tryRestoreNext();
}

void KSMServer::clientRegistered( const char* previousId )
{
    if ( previousId && lastIdStarted == previousId )
        tryRestoreNext();
}

void KSMServer::tryRestoreNext()
{
    if( state != Restoring )
        return;
    restoreTimer.stop();
    startupSuspendTimeoutTimer.stop();
    KConfig* config = KGlobal::config();
    config->setGroup( sessionGroup );

    while ( lastAppStarted < appsToStart ) {
        publishProgress ( appsToStart - lastAppStarted );
        lastAppStarted++;
        QString n = QString::number(lastAppStarted);
        QStringList restartCommand = config->readEntry( QString("restartCommand")+n, QStringList() );
        if ( restartCommand.isEmpty() ||
             (config->readEntry( QString("restartStyleHint")+n, 0 ) == SmRestartNever)) {
            continue;
        }
        if ( wm == config->readEntry( QString("program")+n, QString() ) )
            continue;
        startApplication( restartCommand,
                          config->readEntry( QString("clientMachine")+n, QString() ),
                          config->readEntry( QString("userId")+n, QString() ));
        lastIdStarted = config->readEntry( QString("clientId")+n, QString() );
        if ( !lastIdStarted.isEmpty() ) {
            restoreTimer.setSingleShot( true );
            restoreTimer.start( 2000 );
            return; // we get called again from the clientRegistered handler
        }
    }

    appsToStart = 0;
    lastIdStarted.clear();
    publishProgress( 0 );

    autoStart2();
}

void KSMServer::autoStart2()
{
    if( state != Restoring )
        return;
    if( !checkStartupSuspend())
        return;
    state = FinishingStartup;
    waitAutoStart2 = true;
    waitKcmInit2 = true;
    QDBusInterface klauncher( "org.kde.klauncher", "/KLauncher", "org.kde.KLauncher" );
    klauncher.call( "autoStart", (int) 2 );
    QDBusInterface kded( "org.kde.kded", "/kded", "org.kde.kded" );
    kded.call( "loadSecondPhase" );
    QDBusInterface kdesktop( "org.kde.kdesktop", "/kdesktop", "org.kde.KDesktopIface" );
    kdesktop.call( "runAutoStart" );
    connect( kcminitSignals, SIGNAL( phase2Done()), SLOT( kcmPhase2Done()));
    QTimer::singleShot( 10000, this, SLOT( kcmPhase2Timeout())); // protection
    QDBusInterface kcminit( "org.kde.kcminit", "/kcminit", "org.kde.KCMInit" );
    kcminit.call( "runPhase2" );
    if( !defaultSession())
        restoreLegacySession( KGlobal::config());
    KNotifyClient::event( 0, "startkde" ); // this is the time KDE is up, more or less
}

void KSMServer::autoStart2Done()
{
    if( state != FinishingStartup )
        return;
    disconnect( klauncherSignals, SIGNAL( autoStart2Done()), this, SLOT( autoStart2Done()));
    kDebug( 1218 ) << "Autostart 2 done" << endl;
    waitAutoStart2 = false;
    finishStartup();
}

void KSMServer::kcmPhase2Done()
{
    if( state != FinishingStartup )
        return;
    kDebug( 1218 ) << "Kcminit phase 2 done" << endl;
    disconnect( kcminitSignals, SIGNAL( phase2Done()), this, SLOT( kcmPhase2Done()));
    delete kcminitSignals;
    kcminitSignals = NULL;
    waitKcmInit2 = false;
    finishStartup();
}

void KSMServer::kcmPhase2Timeout()
{
    if( !waitKcmInit2 )
        return;
    kDebug( 1218 ) << "Kcminit phase 2 timeout" << endl;
    kcmPhase2Done();
}

void KSMServer::finishStartup()
{
    if( state != FinishingStartup )
        return;
    if( waitAutoStart2 || waitKcmInit2 )
        return;

    upAndRunning( "session ready" );
    // TODO
    QDBusInterface knotify( "org.kde.knotify", "/knotify", "org.kde.KNotify" );
    knotify.call( "sessionReady" ); // knotify startup optimization
    state = Idle;
    setupXIOErrorHandler(); // From now on handle X errors as normal shutdown.
}

bool KSMServer::checkStartupSuspend()
{
    if( startupSuspendCount.isEmpty())
        return true;
    // wait for the phase to finish
    if( !startupSuspendTimeoutTimer.isActive())
    {
        startupSuspendTimeoutTimer.setSingleShot( true );
        startupSuspendTimeoutTimer.start( 10000 );
    }
    return false;
}

void KSMServer::suspendStartup( QString app )
{
#if KDE_IS_VERSION( 3, 90, 0 )
#ifdef __GNUC__
#warning Re-enable suspend/resume startup and check it works properly.
#endif
#endif
    return;
    if( !startupSuspendCount.contains( app ))
        startupSuspendCount[ app ] = 0;
    ++startupSuspendCount[ app ];
}

void KSMServer::resumeStartup( QString app )
{
    return;
    if( !startupSuspendCount.contains( app ))
        return;
    if( --startupSuspendCount[ app ] == 0 ) {
        startupSuspendCount.remove( app );
        if( startupSuspendCount.isEmpty() && startupSuspendTimeoutTimer.isActive()) {
            startupSuspendTimeoutTimer.stop();
            resumeStartupInternal();
        }
    }
}

void KSMServer::startupSuspendTimeout()
{
    kDebug( 1218 ) << "Startup suspend timeout:" << state << endl;
    resumeStartupInternal();
}

void KSMServer::resumeStartupInternal()
{
    startupSuspendCount.clear();
    switch( state ) {
        case LaunchingWM:
            autoStart0();
          break;
        case AutoStart0:
            autoStart0Done();
          break;
        case AutoStart1:
            autoStart1Done();
          break;
        case Restoring:
            autoStart2();
          break;
        default:
            kWarning( 1218 ) << "Unknown resume startup state" << endl;
          break;
    }
}

void KSMServer::publishProgress( int progress, bool max  )
{
    QDBusInterface ksplash( "org.kde.ksplash", "/KSplash", "org.kde.KSplash" );
    ksplash.call( max ? "setMaxProgress" : "setProgress", progress );
}


void KSMServer::upAndRunning( const QString& msg )
{
    QDBusInterface ksplash( "org.kde.ksplash", "/KSplash", "org.kde.KSplash" );
    ksplash.call( "upAndRunning", msg );
    XEvent e;
    e.xclient.type = ClientMessage;
    e.xclient.message_type = XInternAtom( QX11Info::display(), "_KDE_SPLASH_PROGRESS", False );
    e.xclient.display = QX11Info::display();
    e.xclient.window = QX11Info::appRootWindow();
    e.xclient.format = 8;
    assert( strlen( msg.toLatin1()) < 20 );
    strcpy( e.xclient.data.b, msg.toLatin1());
    XSendEvent( QX11Info::display(), QX11Info::appRootWindow(), False, SubstructureNotifyMask, &e );
}
