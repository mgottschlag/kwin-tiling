/*

Copyright 1988, 1998  The Open Group
Copyright 2000-2004 Oswald Buddenhagen <ossi@kde.org>

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
 * subdaemon event loop, etc.
 */

#define NEED_SIGNAL
#include "dm.h"
#include "dm_error.h"

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>

#include <stdio.h>
#include <ctype.h>

struct display *td;
const char *td_setup = "auto";

static void DeleteXloginResources( void );
static void LoadXloginResources( void );
static void SetupDisplay( const char *arg );


static Jmp_buf pingTime;

/* ARGSUSED */
static SIGVAL
catchAlrm( int n ATTR_UNUSED )
{
	Longjmp( pingTime, 1 );
}

static Jmp_buf tenaciousClient;

/* ARGSUSED */
static SIGVAL
waitAbort( int n ATTR_UNUSED )
{
	Longjmp( tenaciousClient, 1 );
}

#if defined(_POSIX_SOURCE) || defined(SYSV) || defined(SVR4)
# define killpg(pgrp, sig) kill(-(pgrp), sig)
#endif

static void
AbortClient( int pid )
{
	int sig = SIGTERM;
	volatile int i;
	int retId;

	for (i = 0; i < 4; i++) {
		if (killpg( pid, sig ) == -1) {
			switch (errno) {
			case EPERM:
				LogError( "Can't kill client\n" );
			case EINVAL:
			case ESRCH:
				return;
			}
		}
		if (!Setjmp( tenaciousClient )) {
			(void)Signal( SIGALRM, waitAbort );
			(void)alarm( (unsigned)10 );
			retId = wait( (waitType *)0 );
			(void)alarm( (unsigned)0 );
			(void)Signal( SIGALRM, SIG_DFL );
			if (retId == pid)
				break;
		} else
			(void)Signal( SIGALRM, SIG_DFL );
		sig = SIGKILL;
	}
}


static char *
conv_auto( int what, const char *prompt ATTR_UNUSED )
{
	switch (what) {
	case GCONV_USER:
		return curuser;
	case GCONV_PASS:
	case GCONV_PASS_ND:
		return curpass;
	default:
		LogError( "Unknown authentication data type requested for autologin.\n" );
		return 0;
	}
}

static int
AutoLogon()
{
	Time_t tdiff;

	tdiff = time( 0 ) - td->hstent->lastExit - td->openDelay;
	Debug( "autoLogon, tdiff = %d, rLogin = %d, goodexit = %d, nuser = %s\n",
	       tdiff, td->hstent->rLogin, td->hstent->goodExit, td->hstent->nuser );
	if (td->hstent->rLogin == 2 ||
	    (td->hstent->rLogin == 1 &&
	     tdiff <= 0 && !td->hstent->goodExit && !td->hstent->lock))
	{
		curuser = td->hstent->nuser;
		td->hstent->nuser = 0;
		curpass = td->hstent->npass;
		td->hstent->npass = 0;
		newdmrc = td->hstent->nargs;
		td->hstent->nargs = 0;
	} else if (*td->autoUser && tdiff > 0) {
		unsigned int lmask;
		Window dummy1, dummy2;
		int dummy3, dummy4, dummy5, dummy6;
		XQueryPointer( dpy, DefaultRootWindow( dpy ),
		               &dummy1, &dummy2, &dummy3, &dummy4, &dummy5, &dummy6,
		               &lmask );
		if (lmask & ShiftMask)
			return 0;
		StrDup( &curuser, td->autoUser );
		StrDup( &curpass, td->autoPass );
	} else
		return 0;
	return 1;
}


static const struct {
  int vcode, echo, ndelay;
} grqs[] = {
	{ V_GET_TEXT, TRUE, FALSE },
	{ V_GET_TEXT, FALSE, FALSE },
	{ V_GET_TEXT, TRUE, FALSE },
	{ V_GET_TEXT, FALSE, FALSE },
	{ V_GET_TEXT, FALSE, TRUE },
	{ V_GET_BINARY, 0, 0 }
};

