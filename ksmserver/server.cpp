/*****************************************************************
ksmserver - the KDE session management server

Copyright (C) 2000 Matthias Ettrich <ettrich@kde.org>

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
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_VFORK_H
#include <vfork.h>
#endif

#include <sys/types.h>
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif

#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif

#define QT_CLEAN_NAMESPACE 1
#include <qfile.h>
#include <qtextstream.h>
#include <qdatastream.h>
#include <qstack.h>
#include <qpushbutton.h>
#include <qmessagebox.h>
#include <qguardedptr.h>
#include <qtimer.h>
#include <klocale.h>
#include <kglobal.h>
#include <kconfig.h>
#include <unistd.h>

#include "server.h"
#include "global.h"
#include "shutdown.h"

extern "C" {
    int umask(...);

}
KSMServer* the_server = 0;

KSMClient::KSMClient( SmsConn conn)
{
    smsConn = conn;
    clientId = 0;
    resetState();
}

KSMClient::~KSMClient()
{
    for ( SmProp* prop = properties.first(); prop; prop = properties.next() )
	SmFreeProperty( prop );
}

SmProp* KSMClient::property( const char* name ) const
{
    for ( QListIterator<SmProp> it( properties ); it.current(); ++it ) {
	if ( !qstrcmp( it.current()->name, name ) )
	    return it.current();
    }
    return 0;
}

void KSMClient::resetState()
{
    saveYourselfDone = FALSE;
    pendingInteraction = FALSE;
    waitForPhase2 = FALSE;
    phase2Workaround = FALSE;
}

void KSMClient::registerClient( const char* previousId )
{
    clientId = previousId;
    if ( !clientId )
	clientId = SmsGenerateClientID( smsConn );
    SmsRegisterClientReply( smsConn, (char*) clientId );
}


QString KSMClient::program() const
{
    SmProp* p = property( SmProgram );
    if ( !p || qstrcmp( p->type, SmARRAY8) || p->num_vals < 1)
	return QString::null;
    return QString::fromLatin1( (const char*) p->vals[0].value );
}

QStringList KSMClient::restartCommand() const
{
    QStringList result;
    SmProp* p = property( SmRestartCommand );
    if ( !p || qstrcmp( p->type, SmLISTofARRAY8) || p->num_vals < 1)
	return result;
    for ( int i = 0; i < p->num_vals; i++ )
	result +=QString::fromLatin1( (const char*) p->vals[i].value );
    return result;
}

QStringList KSMClient::discardCommand() const
{
    QStringList result;
    SmProp* p = property( SmDiscardCommand );
    if ( !p || qstrcmp( p->type, SmLISTofARRAY8) || p->num_vals < 1)
	return result;
    for ( int i = 0; i < p->num_vals; i++ )
	result +=QString::fromLatin1( (const char*) p->vals[i].value );
    return result;
}

int KSMClient::restartStyleHint() const
{
    SmProp* p = property( SmRestartStyleHint );
    if ( !p || qstrcmp( p->type, SmCARD8) || p->num_vals < 1)
	return SmRestartIfRunning;
    return *((int*)p->vals[0].value);
}

QString KSMClient::userId() const
{
    SmProp* p = property( SmUserID );
    if ( !p || qstrcmp( p->type, SmARRAY8) || p->num_vals < 1)
	return QString::null;
    return QString::fromLatin1( (const char*) p->vals[0].value );
}


/*! Utility function to execute a command on the local machine. Used
 to restart applications and to discard session data
 */
static void executeCommand( const QStringList& command )
{
    if ( command.isEmpty() )
	return;
    QApplication::flushX();
    if ( !vfork() ) {
	int n = command.count();
	char** arglist = (char **)malloc( (n+1)*sizeof(char *));
	for ( int i=0; i < n; i++)
	    arglist[i] = (char*) (*command.at(i)).latin1();
	arglist[n]= 0;
	setpgid(0,0);
	execvp(arglist[0], arglist);
	_exit(-1);
    }
}

IceAuthDataEntry *authDataEntries = 0;
static char *addAuthFile = 0;
static char *remAuthFile = 0;

