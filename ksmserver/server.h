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
#include <qptrlist.h>
#include <qvaluelist.h>
#include <qcstring.h>
#include <qdict.h>
#include <qptrqueue.h>
#include <qptrdict.h>
#include <qapplication.h>
#include <qtimer.h>

#define INT32 QINT32
#include <X11/Xlib.h>
#include <X11/Xmd.h>
#include <X11/ICE/ICElib.h>
extern "C" {
#include <X11/ICE/ICEutil.h>
#include <X11/ICE/ICEmsg.h>
#include <X11/ICE/ICEproto.h>
#include <X11/SM/SM.h>
#include <X11/SM/SMlib.h>
}

#include "KSMServerInterface.h"

#define SESSION_PREVIOUS_LOGOUT "saved at previous logout"
#define SESSION_BY_USER  "saved by user"

typedef QValueList<QCString> QCStringList;
class KSMListener;
class KSMConnection;
class KSMClient
{
public:
    KSMClient( SmsConn );
    ~KSMClient();

    void registerClient( const char* previousId  = 0 );
    SmsConn connection() const { return smsConn; }

    void resetState();
    uint saveYourselfDone : 1;
    uint pendingInteraction : 1;
    uint waitForPhase2 : 1;
    uint wasPhase2 : 1;

    QPtrList<SmProp> properties;
    SmProp* property( const char* name ) const;

    QString program() const;
    QStringList restartCommand() const;
    QStringList discardCommand() const;
    int restartStyleHint() const;
    QString userId() const;
    const char* clientId() { return id ? id : ""; }

private:
    const char* id;
    SmsConn smsConn;
};

#ifndef NO_LEGACY_SESSION_MANAGEMENT
enum SMType { SM_ERROR, SM_WMCOMMAND, SM_WMSAVEYOURSELF };
struct SMData
    {
    SMType type;
    QStringList wmCommand;
    QString wmClientMachine;
    };
typedef QMap<WId,SMData> WindowMap;
#endif

class KSMServer : public QObject, public KSMServerInterface
{
Q_OBJECT
public:
    KSMServer( const QString& windowManager, bool only_local );
    ~KSMServer();

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
    void shutdown( KApplication::ShutdownConfirm confirm,
                   KApplication::ShutdownType sdtype,
                   KApplication::ShutdownMode sdmode );

public slots:
    void cleanUp();

private slots:
    void newConnection( int socket );
    void processData( int socket );
    void restoreSessionInternal();
    void restoreSessionDoneInternal();

    void protectionTimeout();

    void autoStart();
    void autoStart2();
    void restoreNextInternal();

private:
    void handlePendingInteractions();
    void completeShutdownOrCheckpoint();
    void completeKilling();
    void cancelShutdown( KSMClient* c );

    void discardSession();
    void storeSession();

    void startProtection() { protectionTimer.start( 8000, TRUE ); }
    void endProtection() { protectionTimer.stop(); }

    void startApplication( const QStringList& command );
    void executeCommand( const QStringList& command );

#ifndef NO_LEGACY_SESSION_MANAGEMENT
    void performLegacySessionSave();
    void storeLegacySession( KConfig* config );
    void restoreLegacySession( KConfig* config );
    void restoreLegacySessionInternal( KConfig* config, char sep = ',' );
    QStringList windowWmCommand(WId w);
    QString windowWmClientMachine(WId w);
    WId windowWmClientLeader(WId w);
    QCString windowSessionId(WId w, WId leader);
#endif

    // public dcop interface
    void logout( int, int, int );
    QStringList sessionList();
    QString currentSession();
    void saveCurrentSession();
    void saveCurrentSessionAs( QString );

 private:
    QPtrList<KSMListener> listener;
    QPtrList<KSMClient> clients;

    enum State { Idle, Shutdown, Checkpoint, Killing };
    State state;
    bool dialogActive;
    bool saveSession;

    bool clean;
    KSMClient* clientInteracting;
    QString wm;
    QString sessionGroup;
    QString sessionName;
    QCString launcher;
    QTimer protectionTimer;
    QTimer restoreTimer;

    // ksplash interface
    void upAndRunning( const QString& msg );
    void publishProgress( int progress, bool max  = false  );

    // sequential startup
    int appsToStart;
    int lastAppStarted;
    QString lastIdStarted;

#ifndef NO_LEGACY_SESSION_MANAGEMENT
    WindowMap legacyWindows;
#endif
};

#endif