char *
conv_interact( int what, const char *prompt )
{
	char *ret;
	int tag;

	GSendInt( grqs[what].vcode );
	if (what == GCONV_BINARY) {
		unsigned const char *up = (unsigned const char *)prompt;
		int len = up[3] | (up[2] << 8) | (up[1] << 16) | (up[0] << 24);
		GSendArr( len, prompt );
		GSendInt( FALSE ); /* ndelay */
		return GRecvArr( &len );
	} else {
		GSendStr( prompt );
		GSendInt( grqs[what].echo );
		GSendInt( grqs[what].ndelay );
		ret = GRecvStr();
		if (ret) {
			tag = GRecvInt();
			switch (what) {
			case GCONV_USER:
				/* assert(tag & V_IS_USER); */
				if (curuser)
					free( curuser );
				curuser = ret;
				break;
			case GCONV_PASS:
			case GCONV_PASS_ND:
				/* assert(tag & V_IS_PASSWORD); */
				if (curpass)
					free( curpass );
				curpass = ret;
				break;
			default:
				if (tag & V_IS_USER)
					ReStr( &curuser, ret );
				else if (tag & V_IS_PASSWORD)
					ReStr( &curpass, ret );
				else if (tag & V_IS_NEWPASSWORD)
					ReStr( &newpass, ret );
				else if (tag & V_IS_OLDPASSWORD)
					ReStr( &ret, curpass );
			}
		}
		return ret;
	}
}

static int greeter;
GProc grtproc;
GTalk grttalk;

GTalk mstrtalk; /* make static; see dm.c */

int
CtrlGreeterWait( int wreply )
{
	int i, cmd, type, rootok;
	char *name, *pass, **avptr;
#ifdef XDMCP
	ARRAY8Ptr aptr;
#endif

	if (Setjmp( mstrtalk.errjmp )) {
		CloseGreeter( TRUE );
		SessionExit( EX_UNMANAGE_DPY );
	}

	while (GRecvCmd( &cmd )) {
		switch (cmd)
		{
		case G_Ready:
			Debug( "G_Ready\n" );
			return 0;
		case G_GetCfg:
			/*Debug ("G_GetCfg\n");*/
			type = GRecvInt();
			/*Debug (" index %#x\n", type);*/
			if (type == C_isLocal)
				i = (td->displayType & d_location) == dLocal;
			else if (type == C_hasConsole)
#ifdef HAVE_VTS
				i = *consoleTTYs != 0;
#else
				i = td->console != 0;
#endif
			else if (type == C_isAuthorized)
				i = td->authorizations != 0;
			else
				goto normal;
			GSendInt( GE_Ok );
			/*Debug (" -> bool %d\n", i);*/
			GSendInt( i );
			break;
		  normal:
			if (!(avptr = FindCfgEnt( td, type ))) {
				/*Debug (" -> not found\n");*/
				GSendInt( GE_NoEnt );
				break;
			}
			switch (type & C_TYPE_MASK) {
			default:
				/*Debug (" -> unknown type\n");*/
				GSendInt( GE_BadType );
				break;
			case C_TYPE_INT:
			case C_TYPE_STR:
			case C_TYPE_ARGV:
#ifdef XDMCP
			case C_TYPE_ARR:
#endif
				GSendInt( GE_Ok );
				switch (type & C_TYPE_MASK) {
				case C_TYPE_INT:
					/*Debug (" -> int %#x (%d)\n", *(int *)avptr, *(int *)avptr);*/
					GSendInt( *(long *)avptr );
					break;
				case C_TYPE_STR:
					/*Debug (" -> string %\"s\n", *avptr);*/
					GSendStr( *avptr );
					break;
				case C_TYPE_ARGV:
					/*Debug (" -> sending argv %\"[{s\n", *(char ***)avptr);*/
					GSendArgv( *(char ***)avptr );
					break;
#ifdef XDMCP
				case C_TYPE_ARR:
					aptr = *(ARRAY8Ptr *)avptr;
					/*Debug (" -> sending array %02[*:hhx\n",
					         aptr->length, aptr->data);*/
					GSendArr( aptr->length, (char *)aptr->data );
					break;
#endif
				}
				break;
			}
			break;
		case G_ReadDmrc:
			Debug( "G_ReadDmrc\n" );
			name = GRecvStr();
			Debug( " user %\"s\n", name );
			if (StrCmp( dmrcuser, name )) {
				if (curdmrc) { free( curdmrc ); curdmrc = 0; }
				if (dmrcuser)
					free( dmrcuser );
				dmrcuser = name;
				i = ReadDmrc();
				Debug( " -> status %d\n", i );
				GSendInt( i );
				Debug( " => %\"s\n", curdmrc );
			} else {
				if (name)
					free( name );
				Debug( " -> status " stringify( GE_Ok ) "\n" );
				GSendInt( GE_Ok );
				Debug( " => keeping old\n" );
			}
			break;
		case G_GetDmrc:
			Debug( "G_GetDmrc\n" );
			name = GRecvStr();
			Debug( " key %\"s\n", name );
			pass = iniEntry( curdmrc, "Desktop", name, 0 );
			Debug( " -> %\"s\n", pass );
			GSendStr( pass );
			if (pass)
				free( pass );
			free( name );
			break;
/*		case G_ResetDmrc:
			Debug ("G_ResetDmrc\n");
			if (newdmrc) { free (newdmrc); newdmrc = 0; }
			break; */
		case G_PutDmrc:
			Debug( "G_PutDmrc\n" );
			name = GRecvStr();
			Debug( " key %\"s\n", name );
			pass = GRecvStr();
			Debug( " value %\"s\n", pass );
			newdmrc = iniEntry( newdmrc, "Desktop", name, pass );
			free( pass );
			free( name );
			break;
		case G_VerifyRootOK:
			Debug( "G_VerifyRootOK\n" );
			rootok = TRUE;
			goto doverify;
		case G_Verify:
			Debug( "G_Verify\n" );
			rootok = FALSE;
		  doverify:
			if (curuser) { free( curuser ); curuser = 0; }
			if (curpass) { free( curpass ); curpass = 0; }
			if (curtype) free( curtype );
			curtype = GRecvStr();
			Debug( " type %\"s\n", curtype );
			if (Verify( conv_interact, rootok )) {
				Debug( " -> return success\n" );
				GSendInt( V_OK );
			} else
				Debug( " -> failure returned\n" );
			break;
		case G_SetupDpy:
			Debug( "G_SetupDpy\n" );
			SetupDisplay( 0 );
			td_setup = 0;
			GSendInt( 0 );
			break;
		default:
			return cmd;
		}
		if (!wreply)
			return -1;
	}
	Debug( "lost connection to greeter\n" );
	return -2;
}