static IceListenObj *listenObjs = 0;
int numTransports = 0;

static Bool HostBasedAuthProc ( char* /*hostname*/)
{
    return FALSE; // no host based authentication
}


Status KSMRegisterClientProc (
    SmsConn 		/* smsConn */,
    SmPointer		managerData,
    char *		previousId
)
{
    KSMClient* client = (KSMClient*) managerData;
    client->registerClient( previousId );
    return 1;
}

void KSMInteractRequestProc (
    SmsConn		/* smsConn */,
    SmPointer		managerData,
    int			dialogType
)
{
//     qDebug("KSMInteractRequestProc");
    the_server->interactRequest( (KSMClient*) managerData, dialogType );
}

void KSMInteractDoneProc (
    SmsConn		/* smsConn */,
    SmPointer		managerData,
    Bool			cancelShutdown
)
{
//     qDebug("KSMInteractDoneProc");
    the_server->interactDone( (KSMClient*) managerData, cancelShutdown );
}

void KSMSaveYourselfRequestProc (
    SmsConn		/* smsConn */,
    SmPointer		/* managerData */,
    int  		/* saveType */,
    Bool		/* shutdown */,
    int			/* interactStyle */,
    Bool		/* fast */,
    Bool		/* global */
)
{
//     qDebug("KSMSaveYourselfRequestProc");
    the_server->shutdown();
}

void KSMSaveYourselfPhase2RequestProc (
    SmsConn		/* smsConn */,
    SmPointer		managerData
)
{
//     qDebug("KSMSaveYourselfPhase2RequestProc");
    the_server->phase2Request( (KSMClient*) managerData );
}

void KSMSaveYourselfDoneProc (
    SmsConn		/* smsConn */,
    SmPointer		managerData,
    Bool		success
)
{
//     qDebug("KSMSaveYourselfDoneProc");
    the_server->saveYourselfDone( (KSMClient*) managerData, success );
}

void KSMCloseConnectionProc (
    SmsConn		smsConn,
    SmPointer		managerData,
    int			count,
    char **		reasonMsgs
)
{
//     qDebug("KSMCloseConnectionProc %p", managerData);
    the_server->deleteClient( ( KSMClient* ) managerData );
    if ( count )
	SmFreeReasons( count, reasonMsgs );
    IceConn iceConn = SmsGetIceConnection( smsConn );
    SmsCleanUp( smsConn );
    IceSetShutdownNegotiation (iceConn, False);
    IceCloseConnection( iceConn );
}

void KSMSetPropertiesProc (
    SmsConn		/* smsConn */,
    SmPointer		managerData,
    int			numProps,
    SmProp **		props
)
{
    KSMClient* client = ( KSMClient* ) managerData;
    for ( int i = 0; i < numProps; i++ ) {
	SmProp *p = client->property( props[i]->name );
	if ( p )
	    client->properties.removeRef( p );
	client->properties.append( props[i] );
    }
    if ( numProps )
	free( props );
}

void KSMDeletePropertiesProc (
    SmsConn		/* smsConn */,
    SmPointer		managerData,
    int			numProps,
    char **		propNames
)
{
    KSMClient* client = ( KSMClient* ) managerData;
    for ( int i = 0; i < numProps; i++ ) {
	SmProp *p = client->property( propNames[i] );
	if ( p )
	    client->properties.removeRef( p );
    }
}

void KSMGetPropertiesProc (
    SmsConn		smsConn,
    SmPointer		managerData
)
{
//     qDebug("KSMGetPropertiesProc");
    KSMClient* client = ( KSMClient* ) managerData;
    SmProp** props = new SmProp*[client->properties.count()];
    int i = 0;
    for ( SmProp* prop = client->properties.first(); prop; prop = client->properties.next() )
	props[i++] = prop;

    SmsReturnProperties( smsConn, i, props );
    delete [] props;
}


class KSMListener : public QSocketNotifier
{
public:
    KSMListener( IceListenObj obj )
	: QSocketNotifier( IceGetListenConnectionNumber( obj ),
			   QSocketNotifier::Read, 0, 0)
{
    listenObj = obj;
}

