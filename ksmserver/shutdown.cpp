/*****************************************************************
ksmserver - the KDE session management server

Copyright (C) 2000 Matthias Ettrich <ettrich@kde.org>

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
#include <ktemporaryfile.h>
#include <kprocess.h>
#include <kwinmodule.h>
#include <knotification.h>
#include <dmctl.h>
#include "server.h"
#include "global.h"
#include "client.h"
#include "shutdowndlg.h"


#include <kdebug.h>

#include <QX11Info>
#include <QApplication>

void KSMServer::logout( int confirm, int sdtype, int sdmode )
{
    shutdown( (KWorkSpace::ShutdownConfirm)confirm,
            (KWorkSpace::ShutdownType)sdtype,
            (KWorkSpace::ShutdownMode)sdmode );
}

void KSMServer::shutdown( KWorkSpace::ShutdownConfirm confirm,
    KWorkSpace::ShutdownType sdtype, KWorkSpace::ShutdownMode sdmode )
{
    pendingShutdown.stop();
    if( dialogActive )
        return;
    if( state >= Shutdown ) // already performing shutdown
        return;
    if( state != Idle ) // performing startup
    {
    // perform shutdown as soon as startup is finished, in order to avoid saving partial session
        if( !pendingShutdown.isActive())
        {
            pendingShutdown.start( 1000 );
            pendingShutdown_confirm = confirm;
            pendingShutdown_sdtype = sdtype;
            pendingShutdown_sdmode = sdmode;
        }
        return;
    }

    KSharedConfig::Ptr config = KGlobal::config();
    config->reparseConfiguration(); // config may have changed in the KControl module

    config->setGroup("General" );

    bool logoutConfirmed =
        (confirm == KWorkSpace::ShutdownConfirmYes) ? false :
    (confirm == KWorkSpace::ShutdownConfirmNo) ? true :
                !config->readEntry( "confirmLogout", true );
    bool maysd = false;
    if (config->readEntry( "offerShutdown", true ) && DM().canShutdown())
        maysd = true;
    if (!maysd) {
        if (sdtype != KWorkSpace::ShutdownTypeNone &&
            sdtype != KWorkSpace::ShutdownTypeDefault &&
            logoutConfirmed)
            return; /* unsupported fast shutdown */
        sdtype = KWorkSpace::ShutdownTypeNone;
    } else if (sdtype == KWorkSpace::ShutdownTypeDefault)
        sdtype = (KWorkSpace::ShutdownType)
                config->readEntry( "shutdownType", (int)KWorkSpace::ShutdownTypeNone );
    if (sdmode == KWorkSpace::ShutdownModeDefault)
        sdmode = KWorkSpace::ShutdownModeInteractive;

    dialogActive = true;
    QString bopt;
    if ( !logoutConfirmed ) {
        KSMShutdownFeedback::start(); // make the screen gray
        logoutConfirmed =
            KSMShutdownDlg::confirmShutdown( maysd, sdtype, bopt );
        // ###### We can't make the screen remain gray while talking to the apps,
        // because this prevents interaction ("do you want to save", etc.)
        // TODO: turn the feedback widget into a list of apps to be closed,
        // with an indicator of the current status for each.
        KSMShutdownFeedback::stop(); // make the screen become normal again
    }

    if ( logoutConfirmed ) {

        shutdownType = sdtype;
        shutdownMode = sdmode;
        bootOption = bopt;

        // shall we save the session on logout?
        saveSession = ( config->readEntry( "loginMode", "restorePreviousLogout" ) == "restorePreviousLogout" );

        if ( saveSession )
            sessionGroup = QString("Session: ") + SESSION_PREVIOUS_LOGOUT;

        // Set the real desktop background to black so that exit looks
        // clean regardless of what was on "our" desktop.
                QPalette palette;
        palette.setColor( kapp->desktop()->backgroundRole(), Qt::black );
        kapp->desktop()->setPalette(palette);
        state = Shutdown;
        wmPhase1WaitingCount = 0;
        saveType = saveSession?SmSaveBoth:SmSaveGlobal;
#ifndef NO_LEGACY_SESSION_MANAGEMENT
        performLegacySessionSave();
#endif
        startProtection();
        foreach( KSMClient* c, clients ) {
            c->resetState();
            // Whoever came with the idea of phase 2 got it backwards
            // unfortunately. Window manager should be the very first
            // one saving session data, not the last one, as possible
            // user interaction during session save may alter
            // window positions etc.
            // Moreover, KWin's focus stealing prevention would lead
            // to undesired effects while session saving (dialogs
            // wouldn't be activated), so it needs be assured that
            // KWin will turn it off temporarily before any other
            // user interaction takes place.
            // Therefore, make sure the WM finishes its phase 1
            // before others a chance to change anything.
            // KWin will check if the session manager is ksmserver,
            // and if yes it will save in phase 1 instead of phase 2.
            if( isWM( c )) {
                ++wmPhase1WaitingCount;
                SmsSaveYourself( c->connection(), saveType,
                            true, SmInteractStyleAny, false );
            }

        }
        if( wmPhase1WaitingCount == 0 ) { // no WM, simply start them all
            foreach( KSMClient* c, clients )
                SmsSaveYourself( c->connection(), saveType,
                            true, SmInteractStyleAny, false );
        }
        if ( clients.isEmpty() )
            completeShutdownOrCheckpoint();
    }
    dialogActive = false;
}

