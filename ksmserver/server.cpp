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

#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <string.h>

#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif

#include <qfile.h>
#include <qtextstream.h>
#include <qdatastream.h>
#include <qptrstack.h>
#include <qpushbutton.h>
#include <qmessagebox.h>
#include <qguardedptr.h>
#include <qtimer.h>

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

#include "server.h"
#include "global.h"
#include "shutdown.h"

#include "server.moc"

#include <kdebug.h>

const int XNone = None;
#undef None
#include <knotifyclient.h>
#include <qregexp.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

extern "C" {
    /* int umask(...); */

static void the_reaper(int /*sig*/)
{
    int status;
#ifdef HAVE_WAITPID
    while(waitpid(-1, &status, WNOHANG) > 0) ;
#else
    wait(&status);
#endif
    signal(SIGCHLD, the_reaper);
}

}
KSMServer* the_server = 0;

KSMClient::KSMClient( SmsConn conn)
{
    smsConn = conn;
    id = 0;
    resetState();
}

KSMClient::~KSMClient()
{
    for ( SmProp* prop = properties.first(); prop; prop = properties.next() )
        SmFreeProperty( prop );
    if (id) free((void*)id);
}

SmProp* KSMClient::property( const char* name ) const
{
    for ( QPtrListIterator<SmProp> it( properties ); it.current(); ++it ) {
        if ( !qstrcmp( it.current()->name, name ) )
            return it.current();
    }
    return 0;
}

void KSMClient::resetState()
{
    saveYourselfDone = false;
    pendingInteraction = false;
    waitForPhase2 = false;
    wasPhase2 = false;
}

/*
 * This fakes SmsGenerateClientID() in case we can't read our own hostname.
 * In this case SmsGenerateClientID() returns NULL, but we really want a
 * client ID, so we fake one.
 */
static KStaticDeleter<QString> smy_addr;
char * safeSmsGenerateClientID( SmsConn c )
{
    char *ret = SmsGenerateClientID(c);
    if (!ret) {
        static QString *my_addr = 0;
       if (!my_addr) {
           qWarning("Can't get own host name. Your system is severely misconfigured\n");
           my_addr = smy_addr.setObject(new QString);

           /* Faking our IP address, the 0 below is "unknown" address format
              (1 would be IP, 2 would be DEC-NET format) */
           my_addr->sprintf("0%.8x", KApplication::random());
       }
       /* Needs to be malloc(), to look the same as libSM */
       ret = (char *)malloc(1+9+13+10+4+1 + /*safeness*/ 10);
       static int sequence = 0;

       if (ret == NULL)
           return NULL;

       sprintf(ret, "1%s%.13ld%.10d%.4d", my_addr->latin1(), (long)time(NULL),
           getpid(), sequence);
       sequence = (sequence + 1) % 10000;
    }
    return ret;
}