    IceListenObj listenObj;
};

class KSMConnection : public QSocketNotifier
{
 public:
  KSMConnection( IceConn conn )
    : QSocketNotifier( IceConnectionNumber( conn ),
		       QSocketNotifier::Read, 0, 0 )
    {
	iceConn = conn;
    }

    IceConn iceConn;
};


/* for printing hex digits */
static void fprintfhex (FILE *fp, unsigned int len, char *cp)
{
    static char hexchars[] = "0123456789abcdef";

    for (; len > 0; len--, cp++) {
	unsigned char s = *cp;
	putc(hexchars[s >> 4], fp);
	putc(hexchars[s & 0x0f], fp);
    }
}

/*
 * We use temporary files which contain commands to add/remove entries from
 * the .ICEauthority file.
 */
static void write_iceauth (FILE *addfp, FILE *removefp, IceAuthDataEntry *entry)
{
    fprintf (addfp,
	     "add %s \"\" %s %s ",
	     entry->protocol_name,
	     entry->network_id,
	     entry->auth_name);
    fprintfhex (addfp, entry->auth_data_length, entry->auth_data);
    fprintf (addfp, "\n");

    fprintf (removefp,
	     "remove protoname=%s protodata=\"\" netid=%s authname=%s\n",
	     entry->protocol_name,
	     entry->network_id,
	     entry->auth_name);
}


#ifndef HAVE_MKSTEMP
static char *unique_filename (const char *path, const char *prefix)
#else
static char *unique_filename (const char *path, const char *prefix, int *pFd)
#endif
{
#ifndef HAVE_MKSTEMP
#ifndef X_NOT_POSIX
    return ((char *) tempnam (path, prefix));
#else
    char tempFile[PATH_MAX];
    char *tmp;

    sprintf (tempFile, "%s/%sXXXXXX", path, prefix);
    tmp = (char *) mktemp (tempFile);
    if (tmp)
	{
	    char *ptr = (char *) malloc (strlen (tmp) + 1);
	    strcpy (ptr, tmp);
	    return (ptr);
	}
    else
	return (NULL);
#endif
#else
    char tempFile[PATH_MAX];
    char *ptr;

    sprintf (tempFile, "%s/%sXXXXXX", path, prefix);
    ptr = (char *)malloc(strlen(tempFile) + 1);
    if (ptr != NULL)
	{
	    strcpy(ptr, tempFile);
	    *pFd =  mkstemp(ptr);
	}
    return ptr;
#endif
}

#define MAGIC_COOKIE_LEN 16

