/*

Copyright 1988, 1998  The Open Group
Copyright 2000-2005 Oswald Buddenhagen <ossi@kde.org>

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of a copyright holder shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from the copyright holder.

*/

/*
 * xdm - display manager daemon
 * Author: Keith Packard, MIT X Consortium
 *
 * display manager
 */

#define NEED_SIGNAL
#define NEED_UTMP
#include "dm.h"
#include "dm_auth.h"
#include "dm_error.h"

#include <stdio.h>
#include <string.h>

#ifndef sigmask
# define sigmask(m) (1 << ((m - 1)))
#endif

#include <sys/stat.h>
#include <X11/Xfuncproto.h>
#include <stdarg.h>

#ifndef X_NOT_POSIX
# include <unistd.h>
#endif

#ifndef PATH_MAX
# ifdef WIN32
#  define PATH_MAX 512
# else
#  include <sys/param.h>
# endif
# ifndef PATH_MAX
#  ifdef MAXPATHLEN
#   define PATH_MAX MAXPATHLEN
#  else
#   define PATH_MAX 1024
#  endif
# endif
#endif

#ifdef HAVE_VTS
# include <sys/ioctl.h>
# include <sys/vt.h>
#endif

#if defined(SVR4) && !defined(SCO) && !defined(sun)
extern FILE *fdopen();
#endif

static SIGVAL SigHandler( int n );
static int ScanConfigs( int force );
static void StartDisplays( void );
#define XS_KEEP 0
#define XS_RESTART 1
#define XS_RETRY 2
static void ExitDisplay( struct display *d, int endState, int serverCmd, int goodExit );
static void rStopDisplay( struct display *d, int endState );
static void MainLoop( void );

static int signalFds[2];

#if !defined(HAS_SETPROCTITLE) && !defined(NOXDMTITLE)
static char *Title;
static int TitleLen;
#endif

static int StorePid( void );

static int Stopping;
SdRec sdRec = { 0, 0, 0, TO_INF, TO_INF };

time_t now;

char *prog, *progpath;