void KSMClient::registerClient( const char* previousId )
{
    id = previousId;
    if ( !id )
        id = safeSmsGenerateClientID( smsConn );
    SmsRegisterClientReply( smsConn, (char*) id );
    SmsSaveYourself( smsConn, SmSaveLocal, false, SmInteractStyleNone, false );
    SmsSaveComplete( smsConn );
    the_server->clientRegistered( previousId  );
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
 * to restart applications.
 */
void KSMServer::startApplication( const QStringList& command )
{
    if ( command.isEmpty() )
        return;
    int n = command.count();
    QCString app = command[0].latin1();
    QValueList<QCString> argList;
    for ( int i=1; i < n; i++)
       argList.append( QCString(command[i].latin1()));
    DCOPRef( launcher ).send( "exec_blind", app, DCOPArg( argList, "QValueList<QCString>" ) );
}

/*! Utility function to execute a command on the local machine. Used
 * to discard session data
 */
void KSMServer::executeCommand( const QStringList& command )
{
    if ( command.isEmpty() )
        return;
    int n = command.count();
    QCString app = command[0].latin1();
    char ** argList = new char *[n+1];

    for ( int i=0; i < n; i++)
       argList[i] = (char *) command[i].latin1();
    argList[n] = 0;

    int pid = fork();
    if (pid == -1)
       return;
    if (pid == 0) {
       execvp(app.data(), argList);
       exit(127);
    }
    int status;
    waitpid(pid, &status, 0);
    delete [] argList;
}

IceAuthDataEntry *authDataEntries = 0;
static KTempFile *remAuthFile = 0;

static IceListenObj *listenObjs = 0;
int numTransports = 0;
static bool only_local = 0;

static Bool HostBasedAuthProc ( char* /*hostname*/)
{
    if (only_local)
        return true;
    else
        return false;
}


Status KSMRegisterClientProc (
    SmsConn             /* smsConn */,
    SmPointer           managerData,
    char *              previousId
)
{
    KSMClient* client = (KSMClient*) managerData;
    client->registerClient( previousId );
    return 1;
}

void KSMInteractRequestProc (
    SmsConn             /* smsConn */,
    SmPointer           managerData,
    int                 dialogType
)
{
    the_server->interactRequest( (KSMClient*) managerData, dialogType );
}

void KSMInteractDoneProc (
    SmsConn             /* smsConn */,
    SmPointer           managerData,
    Bool                        cancelShutdown
)
{
    the_server->interactDone( (KSMClient*) managerData, cancelShutdown );
}

void KSMSaveYourselfRequestProc (
    SmsConn             smsConn ,
    SmPointer           /* managerData */,
    int                 saveType,
    Bool                shutdown,
    int                 interactStyle,
    Bool                fast,
    Bool                global
)
{
    if ( shutdown ) {
        the_server->shutdown( fast ?
                              KApplication::ShutdownConfirmNo :
                              KApplication::ShutdownConfirmDefault,
                              KApplication::ShutdownTypeDefault,
                              KApplication::ShutdownModeDefault );
    } else if ( !global ) {
        SmsSaveYourself( smsConn, saveType, false, interactStyle, fast );
        SmsSaveComplete( smsConn );
    }
    // else checkpoint only, ksmserver does not yet support this
    // mode. Will come for KDE 3.1
}

void KSMSaveYourselfPhase2RequestProc (
    SmsConn             /* smsConn */,
    SmPointer           managerData
)
{
    the_server->phase2Request( (KSMClient*) managerData );
}

void KSMSaveYourselfDoneProc (
    SmsConn             /* smsConn */,
    SmPointer           managerData,
    Bool                success
)
{
    the_server->saveYourselfDone( (KSMClient*) managerData, success );
}

void KSMCloseConnectionProc (
    SmsConn             smsConn,
    SmPointer           managerData,
    int                 count,
    char **             reasonMsgs
)
{
    the_server->deleteClient( ( KSMClient* ) managerData );
    if ( count )
        SmFreeReasons( count, reasonMsgs );
    IceConn iceConn = SmsGetIceConnection( smsConn );
    SmsCleanUp( smsConn );
    IceSetShutdownNegotiation (iceConn, False);
    IceCloseConnection( iceConn );
}

void KSMSetPropertiesProc (
    SmsConn             /* smsConn */,
    SmPointer           managerData,
    int                 numProps,
    SmProp **           props
)
{
    KSMClient* client = ( KSMClient* ) managerData;
    for ( int i = 0; i < numProps; i++ ) {
        SmProp *p = client->property( props[i]->name );
        if ( p ) {
            client->properties.removeRef( p );
            SmFreeProperty( p );
        }
        client->properties.append( props[i] );
        if ( !qstrcmp( props[i]->name, SmProgram ) )
            the_server->clientSetProgram( client );
    }

    if ( numProps )
        free( props );

}

void KSMDeletePropertiesProc (
    SmsConn             /* smsConn */,
    SmPointer           managerData,
    int                 numProps,
    char **             propNames
)
{
    KSMClient* client = ( KSMClient* ) managerData;
    for ( int i = 0; i < numProps; i++ ) {
        SmProp *p = client->property( propNames[i] );
        if ( p ) {
            client->properties.removeRef( p );
            SmFreeProperty( p );
        }
    }
}

void KSMGetPropertiesProc (
    SmsConn             smsConn,
    SmPointer           managerData
)
{
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
    static const char hexchars[] = "0123456789abcdef";

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


#define MAGIC_COOKIE_LEN 16

Status SetAuthentication_local (int count, IceListenObj *listenObjs)
{
    int i;
    for (i = 0; i < count; i ++) {
        char *prot = IceGetListenConnectionString(listenObjs[i]);
        if (!prot) continue;
        char *host = strchr(prot, '/');
        char *sock = 0;
        if (host) {
            *host=0;
            host++;
            sock = strchr(host, ':');
            if (sock) {
                *sock = 0;
                sock++;
            }
        }
        kdDebug() << "KSMServer: SetAProc_loc: conn " << (unsigned)i << ", prot=" << prot << ", file=" << sock << endl;
        if (sock && !strcmp(prot, "local")) {
            chmod(sock, 0700);
        }
        IceSetHostBasedAuthProc (listenObjs[i], HostBasedAuthProc);
        free(prot);
    }
    return 1;
}

Status SetAuthentication (int count, IceListenObj *listenObjs,
                          IceAuthDataEntry **authDataEntries)
{
    KTempFile addAuthFile;
    addAuthFile.setAutoDelete(true);
    
    remAuthFile = new KTempFile;
    remAuthFile->setAutoDelete(true);    

    if ((addAuthFile.status() != 0) || (remAuthFile->status() != 0))
        return 0;

    if ((*authDataEntries = (IceAuthDataEntry *) malloc (
                         count * 2 * sizeof (IceAuthDataEntry))) == NULL)
        return 0;

    for (int i = 0; i < numTransports * 2; i += 2) {
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

        write_iceauth (addAuthFile.fstream(), remAuthFile->fstream(), &(*authDataEntries)[i]);
        write_iceauth (addAuthFile.fstream(), remAuthFile->fstream(), &(*authDataEntries)[i+1]);

        IceSetPaAuthData (2, &(*authDataEntries)[i]);

        IceSetHostBasedAuthProc (listenObjs[i/2], HostBasedAuthProc);
    }
    addAuthFile.close();
    remAuthFile->close();

    QString iceAuth = KGlobal::dirs()->findExe("iceauth");
    if (iceAuth.isEmpty())
    {
        qWarning("KSMServer: could not find iceauth");
        return 0;
    }

    KProcess p;
    p << iceAuth << "source" << addAuthFile.name();
    p.start(KProcess::Block);

    return (1);
}

/*
 * Free up authentication data.
 */
void FreeAuthenticationData(int count, IceAuthDataEntry *authDataEntries)
{
    /* Each transport has entries for ICE and XSMP */
    if (only_local)
        return;

    for (int i = 0; i < count * 2; i++) {
        free (authDataEntries[i].network_id);
        free (authDataEntries[i].auth_data);
    }

    free (authDataEntries);

    QString iceAuth = KGlobal::dirs()->findExe("iceauth");
    if (iceAuth.isEmpty())
    {
        qWarning("KSMServer: could not find iceauth");
        return;
    }
    
    KProcess p;
    p << iceAuth << "source" << remAuthFile->name();
    p.start(KProcess::Block);

    delete remAuthFile;
    remAuthFile = 0;
}

static int Xio_ErrorHandler( Display * )
{
    qWarning("ksmserver: Fatal IO error: client killed");

    // Don't do anything that might require the X connection
    if (the_server)
    {
       KSMServer *server = the_server;
       the_server = 0;
       server->cleanUp();
       // Don't delete server!!
    }
    
    exit(0); // Don't report error, it's not our fault.
}

static void sighandler(int sig)
{
    if (sig == SIGHUP) {
        signal(SIGHUP, sighandler);
        return;
    }

    if (the_server)
    {
       KSMServer *server = the_server;
       the_server = 0;
       server->cleanUp();
       delete server;
    }

    if (kapp)
        kapp->quit();
    //::exit(0);
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


#ifdef HAVE__ICETRANSNOLISTEN
extern "C" int _IceTransNoListen(const char * protocol);
#endif

KSMServer::KSMServer( const QString& windowManager, bool _only_local )
  : DCOPObject("ksmserver"), sessionGroup( "" )
{
    the_server = this;
    clean = false;
    wm = windowManager;

    state = Idle;
    dialogActive = false;
    saveSession = false;
    wmPhase1WaitingCount = 0;
    KConfig* config = KGlobal::config();
    config->setGroup("General" );
    clientInteracting = 0;

    only_local = _only_local;
#ifdef HAVE__ICETRANSNOLISTEN
    if (only_local)
        _IceTransNoListen("tcp");
#else
    only_local = false;
#endif

    launcher = KApplication::launcher();

    char        errormsg[256];
    if (!SmsInitialize ( (char*) KSMVendorString, (char*) KSMReleaseString,
                         KSMNewClientProc,
                         (SmPointer) this,
                         HostBasedAuthProc, 256, errormsg ) ) {

        qWarning("KSMServer: could not register XSM protocol");
    }

    if (!IceListenForConnections (&numTransports, &listenObjs,
                                  256, errormsg))
    {
        qWarning("KSMServer: Error listening for connections: %s", errormsg);
        qWarning("KSMServer: Aborting.");
        exit(1);
    }

    {
        // publish available transports.
        QCString fName = QFile::encodeName(locateLocal("socket", "KSMserver"));
        QCString display = ::getenv("DISPLAY");
        // strip the screen number from the display
        display.replace(QRegExp("\\.[0-9]+$"), "");
        int i;
        while( (i = display.find(':')) >= 0)
           display[i] = '_';

        fName += "_"+display;
        FILE *f;
        f = ::fopen(fName.data(), "w+");
        if (!f)
        {
            qWarning("KSMServer: can't open %s: %s", fName.data(), strerror(errno));
            qWarning("KSMServer: Aborting.");
            exit(1);
        }
        char* session_manager = IceComposeNetworkIdList(numTransports, listenObjs);
        fprintf(f, "%s\n%i\n", session_manager, getpid());
        fclose(f);
        setenv( "SESSION_MANAGER", session_manager, true  );
       // Pass env. var to kdeinit.
       DCOPRef( launcher ).send( "setLaunchEnv", "SESSION_MANAGER", (const char*) session_manager );
    }

    if (only_local) {
        if (!SetAuthentication_local(numTransports, listenObjs))
            qFatal("KSMSERVER: authentication setup failed.");
    } else {
        if (!SetAuthentication(numTransports, listenObjs, &authDataEntries))
            qFatal("KSMSERVER: authentication setup failed.");
    }

    IceAddConnectionWatch (KSMWatchProc, (IcePointer) this);

    listener.setAutoDelete( true );
    KSMListener* con;
    for ( int i = 0; i < numTransports; i++) {
        con = new KSMListener( listenObjs[i] );
        listener.append( con );
        connect( con, SIGNAL( activated(int) ), this, SLOT( newConnection(int) ) );
    }

    signal(SIGHUP, sighandler);
    signal(SIGTERM, sighandler);
    signal(SIGINT, sighandler);
    signal(SIGCHLD, the_reaper);
    signal(SIGPIPE, SIG_IGN);

    connect( &protectionTimer, SIGNAL( timeout() ), this, SLOT( protectionTimeout() ) );
    connect( &restoreTimer, SIGNAL( timeout() ), this, SLOT( restoreNextInternal() ) );
    connect( kapp, SIGNAL( shutDown() ), this, SLOT( cleanUp() ) );

    KNotifyClient::event( "startkde" ); // this is the time KDE is up
}

KSMServer::~KSMServer()
{
    the_server = 0;
    cleanUp();
}

void KSMServer::cleanUp()
{
    if (clean) return;
    clean = true;
    IceFreeListenObjs (numTransports, listenObjs);

    QCString fName = QFile::encodeName(locateLocal("socket", "KSMserver"));
    QCString display = ::getenv("DISPLAY");
    // strip the screen number from the display
    display.replace(QRegExp("\\.[0-9]+$"), "");
    int i;
    while( (i = display.find(':')) >= 0)
         display[i] = '_';

    fName += "_"+display;
    ::unlink(fName.data());

    FreeAuthenticationData(numTransports, authDataEntries);
    signal(SIGTERM, SIG_DFL);
    signal(SIGINT, SIG_DFL);
    signal(SIGCHLD, SIG_DFL);
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
void KSMServer::ioError( IceConn /*iceConn*/  )
{
}

void KSMServer::processData( int /*socket*/ )
{
    IceConn iceConn = ((KSMConnection*)sender())->iceConn;
    IceProcessMessagesStatus status = IceProcessMessages( iceConn, 0, 0 );
    if ( status == IceProcessMessagesIOError ) {
        IceSetShutdownNegotiation( iceConn, False );
        QPtrListIterator<KSMClient> it ( clients );
        while ( it.current() &&SmsGetIceConnection( it.current()->connection() ) != iceConn )
            ++it;
        if ( it.current() ) {
            SmsConn smsConn = it.current()->connection();
            deleteClient( it.current() );
            SmsCleanUp( smsConn );
        }
        (void) IceCloseConnection( iceConn );
    }
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
    if ( state == Shutdown || state == Checkpoint )
        completeShutdownOrCheckpoint();
    if ( state == Killing )
        completeKilling();
}

void KSMServer::newConnection( int /*socket*/ )
{
    IceAcceptStatus status;
    IceConn iceConn = IceAcceptConnection( ((KSMListener*)sender())->listenObj, &status);
    IceSetShutdownNegotiation( iceConn, False );
    IceConnectStatus cstatus;
    while ((cstatus = IceConnectionStatus (iceConn))==IceConnectPending) {
        (void) IceProcessMessages( iceConn, 0, 0 );
    }

    if (cstatus != IceConnectAccepted) {
        if (cstatus == IceConnectIOError)
            kdDebug() << "IO error opening ICE Connection!" << endl;
        else
            kdDebug() << "ICE Connection rejected!" << endl;
        (void )IceCloseConnection (iceConn);
    }
}


void KSMServer::shutdown( KApplication::ShutdownConfirm confirm,
    KApplication::ShutdownType sdtype, KApplication::ShutdownMode sdmode )
{
    if ( state != Idle || dialogActive )
        return;
    dialogActive = true;

    bool maysd, maynuke;
    KApplication::ShutdownMode defsdmode;
    QString fifoname;
    QStringList dmopt =
        QStringList::split( QChar( ',' ),
                            QString::fromLatin1( ::getenv( "XDM_MANAGED" ) ) );
    if ( dmopt.isEmpty() || dmopt.first()[0] != QChar( '/' ) ) {
        fifoname = QString::null;
        maysd = maynuke = false;
        defsdmode = KApplication::ShutdownModeSchedule;
    } else {
        fifoname = dmopt.first();
        maysd = dmopt.contains( QString::fromLatin1( "maysd" ) ) != 0;
        maynuke = dmopt.contains( QString::fromLatin1( "mayfn" ) ) != 0;
        if ( dmopt.contains( QString::fromLatin1( "fn" ) ) != 0 )
            defsdmode = KApplication::ShutdownModeForceNow;
        else if ( dmopt.contains( QString::fromLatin1( "tn" ) ) != 0 )
            defsdmode = KApplication::ShutdownModeTryNow;
        else
            defsdmode = KApplication::ShutdownModeSchedule;
    }

    KConfig *config = KGlobal::config();
    config->reparseConfiguration(); // config may have changed in the KControl module

    config->setGroup("General" );
    excludeApps = QStringList::split( ':', config->readEntry( "excludeApps" ).lower()); 
    bool logoutConfirmed =
        (confirm == KApplication::ShutdownConfirmYes) ? false :
       (confirm == KApplication::ShutdownConfirmNo) ? true :
                  !config->readBoolEntry( "confirmLogout", true );
    KApplication::ShutdownType old_sdtype = (KApplication::ShutdownType)
                                            config->readNumEntry( "shutdownType",
                                                               (int)KApplication::ShutdownTypeNone );
    if (sdtype == KApplication::ShutdownTypeDefault)
        sdtype = old_sdtype;
    KApplication::ShutdownMode old_sdmode = (KApplication::ShutdownMode)
                                            config->readNumEntry( "shutdownMode", (int)defsdmode );
    if (sdmode == KApplication::ShutdownModeDefault)
        sdmode = old_sdmode;

    if (!maysd)
        sdtype = KApplication::ShutdownTypeNone;
    if (!maynuke && sdmode == KApplication::ShutdownModeForceNow)
        sdmode = KApplication::ShutdownModeSchedule;

    if ( !logoutConfirmed ) {
        KSMShutdownFeedback::start(); // make the screen gray
        logoutConfirmed =
            KSMShutdownDlg::confirmShutdown( maysd, maynuke, sdtype, sdmode );
        // ###### We can't make the screen remain gray while talking to the apps,
        // because this prevents interaction ("do you want to save", etc.)
        // TODO: turn the feedback widget into a list of apps to be closed,
        // with an indicator of the current status for each.
        KSMShutdownFeedback::stop(); // make the screen become normal again
    }

    if ( logoutConfirmed ) {

        // shall we save the session on logout?
        saveSession = ( config->readEntry( "loginMode", "restorePreviousLogout" ) == "restorePreviousLogout" );

        if ( saveSession )
            sessionGroup = QString("Session: ") + SESSION_PREVIOUS_LOGOUT;

        // Set the real desktop background to black so that exit looks
        // clean regardless of what was on "our" desktop.
        kapp->desktop()->setBackgroundColor( Qt::black );
        KNotifyClient::event( "exitkde" ); // KDE says good bye
        if (sdtype != old_sdtype || sdmode != old_sdmode) {
            KConfig* config = KGlobal::config();
            config->setGroup("General" );
            config->writeEntry( "shutdownType", (int)sdtype);
            config->writeEntry( "shutdownMode", (int)sdmode);
        }
        state = Shutdown;
        wmPhase1WaitingCount = 0;
        saveType = saveSession?SmSaveBoth:SmSaveGlobal;
#ifndef NO_LEGACY_SESSION_MANAGEMENT
	performLegacySessionSave();
#endif
        startProtection();
        for ( KSMClient* c = clients.first(); c; c = clients.next() ) {
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
            for ( KSMClient* c = clients.first(); c; c = clients.next() )
                SmsSaveYourself( c->connection(), saveType,
                             true, SmInteractStyleAny, false );
        }
        if ( clients.isEmpty() )
            completeShutdownOrCheckpoint();
        if ( sdtype != KApplication::ShutdownTypeNone ) {
            QFile fifo( fifoname );
            if ( fifo.open( IO_WriteOnly | IO_Raw ) ) {
                QCString cmd( "shutdown\t" );
                cmd.append( sdtype == KApplication::ShutdownTypeReboot ?
                            "reboot\t" : "halt\t" );
                cmd.append( sdmode == KApplication::ShutdownModeForceNow ?
                            "forcenow\n" :
                            sdmode == KApplication::ShutdownModeTryNow ?
                            "trynow\n" : "schedule\n" );
                fifo.writeBlock( cmd.data(), cmd.length() );
                fifo.close();
            }
        }
    }
    dialogActive = false;
}

QString KSMServer::currentSession()
{
    if ( sessionGroup.startsWith( "Session: " ) )
        return sessionGroup.mid( 9 );
    return ""; // empty, not null, since used for KConfig::setGroup
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
    for ( KSMClient* c = clients.first(); c; c = clients.next() ) {
        c->resetState();
        if( isWM( c )) {
            ++wmPhase1WaitingCount;
            SmsSaveYourself( c->connection(), saveType, false, SmInteractStyleNone, false );
        }
    }
    if( wmPhase1WaitingCount == 0 ) {
        for ( KSMClient* c = clients.first(); c; c = clients.next() )
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
    if ( state == Idle )
        return;
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
            for ( KSMClient* c = clients.first(); c; c = clients.next() )
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
            for ( KSMClient* c = clients.first(); c; c = clients.next() )
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

    for ( KSMClient* c = clients.first(); c; c = clients.next() ) {
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
    kdDebug() << "cancelShutdown: client " << c->program() << "(" << c->clientId() << ")" << endl;
    clientInteracting = 0;
    for ( KSMClient* c = clients.first(); c; c = clients.next() )
        SmsShutdownCancelled( c->connection() );
    state = Idle;
}

/*
   Internal protection slot, invoked when clients do not react during
  shutdown.
 */
void KSMServer::protectionTimeout()
{
    if ( ( state != Shutdown && state != Checkpoint ) || clientInteracting )
        return;

    for ( KSMClient* c = clients.first(); c; c = clients.next() ) {
        if ( !c->saveYourselfDone && !c->waitForPhase2 ) {
            kdDebug() << "protectionTimeout: client " << c->program() << "(" << c->clientId() << ")" << endl;
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

    for ( KSMClient* c = clients.first(); c; c = clients.next() ) {
        if ( !c->saveYourselfDone && !c->waitForPhase2 )
            return; // not done yet
    }

    // do phase 2
    bool waitForPhase2 = false;
    for ( KSMClient* c = clients.first(); c; c = clients.next() ) {
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
        // kill all clients
        state = Killing;
        for ( KSMClient* c = clients.first(); c; c = clients.next() ) {
            kdDebug() << "completeShutdown: client " << c->program() << "(" << c->clientId() << ")" << endl;
            if (c->wasPhase2)
                continue;
            SmsDie( c->connection() );
        }

        kdDebug() << " We killed all clients. We have now clients.count()=" <<
           clients.count() << endl;
        completeKilling();
        QTimer::singleShot( 4000, kapp, SLOT( quit() ) );
    } else if ( state == Checkpoint ) {
        state = Idle;
    }
}

void KSMServer::completeKilling()
{
    kdDebug() << "KSMServer::completeKilling clients.count()=" <<
        clients.count() << endl;
    if ( state != Killing ) {
        //      kdWarning() << "Not Killing !!! state=" << state << endl;
        return;
    }

    if ( clients.isEmpty() ) {
        kapp->quit();
    } else {
        for (KSMClient *c = clients.first(); c; c = clients.next()) {
            if (! c->wasPhase2)
                return;
        }
        // the wm was not killed yet, do it
        for (KSMClient *c = clients.first(); c; c = clients.next()) {
            SmsDie( c->connection() );
        }
    }
}

void KSMServer::discardSession()
{
    KConfig* config = KGlobal::config();
    config->setGroup( sessionGroup );
    int count =  config->readNumEntry( "count", 0 );
    for ( KSMClient* c = clients.first(); c; c = clients.next() ) {
        QStringList discardCommand = c->discardCommand();
        if ( discardCommand.isEmpty())
            continue;
        // check that non of the old clients used the exactly same
        // discardCommand before we execute it. This used to be the
        // case up to KDE and Qt < 3.1
        int i = 1;
        while ( i <= count &&
                config->readPathListEntry( QString("discardCommand") + QString::number(i) ) != discardCommand )
            i++;
        if ( i <= count )
            executeCommand( discardCommand );
    }
}

void KSMServer::storeSession()
{
    KConfig* config = KGlobal::config();
    config->setGroup( sessionGroup );
    int count =  config->readNumEntry( "count" );
    for ( int i = 1; i <= count; i++ ) {
        QStringList discardCommand = config->readPathListEntry( QString("discardCommand") + QString::number(i) );
        if ( discardCommand.isEmpty())
            continue;
        // check that non of the new clients uses the exactly same
        // discardCommand before we execute it. This used to be the
        // case up to KDE and Qt < 3.1
        KSMClient* c = clients.first();
        while ( c && discardCommand != c->discardCommand() )
            c = clients.next();
        if ( c )
            continue;
        executeCommand( discardCommand );
    }
    config->deleteGroup( sessionGroup ); //### does not work with global config object...
    config->setGroup( sessionGroup );
    count =  0;

    if ( !wm.isEmpty() ) {
        // put the wm first
        for ( KSMClient* c = clients.first(); c; c = clients.next() )
            if ( c->program() == wm ) {
                clients.prepend( clients.take() );
                break;
            }
    }

    for ( KSMClient* c = clients.first(); c; c = clients.next() ) {
        int restartHint = c->restartStyleHint();
        if (restartHint == SmRestartNever)
           continue;
        QString program = c->program();
        QStringList restartCommand = c->restartCommand();
        if (program.isEmpty() && restartCommand.isEmpty())
           continue;
        if (excludeApps.contains( program.lower()))
            continue;

        count++;
        QString n = QString::number(count);
        config->writeEntry( QString("program")+n, program );
        config->writeEntry( QString("clientId")+n, c->clientId() );
        config->writeEntry( QString("restartCommand")+n, restartCommand );
        config->writePathEntry( QString("discardCommand")+n, c->discardCommand() );
        config->writeEntry( QString("restartStyleHint")+n, restartHint );
        config->writeEntry( QString("userId")+n, c->userId() );
    }
    config->writeEntry( "count", count );

    config->setGroup("General");
    config->writeEntry( "screenCount", ScreenCount(qt_xdisplay()));

#ifndef NO_LEGACY_SESSION_MANAGEMENT
    storeLegacySession( config );
#endif
    config->sync();
}


/*!  Restores the previous session. Ensures the window manager is
  running (if specified).
 */
void KSMServer::restoreSession( QString sessionName )
{
    kdDebug() << "KSMServer::restoreSession " << sessionName << endl;
    upAndRunning( "restore session");
    KConfig* config = KGlobal::config();

    sessionGroup = "Session: " + sessionName;

    config->setGroup( sessionGroup );
    int count =  config->readNumEntry( "count" );
    appsToStart = count;

    QValueList<QStringList> wmCommands;
    if ( !wm.isEmpty() ) {
	for ( int i = 1; i <= count; i++ ) {
	    QString n = QString::number(i);
	    if ( wm == config->readEntry( QString("program")+n ) ) {
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
        for (uint i = 0; i < wmCommands.count(); i++)
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
    startApplication( wm );
    QTimer::singleShot( 4000, this, SLOT( autoStart() ) );
}


void KSMServer::logout( int confirm, int sdtype, int sdmode )
{
    shutdown( (KApplication::ShutdownConfirm)confirm,
              (KApplication::ShutdownType)sdtype,
              (KApplication::ShutdownMode)sdmode );
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
        restoreNextInternal();
}


void KSMServer::restoreSessionInternal()
{
    disconnectDCOPSignal( launcher, launcher, "autoStartDone()",
                          "restoreSessionInternal()");
    lastAppStarted = 0;
    lastIdStarted = QString::null;
    restoreNextInternal();
}

void KSMServer::restoreNextInternal()
{
    restoreTimer.stop();
    KConfig* config = KGlobal::config();
    config->setGroup( sessionGroup );

    while ( lastAppStarted < appsToStart ) {
        publishProgress ( appsToStart - lastAppStarted );
        lastAppStarted++;
        QString n = QString::number(lastAppStarted);
        QStringList restartCommand = config->readListEntry( QString("restartCommand")+n );
        if ( restartCommand.isEmpty() ||
             (config->readNumEntry( QString("restartStyleHint")+n ) == SmRestartNever)) {
            continue;
        }
        if ( wm == config->readEntry( QString("program")+n ) )
            continue;
        startApplication( restartCommand );
        lastIdStarted = config->readEntry( QString("clientId")+n );
        if ( !lastIdStarted.isEmpty() ) {
            restoreTimer.start( 2000, TRUE );
            return; // we get called again from the clientRegistered handler
        }
    }

    appsToStart = 0;
    lastIdStarted = QString::null;
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
    
    // From now on handle X errors as normal shutdown.
    XSetIOErrorHandler(Xio_ErrorHandler);
}

void KSMServer::publishProgress( int progress, bool max  )
{
    DCOPRef( "ksplash" ).send( max ? "setMaxProgress" : "setProgress", progress );
}


void KSMServer::upAndRunning( const QString& msg )
{
    DCOPRef( "ksplash" ).send( "upAndRunning", msg );
}


QStringList KSMServer::sessionList()
{
    QStringList sessions = "default";
    KConfig* config = KGlobal::config();
    QStringList groups = config->groupList();
    for ( QStringList::ConstIterator it = groups.begin(); it != groups.end(); it++ )
        if ( (*it).startsWith( "Session: " ) )
            sessions << (*it).mid( 9 );
    return sessions;
}

bool KSMServer::isWM( const KSMClient* client ) const
{
    // KWin relies on ksmserver's special treatment in phase1,
    // therefore make sure it's recognized even if ksmserver
    // was initially started with different WM, and kwin replaced
    // it later
    return client->program() == wm
        || client->program() == "kwin";
}

/*
 * Legacy session management
 */

#ifndef NO_LEGACY_SESSION_MANAGEMENT
const int WM_SAVE_YOURSELF_TIMEOUT = 4000;

static WindowMap* windowMapPtr = 0;

static Atom wm_save_yourself = XNone;
static Atom wm_protocols = XNone;
static Atom wm_client_leader = XNone;

extern Time qt_x_time;

static int winsErrorHandler(Display *, XErrorEvent *ev)
{
    if (windowMapPtr) {
        WindowMap::Iterator it = windowMapPtr->find(ev->resourceid);
        if (it != windowMapPtr->end())
            (*it).type = SM_ERROR;
    }
    return 0;
}

void KSMServer::performLegacySessionSave()
{
    kdDebug() << "Saving legacy session apps" << endl;
    // Setup error handler
    legacyWindows.clear();
    windowMapPtr = &legacyWindows;
    XErrorHandler oldHandler = XSetErrorHandler(winsErrorHandler);
    // Compute set of leader windows that need legacy session management
    // and determine which style (WM_COMMAND or WM_SAVE_YOURSELF)
    KWinModule module;
    if( wm_save_yourself == (Atom)XNone ) {
	Atom atoms[ 3 ];
	const char* const names[]
	    = { "WM_SAVE_YOURSELF", "WM_PROTOCOLS", "WM_CLIENT_LEADER" };
	XInternAtoms( qt_xdisplay(), const_cast< char** >( names ), 3,
	    False, atoms );
	wm_save_yourself = atoms[ 0 ];
	wm_protocols = atoms[ 1 ];
	wm_client_leader = atoms[ 2 ];
    }
    for ( QValueList<WId>::ConstIterator it = module.windows().begin();
	  it != module.windows().end(); ++it) {
        WId leader = windowWmClientLeader( *it );
        if (!legacyWindows.contains(leader) && windowSessionId( *it, leader ).isEmpty()) {
            SMType wtype = SM_WMCOMMAND;
            int nprotocols = 0;
            Atom *protocols = 0;
            if( XGetWMProtocols(qt_xdisplay(), leader, &protocols, &nprotocols)) {
                for (int i=0; i<nprotocols; i++)
                    if (protocols[i] == wm_save_yourself) {
                        wtype = SM_WMSAVEYOURSELF;
                        break;
                    }
                XFree((void*) protocols);
            }
	    SMData data;
	    data.type = wtype;
            XClassHint classHint;
            if( XGetClassHint( qt_xdisplay(), leader, &classHint ) ) {
                data.wmclass1 = classHint.res_name;
                data.wmclass2 = classHint.res_class;
                XFree( classHint.res_name );
                XFree( classHint.res_class );
            }
            legacyWindows.insert(leader, data);
        }
    }
    // Open fresh display for sending WM_SAVE_YOURSELF
    XSync(qt_xdisplay(), False);
    Display *newdisplay = XOpenDisplay(DisplayString(qt_xdisplay()));
    if (!newdisplay) {
	windowMapPtr = NULL;
	XSetErrorHandler(oldHandler);
	return;
    }
    WId root = DefaultRootWindow(newdisplay);
    XGrabKeyboard(newdisplay, root, False,
                  GrabModeAsync, GrabModeAsync, CurrentTime);
    XGrabPointer(newdisplay, root, False, Button1Mask|Button2Mask|Button3Mask,
                 GrabModeAsync, GrabModeAsync, XNone, XNone, CurrentTime);
    // Send WM_SAVE_YOURSELF messages
    XEvent ev;
    int awaiting_replies = 0;
    for (WindowMap::Iterator it = legacyWindows.begin(); it != legacyWindows.end(); ++it) {
        if ( (*it).type == SM_WMSAVEYOURSELF ) {
            WId w = it.key();
            awaiting_replies += 1;
            memset(&ev, 0, sizeof(ev));
            ev.xclient.type = ClientMessage;
            ev.xclient.window = w;
            ev.xclient.message_type = wm_protocols;
            ev.xclient.format = 32;
            ev.xclient.data.l[0] = wm_save_yourself;
            ev.xclient.data.l[1] = qt_x_time;
            XSelectInput(newdisplay, w, PropertyChangeMask|StructureNotifyMask);
            XSendEvent(newdisplay, w, False, 0, &ev);
        }
    }
    // Wait for change in WM_COMMAND with timeout
    XFlush(newdisplay);
    QTime start = QTime::currentTime();
    while (awaiting_replies > 0) {
        if (XPending(newdisplay)) {
            /* Process pending event */
            XNextEvent(newdisplay, &ev);
            if ( ( ev.xany.type == UnmapNotify ) ||
                 ( ev.xany.type == PropertyNotify && ev.xproperty.atom == XA_WM_COMMAND ) ) {
                WindowMap::Iterator it = legacyWindows.find( ev.xany.window );
                if ( it != legacyWindows.end() && (*it).type != SM_WMCOMMAND ) {
                    awaiting_replies -= 1;
                    if ( (*it).type != SM_ERROR )
                        (*it).type = SM_WMCOMMAND;
                }
            }
        } else {
            /* Check timeout */
            int msecs = start.elapsed();
            if (msecs >= WM_SAVE_YOURSELF_TIMEOUT)
                break;
            /* Wait for more events */
            fd_set fds;
            FD_ZERO(&fds);
            int fd = ConnectionNumber(newdisplay);
            FD_SET(fd, &fds);
            struct timeval tmwait;
            tmwait.tv_sec = (WM_SAVE_YOURSELF_TIMEOUT - msecs) / 1000;
            tmwait.tv_usec = ((WM_SAVE_YOURSELF_TIMEOUT - msecs) % 1000) * 1000;
            ::select(fd+1, &fds, NULL, &fds, &tmwait);
        }
    }
    // Terminate work in new display
    XAllowEvents(newdisplay, ReplayPointer, CurrentTime);
    XAllowEvents(newdisplay, ReplayKeyboard, CurrentTime);
    XSync(newdisplay, False);
    XCloseDisplay(newdisplay);
    // Restore old error handler
    XSync(qt_xdisplay(), False);
    XSetErrorHandler(oldHandler);
    for (WindowMap::Iterator it = legacyWindows.begin(); it != legacyWindows.end(); ++it) {
        if ( (*it).type != SM_ERROR) {
            WId w = it.key();
            (*it).wmCommand = windowWmCommand(w);
            (*it).wmClientMachine = windowWmClientMachine(w);
	}
    }
    kdDebug() << "Done saving " << legacyWindows.count() << " legacy session apps" << endl;
}

/*!
  Stores legacy session management data
*/
void KSMServer::storeLegacySession( KConfig* config )
{
    // Write LegacySession data
    config->deleteGroup( "Legacy" + sessionGroup );
    KConfigGroupSaver saver( config, "Legacy" + sessionGroup );
    int count = 0;
    for (WindowMap::ConstIterator it = legacyWindows.begin(); it != legacyWindows.end(); ++it) {
        if ( (*it).type != SM_ERROR) {
            if( excludeApps.contains( (*it).wmclass1.lower())
                || excludeApps.contains( (*it).wmclass2.lower()))
                continue;
            if ( !(*it).wmCommand.isEmpty() && !(*it).wmClientMachine.isEmpty() ) {
                count++;
                QString n = QString::number(count);
                config->writeEntry( QString("command")+n, (*it).wmCommand );
                config->writeEntry( QString("clientMachine")+n, (*it).wmClientMachine );
            }
        }
    }
    config->writeEntry( "count", count );
}

/*!
  Restores legacy session management data (i.e. restart applications)
*/
void KSMServer::restoreLegacySession( KConfig* config )
{
    if( config->hasGroup( "Legacy" + sessionGroup )) {
        KConfigGroupSaver saver( config, "Legacy" + sessionGroup );
        restoreLegacySessionInternal( config );
    } else if( wm == "kwin" ) { // backwards comp. - get it from kwinrc
	KConfigGroupSaver saver( config, sessionGroup );
	int count =  config->readNumEntry( "count", 0 );
	for ( int i = 1; i <= count; i++ ) {
    	    QString n = QString::number(i);
    	    if ( config->readEntry( QString("program")+n ) != wm )
                continue;
    	    QStringList restartCommand =
                config->readListEntry( QString("restartCommand")+n );
	    for( QStringList::ConstIterator it = restartCommand.begin();
		 it != restartCommand.end();
		 ++it ) {
		if( (*it) == "-session" ) {
		    ++it;
		    if( it != restartCommand.end()) {
			KConfig cfg( "session/" + wm + "_" + (*it), true );
			cfg.setGroup( "LegacySession" );
			restoreLegacySessionInternal( &cfg, ' ' );
		    }
		}
	    }
	}
    }
}

void KSMServer::restoreLegacySessionInternal( KConfig* config, char sep )
{
    int count = config->readNumEntry( "count" );
    for ( int i = 1; i <= count; i++ ) {
        QString n = QString::number(i);
        QStringList wmCommand = config->readListEntry( QString("command")+n, sep );
        QString wmClientMachine = config->readEntry( QString("clientMachine")+n );
        if ( !wmCommand.isEmpty() && !wmClientMachine.isEmpty() ) {
            if ( wmClientMachine != "localhost" ) {
		wmCommand.prepend( wmClientMachine );
		wmCommand.prepend( "xon" );
	    }
	startApplication( wmCommand );
        }
    }
}
    
static QCString getQCStringProperty(WId w, Atom prop)
{
    Atom type;
    int format, status;
    unsigned long nitems = 0;
    unsigned long extra = 0;
    unsigned char *data = 0;
    QCString result = "";
    status = XGetWindowProperty( qt_xdisplay(), w, prop, 0, 10000,
                                 FALSE, XA_STRING, &type, &format,
                                 &nitems, &extra, &data );
    if ( status == Success) {
	if( data )
	    result = (char*)data;
        XFree(data);
    }
    return result;
}

static QStringList getQStringListProperty(WId w, Atom prop)
{
    Atom type;
    int format, status;
    unsigned long nitems = 0;
    unsigned long extra = 0;
    unsigned char *data = 0;
    QStringList result;

    status = XGetWindowProperty( qt_xdisplay(), w, prop, 0, 10000,
                                 FALSE, XA_STRING, &type, &format,
                                 &nitems, &extra, &data );
    if ( status == Success) {
	if (!data)
	    return result;
        for (int i=0; i<(int)nitems; i++) {
            result << QString::fromLatin1( (const char*)data + i );
            while(data[i]) i++;
        }
        XFree(data);
    }
    return result;
}

QStringList KSMServer::windowWmCommand(WId w)
{
    return getQStringListProperty(w, XA_WM_COMMAND);
}

QString KSMServer::windowWmClientMachine(WId w)
{
    QCString result = getQCStringProperty(w, XA_WM_CLIENT_MACHINE);
    if (result.isEmpty()) {
        result = "localhost";
    } else {
        // special name for the local machine (localhost)
        char hostnamebuf[80];
        if (gethostname (hostnamebuf, sizeof hostnamebuf) >= 0) {
            hostnamebuf[sizeof(hostnamebuf)-1] = 0;
            if (result == hostnamebuf)
                result = "localhost";
            char *dot = strchr(hostnamebuf, '.');
            if (dot && !(*dot = 0) && result == hostnamebuf)
                result = "localhost";
        }
    }
    return QString::fromLatin1(result);
}

WId KSMServer::windowWmClientLeader(WId w)
{
    Atom type;
    int format, status;
    unsigned long nitems = 0;
    unsigned long extra = 0;
    unsigned char *data = 0;
    Window result = w;
    status = XGetWindowProperty( qt_xdisplay(), w, wm_client_leader, 0, 10000,
                                 FALSE, XA_WINDOW, &type, &format,
                                 &nitems, &extra, &data );
    if (status  == Success ) {
        if (data && nitems > 0)
            result = *((Window*) data);
        XFree(data);
    }
    return result;
}


/*
  Returns sessionId for this client,
  taken either from its window or from the leader window.
 */
extern Atom qt_sm_client_id;
QCString KSMServer::windowSessionId(WId w, WId leader)
{
    QCString result = getQCStringProperty(w, qt_sm_client_id);
    if (result.isEmpty() && leader != (WId)XNone && leader != w)
	result = getQCStringProperty(leader, qt_sm_client_id);
    return result;
}
#endif