Status SetAuthentication (int count, IceListenObj *listenObjs,
			  IceAuthDataEntry **authDataEntries)
{
    FILE        *addfp = NULL;
    FILE        *removefp = NULL;
    const char  *path;
    int         original_umask;
    char        command[256];
    int         i;
#ifdef HAVE_MKSTEMP
    int         fd;
#endif

    original_umask = ::umask (0077);      /* disallow non-owner access */

    path = getenv ("KSM_SAVE_DIR");
    if (!path)
	path = "/tmp";
#ifndef HAVE_MKSTEMP
    if ((addAuthFile = unique_filename (path, "ksm")) == NULL)
	goto bad;

    if (!(addfp = fopen (addAuthFile, "w")))
	goto bad;

    if ((remAuthFile = unique_filename (path, "ksm")) == NULL)
	goto bad;

    if (!(removefp = fopen (remAuthFile, "w")))
	goto bad;
#else
    if ((addAuthFile = unique_filename (path, "ksm", &fd)) == NULL)
	goto bad;

    if (!(addfp = fdopen(fd, "wb")))
	goto bad;

    if ((remAuthFile = unique_filename (path, "ksm", &fd)) == NULL)
	goto bad;

    if (!(removefp = fdopen(fd, "wb")))
	goto bad;
#endif

    if ((*authDataEntries = (IceAuthDataEntry *) malloc (
			 count * 2 * sizeof (IceAuthDataEntry))) == NULL)
	goto bad;

    for (i = 0; i < numTransports * 2; i += 2) {
	(*authDataEntries)[i].network_id =
	    IceGetListenConnectionString (listenObjs[i/2]);
	(*authDataEntries)[i].protocol_name = (char *) "ICE";
	(*authDataEntries)[i].auth_name = (char *) "MIT-MAGIC-COOKIE-1";

	(*authDataEntries)[i].auth_data =
	    IceGenerateMagicCookie (MAGIC_COOKIE_LEN);
	(*authDataEntries)[i].auth_data_length = MAGIC_COOKIE_LEN;

	(*authDataEntries)[i+1].network_id =
	    IceGetListenConnectionString (listenObjs[i/2]);
	(*authDataEntries)[i+1].protocol_name = (char *) "XSMP";
	(*authDataEntries)[i+1].auth_name = (char *) "MIT-MAGIC-COOKIE-1";

	(*authDataEntries)[i+1].auth_data =
	    IceGenerateMagicCookie (MAGIC_COOKIE_LEN);
	(*authDataEntries)[i+1].auth_data_length = MAGIC_COOKIE_LEN;

	write_iceauth (addfp, removefp, &(*authDataEntries)[i]);
	write_iceauth (addfp, removefp, &(*authDataEntries)[i+1]);

	IceSetPaAuthData (2, &(*authDataEntries)[i]);

	IceSetHostBasedAuthProc (listenObjs[i/2], HostBasedAuthProc);
    }

    fclose (addfp);
    fclose (removefp);

    umask (original_umask);

    sprintf (command, "iceauth source %s", addAuthFile);
    system (command);

    unlink (addAuthFile);

    return (1);

 bad:

    if (addfp)
	fclose (addfp);

    if (removefp)
	fclose (removefp);

    if (addAuthFile) {
	unlink(addAuthFile);
	free(addAuthFile);
    }
    if (remAuthFile) {
	unlink(remAuthFile);
	free(remAuthFile);
    }

    return (0);
}

/*
 * Free up authentication data.
 */
void FreeAuthenticationData(int count, IceAuthDataEntry *authDataEntries)
{
    /* Each transport has entries for ICE and XSMP */

    char command[256];
    int i;

    for (i = 0; i < count * 2; i++) {
	free (authDataEntries[i].network_id);
	free (authDataEntries[i].auth_data);
    }

    free (authDataEntries);

    sprintf (command, "iceauth source %s", remAuthFile);
    system(command);

    unlink(remAuthFile);

    free(addAuthFile);
    free(remAuthFile);
}

static void CloseListeners ()
{
    IceFreeListenObjs (numTransports, listenObjs);

    QCString fName = ::getenv("HOME");
    fName += "/.KSMserver";
    unlink(fName.data());

    FreeAuthenticationData(numTransports, authDataEntries);
}

static void sighandler(int sig)
{
    if (sig == SIGHUP) {
	signal(SIGHUP, sighandler);
	return;
    }

    CloseListeners();
    exit(0);
}


void KSMWatchProc ( IceConn iceConn, IcePointer client_data, Bool opening, IcePointer* watch_data)
{
    KSMServer* ds = ( KSMServer*) client_data;

    if (opening) {
	*watch_data = (IcePointer) ds->watchConnection( iceConn );
    }
    else  {
	ds->removeConnection( (KSMConnection*) *watch_data );
    }
}