void
OpenGreeter()
{
	char *name, **env;
	static Time_t lastStart;
	int cmd;
	Cursor xcursor;

	GSet( &grttalk );
	if (greeter)
		return;
	if (time( 0 ) < lastStart + 10) /* XXX should use some readiness indicator instead */
		SessionExit( EX_UNMANAGE_DPY );
	greeter = 1;
	ASPrintf( &name, "greeter for display %s", td->name );
	Debug( "starting %s\n", name );

	/* Hourglass cursor */
	if ((xcursor = XCreateFontCursor( dpy, XC_watch ))) {
		XDefineCursor( dpy, DefaultRootWindow( dpy ), xcursor );
		XFreeCursor( dpy, xcursor );
	}
	XFlush( dpy );

	/* Load system default Resources (if any) */
	LoadXloginResources();

	grttalk.pipe = &grtproc.pipe;
	env = systemEnv( (char *)0 );
	if (GOpen( &grtproc, (char **)0, "_greet", env, name, &td->gpipe ))
		SessionExit( EX_UNMANAGE_DPY );
	freeStrArr( env );
	if ((cmd = CtrlGreeterWait( TRUE ))) {
		if (cmd != -2)
			LogError( "Received unknown or unexpected command %d from greeter\n", cmd );
		CloseGreeter( TRUE );
		SessionExit( EX_UNMANAGE_DPY );
	}
	Debug( "%s ready\n", name );
	time( &lastStart );
}

int
CloseGreeter( int force )
{
	int ret;

	if (!greeter)
		return EX_NORMAL;
	greeter = 0;
	ret = GClose (&grtproc, 0, force);
	Debug( "greeter for %s stopped\n", td->name );
	if (WaitCode( ret ) > EX_NORMAL && WaitCode( ret ) <= EX_MAX) {
		Debug( "greeter-initiated session exit, code %d\n", WaitCode( ret ) );
		SessionExit( WaitCode( ret ) );
	}
	return ret;
}

void
PrepErrorGreet()
{
	if (!greeter) {
		OpenGreeter();
		GSendInt( G_ErrorGreet );
		GSendStr( curuser );
	}
}

static Jmp_buf idleTOJmp;