int
main( int argc, char **argv )
{
	int oldpid, oldumask, fd, noDaemonMode;
	char *pt, *errorLogFile, **opts;

	/* make sure at least world write access is disabled */
	if (((oldumask = umask( 022 )) & 002) == 002)
		(void)umask( oldumask );

	/* give /dev/null as stdin */
	if ((fd = open( "/dev/null", O_RDONLY )) > 0) {
		dup2( fd, 0 );
		close( fd );
	}
	if (fcntl( 1, F_GETFD ) < 0)
		dup2( 0, 1 );
	if (fcntl( 2, F_GETFD ) < 0)
		dup2( 0, 2 );

	if (argv[0][0] == '/') {
		if (!StrDup( &progpath, argv[0] ))
			Panic( "Out of memory" );
	} else
#ifdef linux
	{
		/* note that this will resolve symlinks ... */
		int len;
		char fullpath[PATH_MAX];
		if ((len = readlink( "/proc/self/exe", fullpath, sizeof(fullpath) )) < 0)
			Panic( "Invoke with full path specification or mount /proc" );
		if (!StrNDup( &progpath, fullpath, len ))
			Panic( "Out of memory" );
	}
#else
# if 0
		Panic( "Must be invoked with full path specification" );
# else
	{
		char directory[PATH_MAX+1];
#  if !defined(X_NOT_POSIX) || defined(SYSV) || defined(WIN32)
		if (!getcwd( directory, sizeof(directory) ))
			Panic( "Can't find myself (getcwd failed)" );
#  else
		if (!getwd( directory ))
			Panic( "Can't find myself (getwd failed)" );
#  endif
		if (strchr( argv[0], '/' ))
			StrApp( &progpath, directory, "/", argv[0], (char *)0 );
		else {
			int len;
			char *path, *pathe, *name, *thenam, nambuf[PATH_MAX+1];

			if (!(path = getenv( "PATH" )))
				Panic( "Can't find myself (no PATH)" );
			len = strlen( argv[0] );
			name = nambuf + PATH_MAX - len;
			memcpy( name, argv[0], len + 1 );
			*--name = '/';
			do {
				if (!(pathe = strchr( path, ':' )))
					pathe = path + strlen( path );
				len = pathe - path;
				if (!len || (len == 1 && *path == '.')) {
					len = strlen( directory );
					path = directory;
				}
				thenam = name - len;
				if (thenam >= nambuf) {
					memcpy( thenam, path, len );
					if (!access( thenam, X_OK ))
						goto found;
				}
				path = pathe;
			} while (*path++ != '\0');
			Panic( "Can't find myself (not in PATH)" );
		  found:
			if (!StrDup( &progpath, thenam ))
				Panic( "Out of memory" );
		}
	}
# endif
#endif
	prog = strrchr( progpath, '/' ) + 1;

#if !defined(HAS_SETPROCTITLE) && !defined(NOXDMTITLE)
	Title = argv[0];
	TitleLen = (argv[argc - 1] + strlen( argv[argc - 1] )) - Title;
#endif

	/*
	 * Parse command line options
	 */
	noDaemonMode = getppid();
	errorLogFile = 0;
	if (!(opts = Malloc( 2 * sizeof(char *))))
		return 1;
	opts[0] = (char *)"";
	opts[1] = 0;
	while (*++argv) {
		if (**argv != '-')
			break;
		pt = *argv + 1;
		if (*pt == '-')
			pt++;
		if (!strcmp( pt, "help" ) || !strcmp( pt, "h" )) {
			printf( "Usage: %s [options] [tty]\n"
"  -daemon\t\t  - Daemonize even when started by init\n"
"  -nodaemon\t  - Don't daemonize even when started from command line\n"
"  -config <file>\t  - Use alternative master configuration file\n"
"  -xrm <res>\t  - Override frontend-specific resource\n"
"  -debug <num>\t  - Debug option bitfield\n"
"  -error <file>\t  - Use alternative log file\n", prog );
			exit( 0 );
		} else if (!strcmp( pt, "daemon" ))
			noDaemonMode = 0;
		else if (!strcmp( pt, "nodaemon" ))
			noDaemonMode = 1;
		else if (argv[1] && !strcmp( pt, "config" ))
			StrDup( opts, *++argv );
		else if (argv[1] && !strcmp( pt, "xrm" ))
			opts = addStrArr( opts, *++argv, -1 );
		else if (argv[1] && !strcmp( pt, "debug" ))
			sscanf( *++argv, "%i", &debugLevel );
		else if (argv[1] && (!strcmp( pt, "error" ) || !strcmp( pt, "logfile" )))
			errorLogFile = *++argv;
		else {
			fprintf( stderr, "\"%s\" is an unknown option or is missing a parameter\n", *argv );
			exit( 1 );
		}
	}

	/*
	 * Only allow root to run in non-debug mode to avoid problems
	 */
	if (!debugLevel && getuid()) {
		fprintf( stderr, "Only root wants to run %s\n", prog );
		exit( 1 );
	}

	InitErrorLog( errorLogFile );

	if (noDaemonMode != 1)
		BecomeDaemon();

	/*
	 * Step 1 - load configuration parameters
	 */
	if (!InitResources( opts ) || !ScanConfigs( FALSE ))
		LogPanic( "Config reader failed. Aborting ...\n" );

	/* SUPPRESS 560 */
	if ((oldpid = StorePid())) {
		if (oldpid == -1)
			LogError( "Can't create/lock pid file %s\n", pidFile );
		else
			LogError( "Can't lock pid file %s, another xdm is running (pid %d)\n",
			          pidFile, oldpid );
		exit( 1 );
	}

#if !defined(ARC4_RANDOM) && !defined(DEV_RANDOM)
	AddOtherEntropy();
#endif

	/*
	 * We used to clean up old authorization files here. As authDir is
	 * supposed to be /var/run/xauth or /tmp, we needn't to care for it.
	 */

#ifdef XDMCP
	init_session_id();
#else
	Debug( "not compiled for XDMCP\n" );
#endif
	if (pipe( signalFds ))
		LogPanic( "Unable to create signal notification pipe.\n" );
	RegisterInput( signalFds[0] );
	RegisterCloseOnFork( signalFds[0] );
	RegisterCloseOnFork( signalFds[1] );
	(void)Signal( SIGTERM, SigHandler );
	(void)Signal( SIGINT, SigHandler );
	(void)Signal( SIGHUP, SigHandler );
	(void)Signal( SIGCHLD, SigHandler );
	(void)Signal( SIGUSR1, SigHandler );

	/*
	 * Step 2 - run a sub-daemon for each entry
	 */
#ifdef XDMCP
	UpdateListenSockets();
#endif
	openCtrl( 0 );
	MainLoop();
	closeCtrl( 0 );
	if (sdRec.how) {
		if (Fork() <= 0) {
			char *cmd = sdRec.how == SHUT_HALT ? cmdHalt : cmdReboot;
			execute( parseArgs( (char **)0, cmd ), (char **)0 );
			LogError( "Failed to execute shutdown command %\"s\n", cmd );
			exit( 1 );
		} else {
#ifndef X_NOT_POSIX
			sigset_t mask;
			sigemptyset( &mask );
			sigaddset( &mask, SIGCHLD );
			sigaddset( &mask, SIGHUP );
			sigsuspend( &mask );
#else
			sigpause( sigmask( SIGCHLD ) | sigmask( SIGHUP ) );
#endif
		}
	}
	Debug( "nothing left to do, exiting\n" );
	return 0;
}


#ifdef HAVE_VTS
int
activateVT( int vt )
{
	int ret = 0;
	int con = open( "/dev/console", O_RDONLY );
	if (con >= 0) {
		if (!ioctl( con, VT_ACTIVATE, vt ))
			ret = 1;
		close( con );
	}
	return ret;
}


static void
WakeDisplay( struct display *d )
{
	if (d->status == textMode)
		d->status = (d->displayType & d_lifetime) == dReserve ? reserve : notRunning;
}
#endif

enum utState { UtDead, UtWait, UtActive };

struct utmps {
	struct utmps *next;
#ifndef HAVE_VTS
	struct display *d;
#endif
	time_t time;
	enum utState state;
	int hadSess;
};

#define TIME_LOG 40
#define TIME_RELOG 10

static struct utmps *utmpList;
static time_t utmpTimeout = TO_INF;

static void
bombUtmp( void )
{
	struct utmps *utp;

	while ((utp = utmpList)) {
#ifdef HAVE_VTS
		ForEachDisplay( WakeDisplay );
#else
		utp->d->status = notRunning;
#endif
		utmpList = utp->next;
		free( utp );
	}
}