static Status KSMNewClientProc ( SmsConn conn, SmPointer manager_data,
				 unsigned long* mask_ret, SmsCallbacks* cb, char** failure_reason_ret)
{
//     qDebug("KSMNewClientProc");

    //     *failure_reason_ret = qstrdup("some failure" );
    *failure_reason_ret = 0;

    void* client =  ((KSMServer*) manager_data )->newClient( conn );

    cb->register_client.callback = KSMRegisterClientProc;
    cb->register_client.manager_data = client;
    cb->interact_request.callback = KSMInteractRequestProc;
    cb->interact_request.manager_data = client;
    cb->interact_done.callback = KSMInteractDoneProc;
    cb->interact_done.manager_data = client;
    cb->save_yourself_request.callback = KSMSaveYourselfRequestProc;
    cb->save_yourself_request.manager_data = client;
    cb->save_yourself_phase2_request.callback = KSMSaveYourselfPhase2RequestProc;
    cb->save_yourself_phase2_request.manager_data = client;
    cb->save_yourself_done.callback = KSMSaveYourselfDoneProc;
    cb->save_yourself_done.manager_data = client;
    cb->close_connection.callback = KSMCloseConnectionProc;
    cb->close_connection.manager_data = client;
    cb->set_properties.callback = KSMSetPropertiesProc;
    cb->set_properties.manager_data = client;
    cb->delete_properties.callback = KSMDeletePropertiesProc;
    cb->delete_properties.manager_data = client;
    cb->get_properties.callback = KSMGetPropertiesProc;
    cb->get_properties.manager_data = client;

    *mask_ret = SmsRegisterClientProcMask |
		SmsInteractRequestProcMask |
		SmsInteractDoneProcMask |
		SmsSaveYourselfRequestProcMask |
		SmsSaveYourselfP2RequestProcMask |
		SmsSaveYourselfDoneProcMask |
		SmsCloseConnectionProcMask |
		SmsSetPropertiesProcMask |
		SmsDeletePropertiesProcMask |
		SmsGetPropertiesProcMask;
    return 1;
};


KSMServer::KSMServer( const QString& windowManager )
{
    the_server = this;
    wm = windowManager;

    state = Idle;
    KConfig* config = KGlobal::config();
    config->setGroup("General" );
    saveSession = config->readBoolEntry( "saveSession", FALSE );
    clientInteracting = 0;

    char 	errormsg[256];
    if (!SmsInitialize ( (char*) KSMVendorString, (char*) KSMReleaseString,
			 KSMNewClientProc,
			 (SmPointer) this,
			 HostBasedAuthProc, 256, errormsg ) ) {

	qWarning("KSMServer: could not register XSM protocol");
    }

    if (!IceListenForConnections (&numTransports, &listenObjs,
				  256, errormsg))
	{
	    fprintf (stderr, "%s\n", errormsg);
	    exit (1);
	} else {
	    // publish available transports.
	    QCString fName = ::getenv("HOME");
	    fName += "/.KSMserver";
	    FILE *f;
	    f = ::fopen(fName.data(), "w+");
	    char* session_manager = IceComposeNetworkIdList(numTransports, listenObjs);
	    fprintf(f, session_manager);
	    fprintf(f, "\n%i\n", getpid());
	    fclose(f);
	    setenv( "SESSION_MANAGER", session_manager, TRUE  );
	}

    if (!SetAuthentication(numTransports, listenObjs, &authDataEntries)) {
	qFatal("ksmserver could not set authorization");
    }

  IceAddConnectionWatch (KSMWatchProc, (IcePointer) this);

    listener.setAutoDelete( TRUE );
    KSMListener* con;
    for ( int i = 0; i < numTransports; i++) {
	con = new KSMListener( listenObjs[i] );
	listener.append( con );
	connect( con, SIGNAL( activated(int) ), this, SLOT( newConnection(int) ) );
    }

    signal(SIGHUP, sighandler);
    signal(SIGTERM, sighandler);
    signal(SIGINT, sighandler);
    signal(SIGPIPE, SIG_IGN);
}

KSMServer::~KSMServer()
{
    CloseListeners();
}

void* KSMServer::watchConnection( IceConn iceConn )
{
    KSMConnection* conn = new KSMConnection( iceConn );
    connect( conn, SIGNAL( activated(int) ), this, SLOT( processData(int) ) );
    return (void*) conn;
}

void KSMServer::removeConnection( KSMConnection* conn )
{
    delete conn;
}


/*! 
  Called from our IceIoErrorHandler
 */
void KSMServer::ioError( IceConn iceConn )
{
    QListIterator<KSMClient> it ( clients );
    while ( it.current() &&SmsGetIceConnection( it.current()->connection() ) != iceConn )
	++it;

    if ( it.current() ) {
	SmsConn smsConn = it.current()->connection();
	deleteClient( it.current() );
	SmsCleanUp( smsConn );
    }
    IceSetShutdownNegotiation (iceConn, False);
    IceCloseConnection( iceConn );
}