/* ARGSUSED */
static SIGVAL
IdleTOJmp( int n ATTR_UNUSED )
{
	Longjmp( idleTOJmp, 1 );
}


static Jmp_buf abortSession;

/* ARGSUSED */
static SIGVAL
catchTerm( int n ATTR_UNUSED )
{
	Longjmp( abortSession, EX_AL_RESERVER_DPY );
}

/*
 * We need our own error handlers because we can't be sure what exit code Xlib
 * will use, and our Xlib does exit(1) which matches EX_REMANAGE_DPY, which
 * can cause a race condition leaving the display wedged.  We need to use
 * EX_RESERVER_DPY for IO errors, to ensure that the manager waits for the
 * server to terminate.  For other X errors, we should give up.
 */

/*ARGSUSED*/
static int
IOErrorHandler( Display *dspl ATTR_UNUSED )
{
	LogError( "Fatal X server IO error: %m\n" );
	/* The only X interaction during the session are pings, and those
	   have an own IOErrorHandler -> not EX_AL_RESERVER_DPY */
	Longjmp( abortSession, EX_RESERVER_DPY );
	/*NOTREACHED*/
	return 0;
}

/*ARGSUSED*/
static int
ErrorHandler( Display *dspl ATTR_UNUSED, XErrorEvent *event )
{
	LogError( "X error\n" );
	if (event->error_code == BadImplementation)
		Longjmp( abortSession, EX_UNMANAGE_DPY );
	return 0;
}

void
ManageSession( struct display *d )
{
	int ex, cmd;
	volatile int clientPid = 0;

	td = d;
	Debug( "ManageSession %s\n", d->name );
	if ((ex = Setjmp( abortSession ))) {
		CloseGreeter( TRUE );
		if (clientPid)
			AbortClient( clientPid );
		SessionExit( ex );
		/* NOTREACHED */
	}
	(void)XSetIOErrorHandler( IOErrorHandler );
	(void)XSetErrorHandler( ErrorHandler );
	(void)Signal( SIGTERM, catchTerm );

	(void)Signal( SIGHUP, SIG_IGN );

	if (Setjmp( grttalk.errjmp ))
		Longjmp( abortSession, EX_RESERVER_DPY ); /* EX_RETRY_ONCE */

#ifdef XDMCP
	if (d->useChooser)
		DoChoose();
		/* NOTREACHED */
#endif

	if (d->hstent->sdRec.how) {
		OpenGreeter();
		GSendInt( G_ConfShutdown );
		GSendInt( d->hstent->sdRec.how );
		GSendInt( d->hstent->sdRec.uid );
		GSendStr( d->hstent->boRec.name );
		if (CtrlGreeterWait( TRUE ) != G_Ready) {
			LogError( "Received unknown command %d from greeter\n", cmd );
			CloseGreeter( TRUE );
		}
	}

	if (AutoLogon()) {
		if (!StrDup( &curtype, "classic" ) || !Verify( conv_auto, FALSE ))
			goto gcont;
		if (greeter)
			GSendInt( V_OK );
	} else {
	  regreet:
		OpenGreeter();
		if (Setjmp( idleTOJmp )) {
			CloseGreeter( TRUE );
			SessionExit( EX_NORMAL );
		}
		Signal( SIGALRM, IdleTOJmp );
		alarm( td->idleTimeout );
#ifdef XDMCP
		if (((d->displayType & d_location) == dLocal) &&
		    d->loginMode >= LOGIN_DEFAULT_REMOTE)
			goto choose;
#endif
		for (;;) {
			GSendInt( G_Greet );
		  gcont:
			cmd = CtrlGreeterWait( TRUE );
#ifdef XDMCP
		  recmd:
			if (cmd == G_DChoose) {
			  choose:
				cmd = DoChoose();
				goto recmd;
			}
			if (cmd == G_DGreet)
				continue;
#endif
			alarm( 0 );
			if (cmd == G_Ready)
				break;
			if (cmd == -2)
				CloseGreeter( FALSE );
			else {
				LogError( "Received unknown command %d from greeter\n", cmd );
				CloseGreeter( TRUE );
			}
			goto regreet;
		}
	}

	if (CloseGreeter( FALSE ) != EX_NORMAL)
		goto regreet;

	DeleteXloginResources();

	if (td_setup)
		SetupDisplay( td_setup );

	if (!(clientPid = StartClient())) {
		LogError( "Client start failed\n" );
		SessionExit( EX_NORMAL ); /* XXX maybe EX_REMANAGE_DPY? -- enable in dm.c! */
	}
	Debug( "client Started\n" );

	/*
	 * Wait for session to end,
	 */
	for (;;) {
		if (!Setjmp( pingTime )) {
			(void)Signal( SIGALRM, catchAlrm );
			(void)alarm( d->pingInterval * 60 ); /* may be 0 */
			(void)Wait4( clientPid );
			(void)alarm( 0 );
			break;
		} else {
			(void)alarm( 0 );
			if (!PingServer( d ))
				catchTerm( SIGTERM );
		}
	}
	/*
	 * Sometimes the Xsession somehow manages to exit before
	 * a server crash is noticed - so we sleep a bit and wait
	 * for being killed.
	 */
	if (!PingServer( d )) {
		Debug( "X server dead upon session exit.\n" );
		if ((d->displayType & d_location) == dLocal)
			sleep( 10 );
		SessionExit( EX_AL_RESERVER_DPY );
	}
	SessionExit( EX_NORMAL ); /* XXX maybe EX_REMANAGE_DPY? -- enable in dm.c! */
}

