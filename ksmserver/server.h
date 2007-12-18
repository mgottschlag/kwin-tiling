/*****************************************************************
ksmserver - the KDE session management server

Copyright 2000 Matthias Ettrich <ettrich@kde.org>

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

#ifndef SERVER_H
#define SERVER_H

// needed to avoid clash with INT8 defined in X11/Xmd.h on solaris
#define QT_CLEAN_NAMESPACE 1
#include <QStringList>
#include <QObject>

#include <QSocketNotifier>
#include <kapplication.h>
#include <kworkspace.h>
#include <QTimer>
#include <QTime>
#include <QMap>
#include <Qt3Support/Q3CString>
#include "server2.h"

#define SESSION_PREVIOUS_LOGOUT "saved at previous logout"
#define SESSION_BY_USER  "saved by user"

class KSMListener;
class KSMConnection;
class KSMClient;

class OrgKdeKLauncherInterface;
class QDBusInterface;

enum SMType { SM_ERROR, SM_WMCOMMAND, SM_WMSAVEYOURSELF };
struct SMData
    {
    SMType type;
    QStringList wmCommand;
    QString wmClientMachine;
    QString wmclass1, wmclass2;
    };
typedef QMap<WId,SMData> WindowMap;

class KSMServer : public QObject
{
Q_OBJECT
public:
    KSMServer( const QString& windowManager, bool only_local );
    ~KSMServer();

    static KSMServer* self();

    void* watchConnection( IceConn iceConn );
    void removeConnection( KSMConnection* conn );

    KSMClient* newClient( SmsConn );
    void  deleteClient( KSMClient* client );

    // callbacks
    void saveYourselfDone( KSMClient* client, bool success );
    void interactRequest( KSMClient* client, int dialogType );
    void interactDone( KSMClient* client, bool cancelShutdown );
    void phase2Request( KSMClient* client );

    // error handling
    void ioError( IceConn iceConn );

    // notification
    void clientSetProgram( KSMClient* client );
    void clientRegistered( const char* previousId );

    // public API
    void restoreSession( const QString &sessionName );
    void startDefaultSession();
    void shutdown( KWorkSpace::ShutdownConfirm confirm,
                   KWorkSpace::ShutdownType sdtype,
                   KWorkSpace::ShutdownMode sdmode );

    virtual void suspendStartup( const QString &app );
    virtual void resumeStartup( const QString &app );

public Q_SLOTS:
    void cleanUp();

private Q_SLOTS:
    void newConnection( int socket );
    void processData( int socket );

    void protectionTimeout();
    void timeoutQuit();
    void timeoutWMQuit();
    void kcmPhase1Timeout();
    void kcmPhase2Timeout();
    void pendingShutdownTimeout();

    void autoStart0();
    void autoStart1();
    void autoStart2();
    void tryRestoreNext();
    void startupSuspendTimeout();

    void logoutSoundFinished();
    void autoStart0Done();
    void autoStart1Done();
    void autoStart2Done();
    void kcmPhase1Done();
    void kcmPhase2Done();

private:
    void handlePendingInteractions();
    void completeShutdownOrCheckpoint();
    void startKilling();
    void performStandardKilling();
    void completeKilling();
    void killWM();
    void completeKillingWM();
    void cancelShutdown( KSMClient* c );
    void killingCompleted();
    void createLogoutEffectWidget();

    void discardSession();
    void storeSession();

    void startProtection();
    void endProtection();

    void startApplication( QStringList& command,
        const QString& clientMachine = QString(),
        const QString& userId = QString() );
    void executeCommand( const QStringList& command );

    bool isWM( const KSMClient* client ) const;
    bool isWM( const QString& command ) const;
    bool defaultSession() const; // empty session
    void setupXIOErrorHandler();

    void performLegacySessionSave();
    void storeLegacySession( KConfig* config );
    void restoreLegacySession( KConfig* config );
    void restoreLegacySessionInternal( KConfigGroup* config, char sep = ',' );
    QStringList windowWmCommand(WId w);
    QString windowWmClientMachine(WId w);
    WId windowWmClientLeader(WId w);
    QByteArray windowSessionId(WId w, WId leader);

    bool checkStartupSuspend();
    void finishStartup();
    void resumeStartupInternal();

    // public dcop interface

 public Q_SLOTS: //public dcop interface
    void logout( int, int, int );
    QString currentSession();
    void saveCurrentSession();
    void saveCurrentSessionAs( const QString & );
    QStringList sessionList();
 private:
    QList<KSMListener*> listener;
    QList<KSMClient*> clients;

    enum State
        {
        Idle,
        LaunchingWM, AutoStart0, KcmInitPhase1, AutoStart1, Restoring, FinishingStartup, // startup
        Shutdown, Checkpoint, Killing, KillingWM, WaitingForKNotify // shutdown
        };
    State state;
    bool dialogActive;
    bool saveSession;
    int wmPhase1WaitingCount;
    int saveType;
    QMap< QString, int > startupSuspendCount;

    KWorkSpace::ShutdownType shutdownType;
    KWorkSpace::ShutdownMode shutdownMode;
    QString bootOption;

    bool clean;
    KSMClient* clientInteracting;
    QString wm;
    QString sessionGroup;
    QString sessionName;
    QTimer protectionTimer;
    QTimer restoreTimer;
    QString xonCommand;
    QTimer startupSuspendTimeoutTimer;
    bool waitAutoStart2;
    bool waitKcmInit2;
    QTimer pendingShutdown;
    QWidget* logoutEffectWidget;
    KWorkSpace::ShutdownConfirm pendingShutdown_confirm;
    KWorkSpace::ShutdownType pendingShutdown_sdtype;
    KWorkSpace::ShutdownMode pendingShutdown_sdmode;

    // ksplash interface
    void upAndRunning( const QString& msg );
    void publishProgress( int progress, bool max  = false  );

    // sequential startup
    int appsToStart;
    int lastAppStarted;
    QString lastIdStarted;

    QStringList excludeApps;

    WindowMap legacyWindows;
    
    OrgKdeKLauncherInterface* klauncherSignals;
    QDBusInterface* kcminitSignals;
};

#endif
