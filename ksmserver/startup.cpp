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

#include <qfile.h>
#include <qtextstream.h>
#include <qdatastream.h>
#include <qpushbutton.h>
#include <qmessagebox.h>
#include <qtimer.h>
#include <QDesktopWidget>

#include <klocale.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <unistd.h>
#include <kapplication.h>
#include <kstaticdeleter.h>
#include <ktempfile.h>
#include <kprocess.h>
#include <dcopclient.h>
#include <dcopref.h>
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
    kdDebug( 1218 ) << "KSMServer::restoreSession " << sessionName << endl;
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
		wmCommands << config->readListEntry( QString("restartCommand")+n );
	    }
	}
    }
    if ( wmCommands.isEmpty() )
        wmCommands << ( QStringList() << wm );

    publishProgress( appsToStart, true );
    connectDCOPSignal( launcher, launcher, "autoStartDone()",
                       "restoreSessionInternal()", true);
    connectDCOPSignal( launcher, launcher, "autoStart2Done()",
                       "restoreSessionDoneInternal()", true);
    upAndRunning( "ksmserver" );

    if ( !wmCommands.isEmpty() ) {
        // when we have a window manager, we start it first and give
        // it some time before launching other processes. Results in a
        // visually more appealing startup.
        for (int i = 0; i < wmCommands.count(); i++)
            startApplication( wmCommands[i] );
        QTimer::singleShot( 4000, this, SLOT( autoStart() ) );
    } else {
        autoStart();
    }
}

/*!
  Starts the default session.

  Currently, that's the window manager only (if specified).
 */
void KSMServer::startDefaultSession()
{
    sessionGroup = "";
    publishProgress( 0, true );
    upAndRunning( "ksmserver" );
    connectDCOPSignal( launcher, launcher, "autoStartDone()",
                       "autoStart2()", true);
    connectDCOPSignal( launcher, launcher, "autoStart2Done()",
                       "restoreSessionDoneInternal()", true);
    startApplication( QStringList( wm ) );
    QTimer::singleShot( 4000, this, SLOT( autoStart() ) );
}


void KSMServer::autoStart()
{
    static bool beenThereDoneThat = false;
    if ( beenThereDoneThat )
        return;
    beenThereDoneThat = true;
    DCOPRef( launcher ).send( "autoStart", (int) 1 );
}

void KSMServer::autoStart2()
{
    static bool beenThereDoneThat = false;
    if ( beenThereDoneThat )
        return;
    beenThereDoneThat = true;
    DCOPRef( launcher ).send( "autoStart", (int) 2 );
}


void KSMServer::clientSetProgram( KSMClient* client )
{
    if ( !wm.isEmpty() && client->program() == wm )
        autoStart();
}

void KSMServer::clientRegistered( const char* previousId )
{
    if ( previousId && lastIdStarted == previousId )
        tryRestoreNext();
}


void KSMServer::suspendStartup()
{
    ++startupSuspendCount;
}

void KSMServer::resumeStartup()
{
    if( startupSuspendCount > 0 ) {
        --startupSuspendCount;
        if( startupSuspendCount == 0 && startupSuspendTimeoutTimer.isActive())
            restoreNext();
    }
}

void KSMServer::startupSuspendTimeout()
{
    kdDebug( 1218 ) << "Startup suspend timeout" << endl;
    startupSuspendCount = 0;
    restoreNext();
}

void KSMServer::restoreSessionInternal()
{
    disconnectDCOPSignal( launcher, launcher, "autoStartDone()",
                          "restoreSessionInternal()");
    lastAppStarted = 0;
    lastIdStarted.clear();
    tryRestoreNext();
}

void KSMServer::tryRestoreNext()
{
    if( startupSuspendCount > 0 ) {
        startupSuspendTimeoutTimer.start( 10000, true );
        return;
    }
    restoreNext();
}

void KSMServer::restoreNext()
{
    restoreTimer.stop();
    startupSuspendTimeoutTimer.stop();
    KConfig* config = KGlobal::config();
    config->setGroup( sessionGroup );

    while ( lastAppStarted < appsToStart ) {
        publishProgress ( appsToStart - lastAppStarted );
        lastAppStarted++;
        QString n = QString::number(lastAppStarted);
        QStringList restartCommand = config->readListEntry( QString("restartCommand")+n );
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
            restoreTimer.start( 2000, TRUE );
            return; // we get called again from the clientRegistered handler
        }
    }

    appsToStart = 0;
    lastIdStarted.clear();
    publishProgress( 0 );

    autoStart2();
}


void KSMServer::restoreSessionDoneInternal()
{
    disconnectDCOPSignal( launcher, launcher, "autoStart2Done()",
                          "restoreSessionDoneInternal()");
#ifndef NO_LEGACY_SESSION_MANAGEMENT
    restoreLegacySession( KGlobal::config());
#endif
    upAndRunning( "session ready" );
    DCOPRef( "knotify" ).send( "sessionReady" ); // knotify startup optimization
    KNotifyClient::event( 0, "startkde" ); // this is the time KDE is up

    setupXIOErrorHandler(); // From now on handle X errors as normal shutdown.
}

void KSMServer::publishProgress( int progress, bool max  )
{
    DCOPRef( "ksplash" ).send( max ? "setMaxProgress" : "setProgress", progress );
}


void KSMServer::upAndRunning( const QString& msg )
{
    DCOPRef( "ksplash" ).send( "upAndRunning", msg );
    XEvent e;
    e.xclient.type = ClientMessage;
    e.xclient.message_type = XInternAtom( QX11Info::display(), "_KDE_SPLASH_PROGRESS", False );
    e.xclient.display = QX11Info::display();
    e.xclient.window = QX11Info::appRootWindow();
    e.xclient.format = 8;
    assert( strlen( msg.latin1()) < 20 );
    strcpy( e.xclient.data.b, msg.latin1());
    XSendEvent( QX11Info::display(), QX11Info::appRootWindow(), False, SubstructureNotifyMask, &e );
}