static int xResLoaded;

void
LoadXloginResources()
{
	char **args;
	char **env;

	if (!xResLoaded && td->resources[0] && access( td->resources, 4 ) == 0) {
		env = systemEnv( (char *)0 );
		if ((args = parseArgs( (char **)0, td->xrdb )) &&
		    (args = addStrArr( args, td->resources, -1 )))
		{
			Debug( "loading resource file: %s\n", td->resources );
			(void)runAndWait( args, env );
			freeStrArr( args );
		}
		freeStrArr( env );
		xResLoaded = TRUE;
	}
}

void
SetupDisplay( const char *arg )
{
	char **env;

	env = systemEnv( (char *)0 );
	(void)source( env, td->setup, arg );
	freeStrArr( env );
}

void
DeleteXloginResources()
{
	int i;
	Atom prop;

	if (!xResLoaded)
		return;
	xResLoaded = FALSE;
	prop = XInternAtom( dpy, "SCREEN_RESOURCES", True );
	XDeleteProperty( dpy, RootWindow( dpy, 0 ), XA_RESOURCE_MANAGER );
	if (prop)
		for (i = ScreenCount(dpy); --i >= 0; )
			XDeleteProperty( dpy, RootWindow( dpy, i ), prop );
	XSync( dpy, 0 );
}


int
source( char **env, const char *file, const char *arg )
{
	char **args;
	int ret;

	if (file && file[0]) {
		Debug( "source %s\n", file );
		if (!(args = parseArgs( (char **)0, file )))
			return waitCompose( 0,0,3 );
		if (arg && !(args = addStrArr( args, arg, -1 )))
			return waitCompose( 0,0,3 );
		ret = runAndWait( args, env );
		freeStrArr( args );
		return ret;
	}
	return 0;
}

char **
inheritEnv( char **env, const char **what )
{
	char *value;

	for (; *what; ++what)
		if ((value = getenv( *what )))
			env = setEnv( env, *what, value );
	return env;
}

char **
baseEnv( const char *user )
{
	char **env;

	env = 0;

#ifdef AIXV3
	/* we need the tags SYSENVIRON: and USRENVIRON: in the call to setpenv() */
	env = setEnv( env, "SYSENVIRON:", 0 );
#endif

	if (user) {
		env = setEnv( env, "USER", user );
#ifdef AIXV3
		env = setEnv( env, "LOGIN", user );
#endif
		env = setEnv( env, "LOGNAME", user );
	}

#ifdef AIXV3
	env = setEnv( env, "USRENVIRON:", 0 );
#endif

	env = inheritEnv( env, (const char **)exportList );

	env = setEnv( env, "DISPLAY",
	              memcmp( td->name, "localhost:", 10 ) ?
	              td->name : td->name + 9 );

	if (td->ctrl.path)
		env = setEnv( env, "DM_CONTROL", fifoDir );

	return env;
}

char **
systemEnv( const char *user )
{
	char **env;

	env = baseEnv( user );
	if (td->authFile)
		env = setEnv( env, "XAUTHORITY", td->authFile );
	env = setEnv( env, "PATH", td->systemPath );
	env = setEnv( env, "SHELL", td->systemShell );
	return env;
}