static void
CheckUtmp( void )
{
	static time_t modtim;
	time_t nck;
	struct utmps *utp, **utpp;
	struct stat st;
#ifdef BSD_UTMP
	int fd;
	struct utmp ut[1];
#else
	struct UTMP *ut;
#endif

	if (!utmpList)
		return;
	if (stat( UTMP_FILE, &st )) {
		LogError( UTMP_FILE " not found - cannot use console mode\n" );
		bombUtmp();
		return;
	}
	if (modtim != st.st_mtime) {
		Debug( "rescanning " UTMP_FILE "\n" );
		for (utp = utmpList; utp; utp = utp->next)
			utp->state = UtDead;
#ifdef BSD_UTMP
		if ((fd = open( UTMP_FILE, O_RDONLY )) < 0) {
			LogError( "Cannot open " UTMP_FILE " - cannot use console mode\n" );
			bombUtmp();
			return;
		}
		while (Reader( fd, ut, sizeof(ut[0]) ) == sizeof(ut[0]))
#else
		SETUTENT();
		while ((ut = GETUTENT()))
#endif
		{
			for (utp = utmpList; utp; utp = utp->next) {
#ifdef HAVE_VTS
				char **line;
				for (line = consoleTTYs; *line; line++)
					if (!strncmp( *line, ut->ut_line, sizeof(ut->ut_line) ))
						goto hitlin;
				continue;
			  hitlin:
#else
				if (strncmp( utp->d->console, ut->ut_line, sizeof(ut->ut_line) ))
					continue;
#endif
#ifdef BSD_UTMP
				if (!*ut->ut_name) {
#else
				if (ut->ut_type != USER_PROCESS) {
#endif
#ifdef HAVE_VTS
					if (utp->state == UtActive)
						break;
#endif
					utp->state = UtWait;
				} else {
					utp->hadSess = 1;
					utp->state = UtActive;
				}
				if (utp->time < ut->ut_time) /* theoretically superfluous */
					utp->time = ut->ut_time;
				break;
			}
		}
#ifdef BSD_UTMP
		close( fd );
#else
		ENDUTENT();
#endif
		modtim = st.st_mtime;
	}
	for (utpp = &utmpList; (utp = *utpp); ) {
		if (utp->state != UtActive) {
			if (utp->state == UtDead) /* shouldn't happen ... */
				utp->time = 0;
			time_t ends = utp->time + (utp->hadSess ? TIME_RELOG : TIME_LOG);
			if (ends <= now) {
#ifdef HAVE_VTS
				ForEachDisplay( WakeDisplay );
				Debug( "console login timed out\n" );
#else
				utp->d->status = notRunning;
				Debug( "console login for %s at %s timed out\n",
				       utp->d->name, utp->d->console );
#endif
				*utpp = utp->next;
				free( utp );
				continue;
			} else
				nck = ends;
		} else
			nck = TIME_RELOG + now;
		if (nck < utmpTimeout)
			utmpTimeout = nck;
		utpp = &(*utpp)->next;
	}
}

static void
#ifdef HAVE_VTS
SwitchToTty( void )
#else
SwitchToTty( struct display *d )
#endif
{
	struct utmps *utp;

	if (!(utp = Malloc( sizeof(*utp) ))) {
#ifdef HAVE_VTS
		ForEachDisplay( WakeDisplay );
#else
		d->status = notRunning;
#endif
		return;
	}
#ifndef HAVE_VTS
	d->status = textMode;
	utp->d = d;
#endif
	utp->time = now;
	utp->hadSess = 0;
	utp->next = utmpList;
	utmpList = utp;
	CheckUtmp();

#ifdef HAVE_VTS
	if (!memcmp( *consoleTTYs, "tty", 3 ))
		activateVT( atoi( *consoleTTYs + 3 ) );
#endif

	/* XXX output something useful here */
}

#ifdef HAVE_VTS
static void
StopToTTY( struct display *d )
{
	if ((d->displayType & d_location) == dLocal)
		switch (d->status) {
		default:
			rStopDisplay( d, DS_TEXTMODE | 0x100 );
		case reserve:
		case textMode:
			break;
		}
}

static void
CheckTTYMode( void )
{
	struct display *d;

	for (d = displays; d; d = d->next)
		if (d->status == zombie)
			return;

	SwitchToTty();
}

#else

void
SwitchToX( struct display *d )
{
	struct utmps *utp, **utpp;

	for (utpp = &utmpList; (utp = *utpp); utpp = &(*utpp)->next)
		if (utp->d == d) {
			*utpp = utp->next;
			free( utp );
			d->status = notRunning;
			return;
		}
}
#endif

#ifdef XDMCP
static void
StartRemoteLogin( struct display *d )
{
	char **argv;
	int pid;

	Debug( "StartRemoteLogin for %s\n", d->name );
	/* HACK: omitting LoadDisplayResources( d ) here! */
	if (d->authorize)
		SetLocalAuthorization( d );
	switch (pid = Fork()) {
	case 0:
		argv = PrepServerArgv( d, d->serverArgsRemote );
		if (!(argv = addStrArr( argv, "-once", 5 )) ||
		    !(argv = addStrArr( argv, "-query", 6 )) ||
		    !(argv = addStrArr( argv, d->remoteHost, -1 )))
			exit( 1 );
		Debug( "exec %\"[s\n", argv );
		(void)execv( argv[0], argv );
		LogError( "X server %\"s cannot be executed\n", argv[0] );
		exit( 1 );
	case -1:
		LogError( "Forking X server for remote login failed: %m" );
		d->status = notRunning;
		return;
	default:
		break;
	}
	Debug( "X server forked, pid %d\n", pid );
	d->serverPid = pid;

	d->status = remoteLogin;
}
#endif


static void
StopInactiveDisplay( struct display *d )
{
	if (d->status != remoteLogin && d->userSess < 0)
		StopDisplay( d );
}

static void
stoppen( int force )
{
#ifdef XDMCP
	request_port = 0;
	UpdateListenSockets();
#endif
	if (force)
		ForEachDisplay( StopDisplay );
	else
		ForEachDisplay( StopInactiveDisplay );
	Stopping = 1;
}


void
setNLogin( struct display *d,
           const char *nuser, const char *npass, char *nargs, int rl )
{
	struct disphist *he = d->hstent;
	he->rLogin =
		(ReStr( &he->nuser, nuser ) &&
		 ReStr( &he->npass, npass ) &&
		 ReStr( &he->nargs, nargs )) ? rl : 0;
	Debug( "set next login for %s, level %d\n", nuser, rl );
}

static void
processDPipe( struct display *d )
{
	char *user, *pass, *args;
	int cmd;
	GTalk dpytalk;
#ifdef XDMCP
	int ct, len;
	ARRAY8 ca, ha;
#endif

	dpytalk.pipe = &d->pipe;
	if (Setjmp( dpytalk.errjmp )) {
		StopDisplay( d );
		return;
	}
	GSet( &dpytalk );
	if (!GRecvCmd( &cmd )) {
		/* process already exited */
		UnregisterInput( d->pipe.rfd );
		return;
	}
	switch (cmd) {
	case D_User:
		d->userSess = GRecvInt();
		d->userName = GRecvStr();
		d->sessName = GRecvStr();
		break;
	case D_ReLogin:
		user = GRecvStr();
		pass = GRecvStr();
		args = GRecvStr();
		setNLogin( d, user, pass, args, 1 );
		free( args );
		free( pass );
		free( user );
		break;
#ifdef XDMCP
	case D_ChooseHost:
		ca.data = (unsigned char *)GRecvArr( &len );
		ca.length = (CARD16)len;
		ct = GRecvInt();
		ha.data = (unsigned char *)GRecvArr( &len );
		ha.length = (CARD16)len;
		RegisterIndirectChoice( &ca, ct, &ha );
		XdmcpDisposeARRAY8( &ha );
		XdmcpDisposeARRAY8( &ca );
		break;
	case D_RemoteHost:
		if (d->remoteHost)
			free( d->remoteHost );
		d->remoteHost = GRecvStr();
		break;
#endif
	case D_XConnOk:
		startingServer = 0;
		break;
	default:
		LogError( "Internal error: unknown D_* command %d\n", cmd );
		StopDisplay( d );
		break;
	}
}

static void
processGPipe( struct display *d )
{
	struct display *di;
	int cmd, flags;
	GTalk dpytalk;

	dpytalk.pipe = &d->gpipe;
	if (Setjmp( dpytalk.errjmp )) {
		StopDisplay( d );
		return;
	}
	GSet( &dpytalk );
	if (!GRecvCmd( &cmd )) {
		/* process already exited */
		UnregisterInput( d->gpipe.rfd );
		return;
	}
	switch (cmd) {
	case G_Shutdown:
		sdRec.how = GRecvInt();
		sdRec.start = GRecvInt();
		sdRec.timeout = GRecvInt();
		sdRec.force = GRecvInt();
		sdRec.uid = GRecvInt();
		break;
	case G_QueryShutdown:
		GSendInt( sdRec.how );
		GSendInt( sdRec.start );
		GSendInt( sdRec.timeout );
		GSendInt( sdRec.force );
		GSendInt( sdRec.uid );
		break;
	case G_List:
		flags = GRecvInt();
		for (di = displays; di; di = di->next)
			if (((flags & lstRemote) || (di->displayType & d_location) == dLocal) &&
			    (di->status == remoteLogin ||
			     ((flags & lstPassive) ? di->status == running : di->userSess >= 0)))
			{
				GSendStr( di->name );
#ifdef HAVE_VTS
				GSendInt( di->serverVT );
#endif
				GSendStr( di->status == remoteLogin ? "" : di->userName );
				GSendStr( di->sessName );
				GSendInt( di == d );
			}
		GSendInt( 0 );
		break;
#ifdef HAVE_VTS
	case G_Activate:
		activateVT( GRecvInt() );
		break;
#endif
	case G_Console:
#ifdef HAVE_VTS
		if (*consoleTTYs) { /* sanity check against greeter */
			ForEachDisplay( StopToTTY );
			CheckTTYMode();
		}
#else
		if (*d->console) /* sanity check against greeter */
			rStopDisplay( d, DS_TEXTMODE );
#endif
		break;
	default:
		LogError( "Internal error: unknown G_* command %d\n", cmd );
		StopDisplay( d );
		break;
	}
}


static int
ScanConfigs( int force )
{
	if (!LoadDMResources( force ))
		return FALSE;
	ScanServers();
#ifdef XDMCP
	ScanAccessDatabase( force );
#endif
	return TRUE;
}

static void
MarkDisplay( struct display *d )
{
	d->stillThere = 0;
}

static void
RescanConfigs( int force )
{
	if (ScanConfigs( force )) {
#ifdef XDMCP
		UpdateListenSockets();
#endif
		updateCtrl();
	}
}

void
cancelShutdown( void )
{
	sdRec.how = 0;
	Stopping = 0;
	RescanConfigs( TRUE );
}


static void
ReapChildren( void )
{
	int pid;
	struct display *d;
	waitType status;

#ifndef X_NOT_POSIX
	while ((pid = waitpid( -1, &status, WNOHANG )) > 0)
#else
	while ((pid = wait3( &status, WNOHANG, (struct rusage *)0 )) > 0)
#endif
	{
		Debug( "manager wait returns  pid %d  sig %d  core %d  code %d\n",
		       pid, waitSig( status ), waitCore( status ), waitCode( status ) );
		/* SUPPRESS 560 */
		if ((d = FindDisplayByPid( pid ))) {
			d->pid = -1;
			UnregisterInput( d->pipe.rfd );
			GClosen (&d->pipe);
			UnregisterInput( d->gpipe.rfd );
			GClosen (&d->gpipe);
			closeCtrl( d );
			switch (waitVal( status )) {
#ifdef XDMCP
			case EX_REMOTE:
				Debug( "display exited with EX_REMOTE\n" );
				ExitDisplay( d, DS_REMOTE, 0, 0 );
				break;
#endif
			case EX_NORMAL:
				/* (any type of) session ended */
				Debug( "display exited with EX_NORMAL\n" );
				if ((d->displayType & d_lifetime) == dReserve)
					ExitDisplay( d, DS_RESERVE, 0, 0 );
				else
					ExitDisplay( d, DS_RESTART, XS_KEEP, TRUE );
				break;
#if 0
			case EX_REMANAGE_DPY:
				/* user session ended */
				Debug( "display exited with EX_REMANAGE_DPY\n" );
				ExitDisplay( d, DS_RESTART, XS_KEEP, TRUE );
				break;
#endif
			case EX_OPENFAILED_DPY:
				/* WaitForServer() failed */
				LogError( "Display %s cannot be opened\n", d->name );
#ifdef XDMCP
				/*
				 * no display connection was ever made, tell the
				 * terminal that the open attempt failed
				 */
				if ((d->displayType & d_origin) == dFromXDMCP)
					SendFailed( d, "cannot open display" );
#endif
				ExitDisplay( d, DS_RESTART, XS_RETRY, FALSE );
				break;
			case waitCompose( SIGTERM,0,0 ):
				/* killed before/during WaitForServer()
				   - local Xserver died
				   - display stopped (is zombie)
				   - "login now" and "suicide" pipe commands (is raiser)
				*/
				Debug( "display exited on SIGTERM\n" );
				ExitDisplay( d, DS_RESTART, XS_RETRY, FALSE );
				break;
			case EX_AL_RESERVER_DPY:
				/* - killed after WaitForServer()
				   - Xserver dead after remote session exit
				*/
				Debug( "display exited with EX_AL_RESERVER_DPY\n" );
				ExitDisplay( d, DS_RESTART, XS_RESTART, FALSE );
				break;
			case EX_RESERVER_DPY:
				/* induced by greeter:
				   - could not secure display
				   - requested by user
				*/
				Debug( "display exited with EX_RESERVER_DPY\n" );
				ExitDisplay( d, DS_RESTART, XS_RESTART, TRUE );
				break;
			case EX_UNMANAGE_DPY:
				/* some fatal error */
				Debug( "display exited with EX_UNMANAGE_DPY\n" );
				ExitDisplay( d, DS_REMOVE, 0, 0 );
				break;
			default:
				/* prolly crash */
				LogError( "Unknown session exit code %d (sig %d) from manager process\n",
				          waitCode( status ), waitSig( status ) );
				ExitDisplay( d, DS_REMOVE, 0, 0 );
				break;
			}
		} else if ((d = FindDisplayByServerPid( pid ))) {
			d->serverPid = -1;
			switch (d->status) {
			case zombie:
				Debug( "zombie X server for display %s reaped\n", d->name );
#ifdef HAVE_VTS
				if (d->serverVT && d->zstatus != DS_REMOTE) {
					if (d->follower) {
						d->follower->serverVT = d->serverVT;
						d->follower = 0;
					} else {
						int con = open( "/dev/console", O_RDONLY );
						if (con >= 0) {
							struct vt_stat vtstat;
							ioctl( con, VT_GETSTATE, &vtstat );
							if (vtstat.v_active == d->serverVT) {
								int vt = 1;
								struct display *di;
								for (di = displays; di; di = di->next)
									if (di != d && di->serverVT)
										vt = di->serverVT;
								for (di = displays; di; di = di->next)
									if (di != d && di->serverVT &&
									    (di->userSess >= 0 ||
									     di->status == remoteLogin))
										vt = di->serverVT;
								ioctl( con, VT_ACTIVATE, vt );
							}
							ioctl( con, VT_DISALLOCATE, d->serverVT );
							close( con );
						}
					}
					d->serverVT = 0;
				}
#endif
				rStopDisplay( d, d->zstatus );
				break;
			case phoenix:
				Debug( "phoenix X server arises, restarting display %s\n",
				       d->name );
				d->status = notRunning;
				break;
			case remoteLogin:
				Debug( "remote login X server for display %s exited\n",
				       d->name );
				d->status = ((d->displayType & d_lifetime) == dReserve) ?
				            reserve : notRunning;
				break;
			case raiser:
				LogError( "X server for display %s terminated unexpectedly\n",
				          d->name );
				/* don't kill again */
				break;
			case running:
				if (startingServer == d && d->serverStatus != ignore) {
					if (d->serverStatus == starting && waitCode( status ) != 47)
						LogError( "X server died during startup\n" );
					StartServerFailed();
					break;
				}
				LogError( "X server for display %s terminated unexpectedly\n",
				          d->name );
				if (d->pid != -1) {
					Debug( "terminating session pid %d\n", d->pid );
					TerminateProcess( d->pid, SIGTERM );
				}
				break;
			case notRunning:
			case textMode:
			case reserve:
				/* this cannot happen */
				Debug( "X server exited for passive (%d) session on display %s\n",
				       (int)d->status, d->name );
				break;
			}
		} else
			Debug( "unknown child termination\n" );
	}
#if !defined(ARC4_RANDOM) && !defined(DEV_RANDOM)
	AddOtherEntropy();
#endif
}

static int
wouldShutdown( void )
{
	struct display *d;

	if (sdRec.force != SHUT_CANCEL) {
		if (sdRec.force == SHUT_FORCEMY)
			for (d = displays; d; d = d->next)
				if (d->status == remoteLogin ||
				    (d->userSess >= 0 && d->userSess != sdRec.uid))
					return 0;
		return 1;
	}
	return !AnyActiveDisplays();
}

FD_TYPE WellKnownSocketsMask;
int WellKnownSocketsMax;
int WellKnownSocketsCount;

void
RegisterInput( int fd )
{
	/* can be omited, as it is always called right after opening a socket
	if (!FD_ISSET (fd, &WellKnownSocketsMask))
	*/
	{
		FD_SET( fd, &WellKnownSocketsMask );
		if (fd > WellKnownSocketsMax)
			WellKnownSocketsMax = fd;
		WellKnownSocketsCount++;
	}
}

void
UnregisterInput( int fd )
{
	/* the check _is_ necessary, as some handles are unregistered before
	   the regular close sequence.
	*/
	if (FD_ISSET( fd, &WellKnownSocketsMask )) {
		FD_CLR( fd, &WellKnownSocketsMask );
		WellKnownSocketsCount--;
	}
}

static SIGVAL
SigHandler( int n )
{
	int olderrno = errno;
	char buf = (char)n;
	Debug( "caught signal %d\n", n );
	write( signalFds[1], &buf, 1 );
#ifdef SIGNALS_RESET_WHEN_CAUGHT
	(void)Signal( n, SigHandler );
#endif
	errno = olderrno;
}

static void
MainLoop( void )
{
	struct display *d;
	struct timeval *tvp, tv;
	time_t to;
	int nready;
	char buf;
	FD_TYPE reads;

	Debug( "MainLoop\n" );
	time( &now );
	while (
#ifdef XDMCP
	       AnyListenSockets() ||
#endif
		   (Stopping ? AnyRunningDisplays() : AnyDisplaysLeft()))
	{
		if (!Stopping)
			StartDisplays();
		to = TO_INF;
		if (sdRec.how) {
			if (sdRec.start != TO_INF && now < sdRec.start) {
				/*if (sdRec.start < to)*/
					to = sdRec.start;
			} else {
				sdRec.start = TO_INF;
				if (now >= sdRec.timeout) {
					sdRec.timeout = TO_INF;
					if (wouldShutdown())
						stoppen( TRUE );
					else
						cancelShutdown();
				} else {
					stoppen( FALSE );
					/*if (sdRec.timeout < to)*/
						to = sdRec.timeout;
				}
			}
		}
		if (serverTimeout < to)
			to = serverTimeout;
		if (utmpTimeout < to)
			to = utmpTimeout;
		if (to == TO_INF)
			tvp = 0;
		else {
			to -= now;
			if (to < 0)
				to = 0;
			tv.tv_sec = to;
			tv.tv_usec = 0;
			tvp = &tv;
		}
		reads = WellKnownSocketsMask;
		nready = select( WellKnownSocketsMax + 1, &reads, 0, 0, tvp );
		Debug( "select returns %d\n", nready );
		time( &now );
#if !defined(ARC4_RANDOM) && !defined(DEV_RANDOM)
		AddTimerEntropy();
#endif
		if (now >= serverTimeout) {
			serverTimeout = TO_INF;
			StartServerTimeout();
		}
		if (now >= utmpTimeout) {
			utmpTimeout = TO_INF;
			CheckUtmp();
		}
		if (nready > 0) {
			/*
			 * we restart after the first handled fd, as
			 * a) it makes things simpler
			 * b) the probability that multiple fds trigger at once is
			 *    ridiculously small. we handle it in the next iteration.
			 */
			/* XXX a cleaner solution would be a callback mechanism */
			if (FD_ISSET( signalFds[0], &reads )) {
				if (read( signalFds[0], &buf, 1 ) != 1)
					LogPanic( "Signal notification pipe broken.\n" );
				switch (buf) {
				case SIGTERM:
				case SIGINT:
					Debug( "shutting down entire manager\n" );
					stoppen( TRUE );
					break;
				case SIGHUP:
					LogInfo( "Rescanning all config files\n" );
					ForEachDisplay( MarkDisplay );
					RescanConfigs( TRUE );
					break;
				case SIGCHLD:
					ReapChildren();
					if (!Stopping && autoRescan)
						RescanConfigs( FALSE );
					break;
				case SIGUSR1:
					if (startingServer &&
					    startingServer->serverStatus == starting)
						StartServerSuccess();
					break;
				}
				continue;
			}
#ifdef XDMCP
			if (ProcessListenSockets( &reads ))
				continue;
#endif	/* XDMCP */
			if (handleCtrl( &reads, 0 ))
				continue;
			/* Must be last (because of the breaks)! */
		  again:
			for (d = displays; d; d = d->next) {
				if (handleCtrl( &reads, d ))
					goto again;
				if (d->pipe.rfd >= 0 && FD_ISSET( d->pipe.rfd, &reads )) {
					processDPipe( d );
					break;
				}
				if (d->gpipe.rfd >= 0 && FD_ISSET( d->gpipe.rfd, &reads )) {
					processGPipe( d );
					break;
				}
			}
		}
	}
}

static void
CheckDisplayStatus( struct display *d )
{
	if ((d->displayType & d_origin) == dFromFile && !d->stillThere)
		StopDisplay( d );
	else if ((d->displayType & d_lifetime) == dReserve &&
	         d->status == running && d->userSess < 0 && !d->idleTimeout)
		rStopDisplay( d, DS_RESERVE );
}

static void
KickDisplay( struct display *d )
{
	if (d->status == notRunning)
		StartDisplay( d );
	if (d->serverStatus == awaiting && !startingServer)
		StartServer( d );
}

#ifdef HAVE_VTS
static int active_vts;

static int
GetBusyVTs( void )
{
	struct vt_stat vtstat;
	int con;

	if (active_vts == -1) {
		vtstat.v_state = 0;
		if ((con = open( "/dev/console", O_RDONLY )) >= 0) {
			ioctl( con, VT_GETSTATE, &vtstat );
			close( con );
		}
		active_vts = vtstat.v_state;
	}
	return active_vts;
}

static void
AllocateVT( struct display *d )
{
	struct display *cd;
	int i, tvt, volun;

	if ((d->displayType & d_location) == dLocal &&
	    d->status == notRunning && !d->serverVT)
	{
		if (d->reqSrvVT && d->reqSrvVT < 16)
			d->serverVT = d->reqSrvVT;
		else {
			for (i = tvt = 0;;) {
				if (serverVTs[i]) {
					tvt = atoi( serverVTs[i++] );
					volun = 0;
					if (tvt < 0) {
						tvt = -tvt;
						volun = 1;
					}
					if (!tvt || tvt >= 16)
						continue;
				} else {
					if (++tvt >= 16)
						break;
					volun = 1;
				}
				for (cd = displays; cd; cd = cd->next) {
					if (cd->reqSrvVT == tvt && /* protect from lusers */
					    (cd->status != zombie || cd->zstatus != DS_REMOVE))
						goto next;
					if (cd->serverVT == tvt) {
						if (cd->status != zombie || cd->zstatus == DS_REMOTE)
							goto next;
						if (!cd->follower) {
							d->serverVT = -1;
							cd->follower = d;
							return;
						}
					}
				}
				if (!volun || !((1 << tvt) & GetBusyVTs())) {
					d->serverVT = tvt;
					return;
				}
		  next: ;
			}
		}
	}
}
#endif

static void
StartDisplays( void )
{
	ForEachDisplay( CheckDisplayStatus );
#ifdef HAVE_VTS
	active_vts = -1;
	ForEachDisplayRev( AllocateVT );
#endif
	ForEachDisplay( KickDisplay );
	CloseGetter();
}

void
StartDisplay( struct display *d )
{
	if (Stopping) {
		Debug( "stopping display %s because shutdown is scheduled\n", d->name );
		StopDisplay( d );
		return;
	}

#ifdef HAVE_VTS
	if (d->serverVT < 0)
		return;
#endif

	if (!LoadDisplayResources( d )) {
		LogError( "Unable to read configuration for display %s; stopping it.\n",
		          d->name );
		StopDisplay( d );
		return;
	}

	d->status = running;
	if ((d->displayType & d_location) == dLocal) {
		Debug( "StartDisplay %s\n", d->name );
		/* don't bother pinging local displays; we'll
		 * certainly notice when they exit
		 */
		d->pingInterval = 0;
		if (d->authorize) {
			SetLocalAuthorization( d );
			/*
			 * reset the server after writing the authorization information
			 * to make it read the file (for compatibility with old
			 * servers which read auth file only on reset instead of
			 * at first connection)
			 */
			if (d->serverPid != -1 && d->resetForAuth && d->resetSignal)
				kill( d->serverPid, d->resetSignal );
		}
		if (d->serverPid == -1) {
			d->serverStatus = awaiting;
			return;
		}
	} else {
		Debug( "StartDisplay %s, try %d\n", d->name, d->startTries + 1 );
		/* this will only happen when using XDMCP */
		if (d->authorizations)
			SaveServerAuthorizations( d, d->authorizations, d->authNum );
	}
	StartDisplayP2( d );
}

void
StartDisplayP2( struct display *d )
{
	char *cname, *cgname;
	int pid;

	openCtrl( d );
	Debug( "forking session\n" );
	ASPrintf( &cname, "sub-daemon for display %s", d->name );
	ASPrintf( &cgname, "greeter for display %s", d->name );
	pid = GFork( &d->pipe, "master daemon", cname,
	             &d->gpipe, cgname );
	switch (pid) {
	case 0:
		SetTitle( d->name );
		if (debugLevel & DEBUG_WSESS)
			sleep( 100 );
		mstrtalk.pipe = &d->pipe;
		(void)Signal( SIGPIPE, SIG_IGN );
		SetAuthorization( d );
		WaitForServer( d );
		if ((d->displayType & d_location) == dLocal) {
			GSet( &mstrtalk );
			GSendInt( D_XConnOk );
		}
		ManageSession( d );
		/* NOTREACHED */
	case -1:
		closeCtrl( d );
		d->status = notRunning;
		break;
	default:
		Debug( "forked session, pid %d\n", pid );

		/* (void) fcntl (d->pipe.rfd, F_SETFL, O_NONBLOCK); */
		/* (void) fcntl (d->gpipe.rfd, F_SETFL, O_NONBLOCK); */
		RegisterInput( d->pipe.rfd );
		RegisterInput( d->gpipe.rfd );

		d->pid = pid;
		d->hstent->lock = d->hstent->rLogin = d->hstent->goodExit =
			d->hstent->sdRec.how = 0;
		d->lastStart = now;
		break;
	}
}

/*
 * transition from running to zombie, textmode, reserve or deleted
 */

static void
rStopDisplay( struct display *d, int endState )
{
	Debug( "stopping display %s to state %d\n", d->name, endState );
	AbortStartServer( d );
	d->idleTimeout = 0;
	if (d->serverPid != -1 || d->pid != -1) {
		if (d->pid != -1)
			TerminateProcess( d->pid, SIGTERM );
		if (d->serverPid != -1)
			TerminateProcess( d->serverPid, d->termSignal );
		d->status = zombie;
		d->zstatus = endState & 0xff;
		Debug( " zombiefied\n" );
	} else if (endState == DS_TEXTMODE) {
#ifdef HAVE_VTS
		d->status = textMode;
		CheckTTYMode();
	} else if (endState == (DS_TEXTMODE | 0x100)) {
		d->status = textMode;
#else
		SwitchToTty( d );
#endif
	} else if (endState == DS_RESERVE)
		d->status = reserve;
#ifdef XDMCP
	else if (endState == DS_REMOTE)
		StartRemoteLogin( d );
#endif
	else {
#ifndef HAVE_VTS
		SwitchToX( d );
#endif
		RemoveDisplay( d );
	}
}

void
StopDisplay( struct display *d )
{
	rStopDisplay( d, DS_REMOVE );
}

static void
ExitDisplay(
            struct display *d,
            int endState,
            int serverCmd,
            int goodExit )
{
	struct disphist *he;

	if (d->status == raiser) {
		serverCmd = XS_KEEP;
		goodExit = TRUE;
	}

	Debug( "ExitDisplay %s, "
	       "endState = %d, serverCmd = %d, GoodExit = %d\n",
	       d->name, endState, serverCmd, goodExit );

	d->userSess = -1;
	if (d->userName)
		free( d->userName );
	d->userName = 0;
	if (d->sessName)
		free( d->sessName );
	d->sessName = 0;
	he = d->hstent;
	he->lastExit = now;
	he->goodExit = goodExit;
	if (he->sdRec.how) {
		if (he->sdRec.force == SHUT_ASK &&
		    (AnyActiveDisplays() || d->allowShutdown == SHUT_ROOT))
		{
			endState = DS_RESTART;
		} else {
			if (!sdRec.how || sdRec.force != SHUT_FORCE ||
			    !((d->allowNuke == SHUT_NONE && sdRec.uid != he->sdRec.uid) ||
			      (d->allowNuke == SHUT_ROOT && he->sdRec.uid)))
			{
				sdRec = he->sdRec;
				if (now < sdRec.timeout || wouldShutdown())
					endState = DS_REMOVE;
			}
			he->sdRec.how = 0;
		}
	}
	if (d->status == zombie)
		rStopDisplay( d, d->zstatus );
	else {
		if (Stopping) {
			StopDisplay( d );
			return;
		}
		if (endState != DS_RESTART ||
		    (d->displayType & d_origin) != dFromFile)
		{
			rStopDisplay( d, endState );
		} else {
			if (serverCmd == XS_RETRY) {
				if ((d->displayType & d_location) == dLocal) {
					if (he->lastExit - d->lastStart < 120) {
						LogError( "Unable to fire up local display %s;"
						          " disabling.\n", d->name );
						StopDisplay( d );
						return;
					}
				} else {
					if (++d->startTries > d->startAttempts) {
						LogError( "Disabling foreign display %s"
						          " (too many attempts)\n", d->name );
						StopDisplay( d );
						return;
					}
				}
			} else
				d->startTries = 0;
			if (d->serverPid != -1 &&
			    (serverCmd != XS_KEEP || d->terminateServer))
			{
				Debug( "killing X server for %s\n", d->name );
				TerminateProcess( d->serverPid, d->termSignal );
				d->status = phoenix;
			} else
				d->status = notRunning;
		}
	}
}


static int pidFd;
static FILE *pidFilePtr;

static int
StorePid( void )
{
	int oldpid;

	if (pidFile[0] != '\0') {
		pidFd = open( pidFile, O_RDWR );
		if (pidFd == -1 && errno == ENOENT)
			pidFd = open( pidFile, O_RDWR|O_CREAT, 0666 );
		if (pidFd == -1 || !(pidFilePtr = fdopen( pidFd, "r+" ))) {
			LogError( "process-id file %s cannot be opened\n",
			          pidFile );
			return -1;
		}
		if (fscanf( pidFilePtr, "%d\n", &oldpid ) != 1)
			oldpid = -1;
		fseek( pidFilePtr, 0l, 0 );
		if (lockPidFile) {
#ifdef F_SETLK
# ifndef SEEK_SET
#  define SEEK_SET 0
# endif
			struct flock lock_data;
			lock_data.l_type = F_WRLCK;
			lock_data.l_whence = SEEK_SET;
			lock_data.l_start = lock_data.l_len = 0;
			if (fcntl( pidFd, F_SETLK, &lock_data ) == -1) {
				if (errno == EAGAIN)
					return oldpid;
				else
					return -1;
			}
#else
# ifdef LOCK_EX
			if (flock( pidFd, LOCK_EX|LOCK_NB ) == -1) {
				if (errno == EWOULDBLOCK)
					return oldpid;
				else
					return -1;
			}
# else
			if (lockf( pidFd, F_TLOCK, 0 ) == -1) {
				if (errno == EACCES)
					return oldpid;
				else
					return -1;
			}
# endif
#endif
		}
		fprintf( pidFilePtr, "%ld\n", (long)getpid() );
		(void)fflush( pidFilePtr );
		RegisterCloseOnFork( pidFd );
	}
	return 0;
}

#if 0
void
UnlockPidFile( void )
{
	if (lockPidFile)
# ifdef F_SETLK
	{
		struct flock lock_data;
		lock_data.l_type = F_UNLCK;
		lock_data.l_whence = SEEK_SET;
		lock_data.l_start = lock_data.l_len = 0;
		(void)fcntl( pidFd, F_SETLK, &lock_data );
	}
# else
#  ifdef F_ULOCK
		lockf( pidFd, F_ULOCK, 0 );
#  else
		flock( pidFd, LOCK_UN );
#  endif
# endif
	close( pidFd );
	fclose( pidFilePtr );
}
#endif

void
SetTitle( const char *name )
{
#if !defined(HAS_SETPROCTITLE) && !defined(NOXDMTITLE)
	char *p;
	int left;
#endif

	ASPrintf( &prog, "%s: %s", prog, name );
	ReInitErrorLog();
#ifdef HAS_SETPROCTITLE
	setproctitle( "%s", name );
#elif !defined(NOXDMTITLE)
	p = Title;
	left = TitleLen;

	*p++ = '-';
	--left;
	while (*name && left > 0) {
		*p++ = *name++;
		--left;
	}
	while (left > 0) {
		*p++ = '\0';
		--left;
	}
#endif
}