void KSMServer::pendingShutdownTimeout()
{
    shutdown( pendingShutdown_confirm, pendingShutdown_sdtype, pendingShutdown_sdmode );
}

void KSMServer::saveCurrentSession()
{
    if ( state != Idle || dialogActive )
        return;

    if ( currentSession().isEmpty() || currentSession() == SESSION_PREVIOUS_LOGOUT )
        sessionGroup = QString("Session: ") + SESSION_BY_USER;

    state = Checkpoint;
    wmPhase1WaitingCount = 0;
    saveType = SmSaveLocal;
    saveSession = true;
#ifndef NO_LEGACY_SESSION_MANAGEMENT
    performLegacySessionSave();
#endif
    foreach( KSMClient* c, clients ) {
        c->resetState();
        if( isWM( c )) {
            ++wmPhase1WaitingCount;
            SmsSaveYourself( c->connection(), saveType, false, SmInteractStyleNone, false );
        }
    }
    if( wmPhase1WaitingCount == 0 ) {
        foreach( KSMClient* c, clients )
            SmsSaveYourself( c->connection(), saveType, false, SmInteractStyleNone, false );
    }
    if ( clients.isEmpty() )
        completeShutdownOrCheckpoint();
}

void KSMServer::saveCurrentSessionAs( QString session )
{
    if ( state != Idle || dialogActive )
        return;
    sessionGroup = "Session: " + session;
    saveCurrentSession();
}

// callbacks
void KSMServer::saveYourselfDone( KSMClient* client, bool success )
{
    if ( state == Idle ) {
        // State saving when it's not shutdown or checkpoint. Probably
        // a shutdown was canceled and the client is finished saving
        // only now. Discard the saved state in order to avoid
        // the saved data building up.
        QStringList discard = client->discardCommand();
        if( !discard.isEmpty())
            executeCommand( discard );
        return;
    }
    if ( success ) {
        client->saveYourselfDone = true;
        completeShutdownOrCheckpoint();
    } else {
        // fake success to make KDE's logout not block with broken
        // apps. A perfect ksmserver would display a warning box at
        // the very end.
        client->saveYourselfDone = true;
        completeShutdownOrCheckpoint();
    }
    startProtection();
    if( isWM( client ) && !client->wasPhase2 && wmPhase1WaitingCount > 0 ) {
        --wmPhase1WaitingCount;
        // WM finished its phase1, save the rest
        if( wmPhase1WaitingCount == 0 ) {
            foreach( KSMClient* c, clients )
                if( !isWM( c ))
                    SmsSaveYourself( c->connection(), saveType, saveType != SmSaveLocal,
                        saveType != SmSaveLocal ? SmInteractStyleAny : SmInteractStyleNone,
                        false );
        }
    }
}

void KSMServer::interactRequest( KSMClient* client, int /*dialogType*/ )
{
    if ( state == Shutdown )
        client->pendingInteraction = true;
    else
        SmsInteract( client->connection() );

    handlePendingInteractions();
}

void KSMServer::interactDone( KSMClient* client, bool cancelShutdown_ )
{
    if ( client != clientInteracting )
        return; // should not happen
    clientInteracting = 0;
    if ( cancelShutdown_ )
        cancelShutdown( client );
    else
        handlePendingInteractions();
}


void KSMServer::phase2Request( KSMClient* client )
{
    client->waitForPhase2 = true;
    client->wasPhase2 = true;
    completeShutdownOrCheckpoint();
    if( isWM( client ) && wmPhase1WaitingCount > 0 ) {
        --wmPhase1WaitingCount;
        // WM finished its phase1 and requests phase2, save the rest
        if( wmPhase1WaitingCount == 0 ) {
            foreach( KSMClient* c, clients )
                if( !isWM( c ))
                    SmsSaveYourself( c->connection(), saveType, saveType != SmSaveLocal,
                        saveType != SmSaveLocal ? SmInteractStyleAny : SmInteractStyleNone,
                        false );
        }
    }
}

void KSMServer::handlePendingInteractions()
{
    if ( clientInteracting )
        return;

    foreach( KSMClient* c, clients ) {
        if ( c->pendingInteraction ) {
            clientInteracting = c;
            c->pendingInteraction = false;
            break;
        }
    }
    if ( clientInteracting ) {
        endProtection();
        SmsInteract( clientInteracting->connection() );
    } else {
        startProtection();
    }
}


void KSMServer::cancelShutdown( KSMClient* c )
{
    kDebug( 1218 ) << "Client " << c->program() << " (" << c->clientId() << ") canceled shutdown." << endl;
    KNotification::event( "cancellogout" , i18n( "Logout canceled by '%1'", c->program()),
        QPixmap() , 0l , KNotification::DefaultEvent  );
    clientInteracting = 0;
    foreach( KSMClient* c, clients ) {
        SmsShutdownCancelled( c->connection() );
        if( c->saveYourselfDone ) {
            // Discard also saved state.
            QStringList discard = c->discardCommand();
            if( !discard.isEmpty())
                executeCommand( discard );
        }
    }
    state = Idle;
}

