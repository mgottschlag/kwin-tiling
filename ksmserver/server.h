/*****************************************************************
ksmserver - the KDE session management server

Copyright (C) 2000 Matthias Ettrich <ettrich@kde.org>
******************************************************************/

#ifndef SERVER_H
#define SERVER_H

// needed to avoid clash with INT8 defined in X11/Xmd.h on solaris
#define QT_CLEAN_NAMESPACE 1
#include <qobject.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qsocketnotifier.h>
#include <kapplication.h>
#include <kworkspace.h>
#include <qtimer.h>
#include <q3cstring.h>
#include <QTime>
#include <qmap.h>
#include <dcopobject.h>

#include "server2.h"
#include "KSMServerInterface.h"

#define SESSION_PREVIOUS_LOGOUT "saved at previous logout"
#define SESSION_BY_USER  "saved by user"

typedef QList<Q3CString> QCStringList;
class KSMListener;
class KSMConnection;
class KSMClient;

enum SMType { SM_ERROR, SM_WMCOMMAND, SM_WMSAVEYOURSELF };
struct SMData
    {
    SMType type;
    QStringList wmCommand;
    QString wmClientMachine;
    QString wmclass1, wmclass2;
    };
typedef QMap<WId,SMData> WindowMap;

class KSMServer : public QObject, public KSMServerInterface
{
Q_OBJECT
K_DCOP
k_dcop:
    void notifySlot(QString,QString,QString,QString,QString,int,int,int,int);
    void logoutSoundFinished(int,int);
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
    void restoreSession( QString sessionName );
    void startDefaultSession();
    void shutdown( KWorkSpace::ShutdownConfirm confirm,
                   KWorkSpace::ShutdownType sdtype,
                   KWorkSpace::ShutdownMode sdmode );

    virtual void suspendStartup();
    virtual void resumeStartup();

public Q_SLOTS:
    void cleanUp();

private Q_SLOTS:
    void newConnection( int socket );
    void processData( int socket );
    void restoreSessionInternal();
    void restoreSessionDoneInternal();

    void protectionTimeout();
    void timeoutQuit();
    void knotifyTimeout();

    void autoStart();
    void autoStart2();
    void tryRestoreNext();
    void restoreNext();
    void startupSuspendTimeout();

private:
    void handlePendingInteractions();
    void completeShutdownOrCheckpoint();
    void startKilling();
    void completeKilling();
    void cancelShutdown( KSMClient* c );

    void discardSession();
    void storeSession();

    void startProtection() { protectionTimer.setSingleShot( true ); protectionTimer.start( 8000 ); }
    void endProtection() { protectionTimer.stop(); }

    void startApplication( QStringList command,
        const QString& clientMachine = QString(),
        const QString& userId = QString() );
    void executeCommand( const QStringList& command );

    bool isWM( const KSMClient* client ) const;
    void setupXIOErrorHandler();

    void performLegacySessionSave();
    void storeLegacySession( KConfig* config );
    void restoreLegacySession( KConfig* config );
    void restoreLegacySessionInternal( KConfig* config, char sep = ',' );
    QStringList windowWmCommand(WId w);
    QString windowWmClientMachine(WId w);
    WId windowWmClientLeader(WId w);
    QByteArray windowSessionId(WId w, WId leader);

    // public dcop interface
    void logout( int, int, int );
    QStringList sessionList();
    QString currentSession();
    void saveCurrentSession();
    void saveCurrentSessionAs( QString );

 private:
    QList<KSMListener*> listener;
    QList<KSMClient*> clients;

    enum State { Idle, Shutdown, Checkpoint, Killing, Killing2, WaitingForKNotify };
    State state;
    bool dialogActive;
    bool saveSession;
    int wmPhase1WaitingCount;
    int saveType;
    int startupSuspendCount;

    KWorkSpace::ShutdownType shutdownType;
    KWorkSpace::ShutdownMode shutdownMode;
    QString bootOption;

    bool clean;
    KSMClient* clientInteracting;
    QString wm;
    QString sessionGroup;
    QString sessionName;
    Q3CString launcher;
    QTimer protectionTimer;
    QTimer restoreTimer;
    QString xonCommand;
    int logoutSoundEvent;
    QTimer knotifyTimeoutTimer;
    QTimer startupSuspendTimeoutTimer;

    // ksplash interface
    void upAndRunning( const QString& msg );
    void publishProgress( int progress, bool max  = false  );

    // sequential startup
    int appsToStart;
    int lastAppStarted;
    QString lastIdStarted;

    QStringList excludeApps;

    WindowMap legacyWindows;
};

#endif
