/*****************************************************************
ksmserver - the KDE session management server

Copyright (C) 2000 Matthias Ettrich <ettrich@kde.org>
******************************************************************/

#ifndef SERVER_H
#define SERVER_H

#include <qobject.h>
#include <qstring.h>
#include <qsocketnotifier.h>
#include <qlist.h>
#include <qvaluelist.h>
#include <qcstring.h>
#include <qdict.h>
#include <qqueue.h>
#include <qptrdict.h>
#include <qapplication.h>

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

    QList<SmProp> properties;
    SmProp* property( const char* name ) const;

    QString program() const;
    QStringList restartCommand() const;
    QStringList discardCommand() const;
    int restartStyleHint() const;
    QString userId() const;

private:
    const char* clientId;
    SmsConn smsConn;
};

class KSMServer : public QObject
{
Q_OBJECT
public:
    KSMServer();
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

    bool restoreSession( const QString& wm );

public slots:
    void shutdown();

private slots:
    void newConnection( int socket );
    void processData( int socket );
    void timeoutQuit();
    void restoreSessionInternal();

private:
    void handlePendingInteractions();
    void cancelShutdown();
    void completeShutdown();
    void completeKilling();

    void discardSession();
    void storeSesssion();

 private:
    QList<KSMListener> listener;
    QList<KSMClient> clients;

    enum State { Idle, Shutdown, Killing }; // KSMServer does not support pure checkpoints yet.
    State state;
    bool saveSession;

    KSMClient* clientInteracting;
    QString wm;
};

#endif