void KSMServer::processData( int /*socket*/ )
{
    (void) IceProcessMessages( ((KSMConnection*)sender())->iceConn, 0, 0 );
}


KSMClient* KSMServer::newClient( SmsConn conn )
{
    KSMClient* client = new KSMClient( conn );
    clients.append( client );
    return client;
}

void KSMServer::deleteClient( KSMClient* client )
{
    if ( clients.findRef( client ) == -1 ) // paranoia
	return;
    clients.removeRef( client );
    if ( client == clientInteracting ) {
	clientInteracting = 0;
	handlePendingInteractions();
    }
    delete client;
    if ( state == Shutdown )
	completeShutdown();
    if ( state == Killing )
	completeKilling();
}

void KSMServer::newConnection( int /*socket*/ )
{
    IceAcceptStatus status;
    IceConn iceConn = IceAcceptConnection( ((KSMListener*)sender())->listenObj, &status);
    IceSetShutdownNegotiation( iceConn, False );
}


void KSMServer::shutdown()
{
    if ( state != Idle )
	return;
    if ( KSMShutdown::shutdown( saveSession ) ) {
	KConfig* config = KGlobal::config();
	config->setGroup("General" );
	config->writeEntry( "saveSession", saveSession?"true":"false");
	if ( saveSession )
	    discardSession();
	state = Shutdown;
	for ( KSMClient* c = clients.first(); c; c = clients.next() ) {
	    c->resetState();
	    SmsSaveYourself( c->connection(), saveSession?SmSaveBoth: SmSaveGlobal,
			     TRUE, SmInteractStyleAny, FALSE );
	}
	if ( clients.isEmpty() )
	    completeShutdown();
    }
}


// callbacks
void KSMServer::saveYourselfDone( KSMClient* client, bool success )
{
    if ( state == Idle )
	return;
    if ( success ) {
	client->saveYourselfDone = TRUE;

	// workaround for broken qt-2.1beta3: make the window manager
	// pseudo phase2. #### remove this with qt-2.1 final
	if ( !client->waitForPhase2 && !client->phase2Workaround &&
	     !wm.isEmpty() && client->program() == wm ) {
	    client->waitForPhase2 = TRUE;
	    client->phase2Workaround = TRUE;
	    client->saveYourselfDone = FALSE;
	    SmsShutdownCancelled( client->connection() );
	}

	completeShutdown();
    } else {
	cancelShutdown();
    }

}

void KSMServer::interactRequest( KSMClient* client, int /*dialogType*/ )
{
    if ( state == Shutdown )
	client->pendingInteraction = TRUE;
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
	cancelShutdown();
    else
	handlePendingInteractions();
}


void KSMServer::phase2Request( KSMClient* client )
{
    client->waitForPhase2 = TRUE;
    completeShutdown();
}

void KSMServer::handlePendingInteractions()
{
    if ( clientInteracting )
	return;

    for ( KSMClient* c = clients.first(); c; c = clients.next() ) {
	if ( c->pendingInteraction ) {
	    clientInteracting = c;
	    c->pendingInteraction = FALSE;
	    break;
	}
    }
    if ( clientInteracting )
	SmsInteract( clientInteracting->connection() );
}


void KSMServer::cancelShutdown()
{
    clientInteracting = 0;
    for ( KSMClient* c = clients.first(); c; c = clients.next() ) {
	// workaround for broken qt-2.1beta3: make the window
	// manager pseudo phase2. #### remove this with qt-2.1
	// final
	if ( c->phase2Workaround && c->waitForPhase2)
	    continue;

 	SmsShutdownCancelled( c->connection() );
    }
    state = Idle;
}