void KSMServer::startProtection()
{
    protectionTimer.setSingleShot( true );
    protectionTimer.start( 10000 );
}

void KSMServer::endProtection()
{
    protectionTimer.stop();
}

/*
Internal protection slot, invoked when clients do not react during
shutdown.
*/
void KSMServer::protectionTimeout()
{
    if ( ( state != Shutdown && state != Checkpoint ) || clientInteracting )
        return;

    foreach( KSMClient* c, clients ) {
        if ( !c->saveYourselfDone && !c->waitForPhase2 ) {
            kDebug( 1218 ) << "protectionTimeout: client " << c->program() << "(" << c->clientId() << ")" << endl;
            c->saveYourselfDone = true;
        }
    }
    completeShutdownOrCheckpoint();
    startProtection();
}

void KSMServer::completeShutdownOrCheckpoint()
{
    if ( state != Shutdown && state != Checkpoint )
        return;

    foreach( KSMClient* c, clients ) {
        if ( !c->saveYourselfDone && !c->waitForPhase2 )
            return; // not done yet
    }

    // do phase 2
    bool waitForPhase2 = false;
    foreach( KSMClient* c, clients ) {
        if ( !c->saveYourselfDone && c->waitForPhase2 ) {
            c->waitForPhase2 = false;
            SmsSaveYourselfPhase2( c->connection() );
            waitForPhase2 = true;
        }
    }
    if ( waitForPhase2 )
        return;

    if ( saveSession )
        storeSession();
    else
        discardSession();

    if ( state == Shutdown ) {
#ifdef __GNUC__
#warning KNotify TODO
#endif
	/*  How to check if the deamon is still running.    We will not start the knotify daemon just for playing a sound before shutdown.  or do wa want that ?

        knotifySignals = QDBus::sessionBus().findInterface("org.kde.knotify",
            "/knotify", "org.kde.KNotify" );
        if( !knotifySignals->isValid())
            kWarning() << "knotify not running?" << endl;
	*/

        KNotification *n = KNotification::event( "exitkde" , QString() , QPixmap() , 0l ,  KNotification::DefaultEvent  ); // KDE says good bye
        connect(n, SIGNAL( closed() ) , this, SLOT(logoutSoundFinished()) );
	state = WaitingForKNotify;
 
    } else if ( state == Checkpoint ) {
        foreach( KSMClient* c, clients ) {
            SmsSaveComplete( c->connection());
        }
        state = Idle;
    }
}

void KSMServer::startKilling()
{
    // kill all clients
    state = Killing;
    foreach( KSMClient* c, clients ) {
        if( isWM( c )) // kill the WM as the last one in order to reduce flicker
            continue;
        kDebug( 1218 ) << "completeShutdown: client " << c->program() << "(" << c->clientId() << ")" << endl;
        SmsDie( c->connection() );
    }

    kDebug( 1218 ) << " We killed all clients. We have now clients.count()=" <<
    clients.count() << endl;
    completeKilling();
    QTimer::singleShot( 10000, this, SLOT( timeoutQuit() ) );
}

void KSMServer::completeKilling()
{
    kDebug( 1218 ) << "KSMServer::completeKilling clients.count()=" <<
        clients.count() << endl;
    if( state == Killing ) {
        bool wait = false;
        foreach( KSMClient* c, clients ) {
            if( isWM( c ))
                continue;
            wait = true; // still waiting for clients to go away
        }
        if( wait )
            return;
        killWM();
    }
}

void KSMServer::killWM()
{
    state = KillingWM;
    bool iswm = false;
    foreach( KSMClient* c, clients ) {
        if( isWM( c )) {
            iswm = true;
            kDebug( 1218 ) << "killWM: client " << c->program() << "(" << c->clientId() << ")" << endl;
            SmsDie( c->connection() );
        }
    }
    if( iswm ) {
        completeKillingWM();
        QTimer::singleShot( 5000, this, SLOT( timeoutWMQuit() ) );
    }
    else
        killingCompleted();
}
  
void KSMServer::completeKillingWM()
{
    kDebug( 1218 ) << "KSMServer::completeKillingWM clients.count()=" <<
        clients.count() << endl;
    if( state == KillingWM ) {
        if( clients.isEmpty())
            killingCompleted();
    }
}

// shutdown is fully complete
void KSMServer::killingCompleted()
{
    kapp->quit();
}

void KSMServer::logoutSoundFinished(  )
{
    if( state != WaitingForKNotify )
        return;
    startKilling();
}

void KSMServer::timeoutQuit()
{
    foreach( KSMClient* c, clients ) {
        kWarning( 1218 ) << "SmsDie timeout, client " << c->program() << "(" << c->clientId() << ")" << endl;
    }
    killWM();
}

void KSMServer::timeoutWMQuit()
{
    if( state == KillingWM ) {
        kWarning( 1218 ) << "SmsDie WM timeout" << endl;
    }
    killingCompleted();
}