void KSMServer::completeShutdown()
{
    if ( state != Shutdown )
	return;

    for ( KSMClient* c = clients.first(); c; c = clients.next() ) {
	if ( !c->saveYourselfDone && !c->waitForPhase2 )
	    return; // not done yet
    }

    // do phase 2
    bool waitForPhase2 = FALSE;
    for ( KSMClient* c = clients.first(); c; c = clients.next() ) {
	if ( !c->saveYourselfDone && c->waitForPhase2 ) {
	    c->waitForPhase2 = FALSE;
	    if ( c->phase2Workaround ) {
		// workaround for broken qt-2.1beta3: make the window
		// manager pseudo phase2. #### remove this with qt-2.1
		// final
		SmsSaveYourself( c->connection(), saveSession?SmSaveBoth: SmSaveGlobal,
				 TRUE, SmInteractStyleAny, FALSE );
	    } else {
		SmsSaveYourselfPhase2( c->connection() );
	    }
	    waitForPhase2 = TRUE;
	}
    }
    if ( waitForPhase2 )
	return;

    if ( saveSession )
	storeSesssion();

    // kill all clients
    state = Killing;
    for ( KSMClient* c = clients.first(); c; c = clients.next() ) {
	// do not kill the wm yet, we do that in completeKilling()
	// below.
	if ( !wm.isEmpty() && c->program() == wm )
	    continue;
	SmsDie( c->connection() );
    }
    if ( clients.isEmpty() )
	completeKilling();
    else
	QTimer::singleShot( 4000, this, SLOT( timeoutQuit() ) );
}

void KSMServer::completeKilling()
{
    if ( state != Killing )
	return;
    if ( !wm.isEmpty() && clients.count() == 1 && clients.first()->program() == wm ) {
	// the wm was not killed yet, do it
	SmsDie( clients.first()->connection() );
	return;
    }

    if ( clients.isEmpty() )
	qApp->quit();
}

void KSMServer::timeoutQuit()
{
    qApp->quit();
}

void KSMServer::discardSession()
{
    KConfig* config = KGlobal::config();
    config->setGroup("Session" );
    int count =  config->readNumEntry( "count" );
    for ( int i = 1; i <= count; i++ ) {
	QString n = QString::number(i);
	executeCommand( config->readListEntry( QString("discardCommand")+n ) );
    }
    config->writeEntry( "count", 0 );
}

void KSMServer::storeSesssion()
{
    KConfig* config = KGlobal::config();
    config->setGroup("Session" );
    int count =  0;
    for ( KSMClient* c = clients.first(); c; c = clients.next() ) {
	count++;
	QString n = QString::number(count);
	config->writeEntry( QString("program")+n, c->program() );
	config->writeEntry( QString("restartCommand")+n, c->restartCommand() );
	config->writeEntry( QString("discardCommand")+n, c->discardCommand() );
	config->writeEntry( QString("restartStyleHint")+n, c->restartStyleHint() );
	config->writeEntry( QString("userId")+n, c->userId() );
    }
    config->writeEntry( "count", count );
    config->sync();
}


/*!  Restores the previous session. Ensures the window manager is
  running (if specified).
 */
void KSMServer::restoreSession()
{
    KConfig* config = KGlobal::config();
    config->setGroup("Session" );
    int count =  config->readNumEntry( "count" );

    if ( !wm.isEmpty() ) {
	// when we have a window manager, we start it first and give
	// it some time before launching other processes. Results in a
	// visually more appealing startup.
	QStringList wmCommand = wm;
	for ( int i = 1; i <= count; i++ ) {
	    QString n = QString::number(i);
	    if ( wm == config->readEntry( QString("program")+n ) ) {
		wmCommand = config->readListEntry( QString("restartCommand")+n );
		break;
	    }
	}
	executeCommand( wmCommand );
	QTimer::singleShot( 2000, this, SLOT( restoreSessionInternal() ) );
	return;
    }

    restoreSessionInternal();
}

/*!
  Starts the default session.

  Currently, that's the window manager only (if specified).
 */
void KSMServer::startDefaultSession()
{
    executeCommand( wm );
}


void KSMServer::restoreSessionInternal()
{
    KConfig* config = KGlobal::config();
    config->setGroup("Session" );
    int count =  config->readNumEntry( "count" );
    for ( int i = 1; i <= count; i++ ) {
	QString n = QString::number(i);
	if ( wm != config->readEntry( QString("program")+n ) ) {
	    executeCommand( config->readListEntry( QString("restartCommand")+n ) );
	}
    }
}

