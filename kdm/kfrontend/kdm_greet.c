/*

KDE Greeter module for xdm

Copyright (C) 2001-2003 Oswald Buddenhagen <ossi@kde.org>

This file contains code from the old xdm core,
Copyright 1988, 1998  Keith Packard, MIT X Consortium/The Open Group

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#include <config.h>

#include "kdm_greet.h"
#include "kdmconfig.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <signal.h>
#include <setjmp.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#ifdef _POSIX_PRIORITY_SCHEDULING
# include <sched.h>
#endif

# include <X11/Xlib.h>
#if defined(HAVE_XTEST) || defined(HAVE_XKB)
# include <X11/keysym.h>
#endif

#ifdef HAVE_XTEST
# include <X11/extensions/XTest.h>
#endif

#ifdef HAVE_XKB
# include <X11/XKBlib.h>
#endif

extern void LogOutOfMem( void );

static void *
Realloc( void *ptr, size_t size )
{
	void *ret;

	if (!(ret = realloc( ptr, size )) && size)
		LogOutOfMem();
	return ret;
}

#define PRINT_QUOTES
#define PRINT_ARRAYS
#define LOG_NAME "kdm_greet"
#define LOG_DEBUG_MASK DEBUG_GREET
#define LOG_PANIC_EXIT 1
#define STATIC
#include <printf.c>

static void
GDebug( const char *fmt, ... )
{
	va_list args;

	if (debugLevel & DEBUG_HLPCON) {
		va_start( args, fmt );
		Logger( DM_DEBUG, fmt, args );
		va_end( args );
	}
}


char *dname;

int rfd;
static int wfd, mrfd, mwfd, srfd, swfd;
static const char *who;

void
GSet( int master )
{
	if (master)
		rfd = mrfd, wfd = mwfd, who = "core (master)";
	else
		rfd = srfd, wfd = swfd, who = "core";

}

static int
Reader( void *buf, int count )
{
	int ret, rlen;

	for (rlen = 0; rlen < count; ) {
	  dord:
		ret = read( rfd, (void *)((char *)buf + rlen), count - rlen );
		if (ret < 0) {
			if (errno == EINTR)
				goto dord;
			if (errno == EAGAIN)
				break;
			return -1;
		}
		if (!ret)
			break;
		rlen += ret;
	}
	return rlen;
}

static void
GRead( void *buf, int count )
{
	if (Reader( buf, count ) != count)
		LogPanic( "Can't read from %s\n", who );
}

static void
GWrite( const void *buf, int count )
{
	if (write( wfd, buf, count ) != count)
		LogPanic( "Can't write to %s\n", who );
#ifdef _POSIX_PRIORITY_SCHEDULING
	if ((debugLevel & DEBUG_HLPCON))
		sched_yield();
#endif
}

void
GSendInt( int val )
{
	GDebug( "Sending int %d (%#x) to %s\n", val, val, who );
	GWrite( &val, sizeof(val) );
}

void
GSendStr( const char *buf )
{
	int len = buf ? strlen( buf ) + 1 : 0;
	GDebug( "Sending string %'s to %s\n", buf, who );
	GWrite( &len, sizeof(len) );
	GWrite( buf, len );
}

/*
static void
GSendNStr( const char *buf, int len )
{
	int tlen = len + 1;
	GDebug( "Sending string %'.*s to %s\n", len, buf, who );
	GWrite( &tlen, sizeof(tlen) );
	GWrite( buf, len );
	GWrite( "", 1 );
}
*/

void
GSendArr( int len, const char *buf )
{
	GDebug( "Sending array %02[:*hhx to %s\n", len, buf, who );
	GWrite( &len, sizeof(len) );
	GWrite( buf, len );
}

int
GRecvInt()
{
	int val;

	GDebug( "Receiving int from %s ...\n", who );
	GRead( &val, sizeof(val) );
	GDebug( " -> %d (%#x)\n", val, val );
	return val;
}

static char *
iGRecvArr( int *rlen )
{
	int len;
	char *buf;

	GRead( &len, sizeof(len) );
	*rlen = len;
	GDebug( " -> %d bytes\n", len );
	if (!len)
		return (char *)0;
	if (!(buf = malloc( len )))
		LogPanic( "No memory for read buffer\n" );
	GRead( buf, len );
	return buf;
}

char *
GRecvStr()
{
	int len;
	char *buf;

	GDebug( "Receiving string from %s ...\n", who );
	buf = iGRecvArr( &len );
	GDebug( " -> %'.*s\n", len, buf );
	return buf;
}

char **
GRecvStrArr( int *rnum )
{
	int num;
	char **argv, **cargv;

	GDebug( "Receiving string array from %s ...\n", who );
	GRead( &num, sizeof(num) );
	GDebug( " -> %d strings\n", num );
	if (rnum)
		*rnum = num;
	if (!num)
		return (char **)0;
	if (!(argv = malloc( num * sizeof(char *))))
		LogPanic( "No memory for read buffer\n" );
	for (cargv = argv; --num >= 0; cargv++)
		*cargv = GRecvStr();
	return argv;
}

char *
GRecvArr( int *num )
{
	char *arr;

	GDebug( "Receiving array from %s ...\n", who );
	GRead( num, sizeof(*num) );
	GDebug( " -> %d bytes\n", *num );
	if (!*num)
		return (char *)0;
	if (!(arr = malloc( *num )))
		LogPanic( "No memory for read buffer\n" );
	GRead( arr, *num );
	GDebug( " -> %02[*hhx\n", *num, arr );
	return arr;
}

static void
ReqCfg( int id )
{
	GSendInt( G_GetCfg );
	GSendInt( id );
	switch (GRecvInt()) {
	case GE_NoEnt:
		LogPanic( "Config value %#x not available\n", id );
	case GE_BadType:
		LogPanic( "Core does not know type of config value %#x\n", id );
	}
}

int
GetCfgInt( int id )
{
	ReqCfg( id );
	return GRecvInt();
}

char *
GetCfgStr( int id )
{
	ReqCfg( id );
	return GRecvStr();
}

char **
GetCfgStrArr( int id, int *len )
{
	ReqCfg( id );
	return GRecvStrArr( len );
}

static void
disposeSession( dpySpec *sess )
{
	free( sess->display );
	free( sess->from );
	if (sess->user)
		free( sess->user );
	if (sess->session)
		free( sess->session );
}

dpySpec *
fetchSessions( int flags )
{
	dpySpec *sess, *sessions = 0, tsess;

	GSet( 1 );
	GSendInt( G_List );
	GSendInt( flags );
  next:
	while ((tsess.display = GRecvStr())) {
		tsess.from = GRecvStr();
#ifdef HAVE_VTS
		tsess.vt = GRecvInt();
#endif
		tsess.user = GRecvStr();
		tsess.session = GRecvStr();
		tsess.flags = GRecvInt();
		if ((tsess.flags & isTTY) && *tsess.from)
			for (sess = sessions; sess; sess = sess->next)
				if (sess->user && !strcmp( sess->user, tsess.user ) &&
				    !strcmp( sess->from, tsess.from ))
				{
					sess->count++;
					disposeSession( &tsess );
					goto next;
				}
		if (!(sess = malloc( sizeof(*sess) )))
			LogPanic( "Out of memory\n" );
		tsess.count = 1;
		tsess.next = sessions;
		*sess = tsess;
		sessions = sess;
	}
	GSet( 0 );
	return sessions;
}

void
disposeSessions( dpySpec *sess )
{
	while (sess) {
		dpySpec *nsess = sess->next;
		disposeSession( sess );
		free( sess );
		sess = nsess;
	}
}

void
freeStrArr( char **arr )
{
	char **tarr;

	if (arr) {
		for (tarr = arr; *tarr; tarr++)
			free( *tarr );
		free( arr );
	}
}


static int
ignoreErrors( Display *dpy ATTR_UNUSED, XErrorEvent *event ATTR_UNUSED )
{
	Debug( "ignoring X error\n" );
	return 0;
}

/*
 * this is mostly bogus -- but quite useful.  I wish the protocol
 * had some way of enumerating and identifying clients, that way
 * this code wouldn't have to be this kludgy.
 */

static void
killWindows( Display *dpy, Window window )
{
	Window root, parent, *children;
	unsigned child, nchildren = 0;

	while (XQueryTree( dpy, window, &root, &parent, &children, &nchildren )
	       && nchildren > 0)
	{
		for (child = 0; child < nchildren; child++) {
			Debug( "XKillClient 0x%lx\n", (unsigned long)children[child] );
			XKillClient( dpy, children[child] );
		}
		XFree( (char *)children );
	}
}

static jmp_buf resetJmp;

static void
abortReset( int n ATTR_UNUSED )
{
	longjmp (resetJmp, 1);
}

/*
 * this display connection better not have any windows...
 */

static void
pseudoReset( Display *dpy )
{
	int screen;

	if (setjmp( resetJmp )) {
		LogError( "pseudoReset timeout\n" );
	} else {
		(void)signal( SIGALRM, abortReset );
		(void)alarm( 30 );
		XSetErrorHandler( ignoreErrors );
		for (screen = 0; screen < ScreenCount( dpy ); screen++) {
			Debug( "pseudoReset screen %d\n", screen );
			killWindows( dpy, RootWindow( dpy, screen ) );
		}
		Debug( "before XSync\n" );
		XSync( dpy, False );
		(void)alarm( 0 );
	}
	signal( SIGALRM, SIG_DFL );
	XSetErrorHandler( (XErrorHandler)0 );
	Debug( "pseudoReset done\n" );
}


static jmp_buf syncJump;

static void
syncTimeout( int n ATTR_UNUSED )
{
	longjmp( syncJump, 1 );
}

void
SecureDisplay( Display *dpy )
{
	Debug( "SecureDisplay %s\n", dname );
	(void)signal( SIGALRM, syncTimeout );
	if (setjmp( syncJump )) {
		LogError( "Display %s could not be secured\n", dname );
		exit( EX_RESERVER_DPY );
	}
	(void)alarm( (unsigned)_grabTimeout );
	Debug( "Before XGrabServer %s\n", dname );
	XGrabServer( dpy );
	Debug( "XGrabServer succeeded %s\n", dname );
	if (XGrabKeyboard( dpy, DefaultRootWindow( dpy ), True, GrabModeAsync,
	                   GrabModeAsync, CurrentTime ) != GrabSuccess)
	{
		(void)alarm( 0 );
		(void)signal( SIGALRM, SIG_DFL );
		LogError( "Keyboard on display %s could not be secured\n", dname );
		sleep( 10 );
		exit( EX_RESERVER_DPY );
	}
	(void)alarm( 0 );
	(void)signal( SIGALRM, SIG_DFL );
	pseudoReset( dpy );
	if (!_grabServer)
	{
		XUngrabServer( dpy );
		XSync( dpy, 0 );
	}
	Debug( "done secure %s\n", dname );
#ifdef HAVE_XKBSETPERCLIENTCONTROLS
	/*
	 * Activate the correct mapping for modifiers in XKB extension as
	 * grabbed keyboard has its own mapping by default
	 */
	{
		int opcode, evbase, errbase, majret, minret;
		unsigned int value = XkbPCF_GrabsUseXKBStateMask;
		if (XkbQueryExtension( dpy, &opcode, &evbase,
		                       &errbase, &majret, &minret ))
			XkbSetPerClientControls( dpy, value, &value );
	}
#endif
}

void
UnsecureDisplay( Display *dpy )
{
	Debug( "Unsecure display %s\n", dname );
	if (_grabServer) {
		XUngrabServer( dpy );
		XSync( dpy, 0 );
	}
}

static jmp_buf pingTime;

static int
PingLostIOErr( Display *dpy ATTR_UNUSED )
{
	longjmp( pingTime, 1 );
}

static void
PingLostSig( int n ATTR_UNUSED )
{
	longjmp( pingTime, 1 );
}

int
PingServer( Display *dpy )
{
	int (*oldError)( Display * );
	void (*oldSig)( int );
	int oldAlarm;

	oldError = XSetIOErrorHandler( PingLostIOErr );
	oldAlarm = alarm( 0 );
	oldSig = signal( SIGALRM, PingLostSig );
	(void)alarm( _pingTimeout * 60 );
	if (!setjmp( pingTime )) {
		Debug( "Ping server\n" );
		XSync( dpy, 0 );
	} else {
		Debug( "Server dead\n" );
		(void)alarm( 0 );
		(void)signal( SIGALRM, SIG_DFL );
		XSetIOErrorHandler( oldError );
		return 0;
	}
	(void)alarm( 0 );
	(void)signal( SIGALRM, oldSig );
	(void)alarm( oldAlarm );
	Debug( "Server alive\n" );
	XSetIOErrorHandler( oldError );
	return 1;
}

/*
 * Modifier changing code based on kdebase/kxkb/kcmmisc.cpp
 *
 * XTest part: Copyright (C) 2000-2001 Lubos Lunak <l.lunak@kde.org>
 * XKB part:   Copyright (C) 2001-2002 Oswald Buddenhagen <ossi@kde.org>
 *
 */

#ifdef HAVE_XKB
static int
xkb_init( Display *dpy )
{
	int xkb_opcode, xkb_event, xkb_error;
	int xkb_lmaj = XkbMajorVersion;
	int xkb_lmin = XkbMinorVersion;
	return XkbLibraryVersion( &xkb_lmaj, &xkb_lmin ) &&
	       XkbQueryExtension( dpy, &xkb_opcode, &xkb_event,
	                          &xkb_error, &xkb_lmaj, &xkb_lmin );
}

static unsigned int
xkb_modifier_mask_work( XkbDescPtr xkb, const char *name )
{
	int i;

	if (!xkb->names)
		return 0;
	for (i = 0; i < XkbNumVirtualMods; i++) {
		char *modStr = XGetAtomName( xkb->dpy, xkb->names->vmods[i] );
		if (modStr != NULL && strcmp( name, modStr ) == 0) {
			unsigned int mask;
			XkbVirtualModsToReal( xkb, 1 << i, &mask );
			return mask;
		}
	}
	return 0;
}

static unsigned int
xkb_modifier_mask( Display *dpy, const char *name )
{
	XkbDescPtr xkb;

	if ((xkb = XkbGetKeyboard( dpy, XkbAllComponentsMask, XkbUseCoreKbd ))) {
		unsigned int mask = xkb_modifier_mask_work( xkb, name );
		XkbFreeKeyboard( xkb, 0, True );
		return mask;
	}
	return 0;
}

static int
xkb_get_modifier_state( Display *dpy, const char *name )
{
	unsigned int mask;
	XkbStateRec state;

	if (!(mask = xkb_modifier_mask( dpy, name )))
		return 0;
	XkbGetState( dpy, XkbUseCoreKbd, &state );
	return (mask & state.locked_mods) != 0;
}

static int
xkb_set_modifier( Display *dpy, const char *name, int sts )
{
	unsigned int mask;

	if (!(mask = xkb_modifier_mask( dpy, name )))
		return 0;
	XkbLockModifiers( dpy, XkbUseCoreKbd, mask, sts ? mask : 0 );
	return 1;
}
#endif /* HAVE_XKB */

#ifdef HAVE_XTEST
static int
xtest_get_modifier_state( Display *dpy, int key )
{
	XModifierKeymap *map;
	KeyCode modifier_keycode;
	unsigned int i, mask;
	Window dummy1, dummy2;
	int dummy3, dummy4, dummy5, dummy6;

	if ((modifier_keycode = XKeysymToKeycode( dpy, key )) == NoSymbol)
		return 0;
	map = XGetModifierMapping( dpy );
	for (i = 0; i < 8; ++i)
		if (map->modifiermap[map->max_keypermod * i] == modifier_keycode) {
			XFreeModifiermap( map );
			XQueryPointer( dpy, DefaultRootWindow( dpy ),
			               &dummy1, &dummy2, &dummy3, &dummy4, &dummy5, &dummy6,
			               &mask );
			return (mask & (1 << i)) != 0;
		}
	XFreeModifiermap( map );
	return 0;
}

static void
xtest_fake_keypress( Display *dpy, int key )
{
	XTestFakeKeyEvent( dpy, XKeysymToKeycode( dpy, key ), True, CurrentTime );
	XTestFakeKeyEvent( dpy, XKeysymToKeycode( dpy, key ), False, CurrentTime );
}
#endif /* HAVE_XTEST */

#ifdef HAVE_XKB
static int havexkb;
#endif
static int nummodified, oldnumstate, newnumstate;
static Display *dpy;

void
setup_modifiers( Display *mdpy, int numlock )
{
	if (numlock == 2)
		return;
	newnumstate = numlock;
	nummodified = 1;
	dpy = mdpy;
#ifdef HAVE_XKB
	if (xkb_init( mdpy )) {
		havexkb = 1;
		oldnumstate = xkb_get_modifier_state( mdpy, "NumLock" );
		xkb_set_modifier( mdpy, "NumLock", numlock );
		return;
	}
#endif
#ifdef HAVE_XTEST
	oldnumstate = xtest_get_modifier_state( mdpy, XK_Num_Lock );
	if (oldnumstate != numlock)
		xtest_fake_keypress( mdpy, XK_Num_Lock );
#endif
}

void
restore_modifiers( void )
{
#ifdef HAVE_XTEST
	int numstat;
#endif

	if (!nummodified)
		return;
#ifdef HAVE_XKB
	if (havexkb) {
		if (xkb_get_modifier_state( dpy, "NumLock" ) == newnumstate)
			xkb_set_modifier( dpy, "NumLock", oldnumstate );
		return;
	}
#endif
#ifdef HAVE_XTEST
	numstat = xtest_get_modifier_state( dpy, XK_Num_Lock );
	if (numstat == newnumstate && newnumstate != oldnumstate)
		xtest_fake_keypress( dpy, XK_Num_Lock );
#endif
}

void
setCursor( Display *mdpy, int window, int shape )
{
	Cursor xcursor;

	if ((xcursor = XCreateFontCursor( mdpy, shape ))) {
		XDefineCursor( mdpy, window, xcursor );
		XFreeCursor( mdpy, xcursor );
		XFlush( mdpy );
	}
}

static void
sigterm( int n ATTR_UNUSED )
{
	exit( EX_NORMAL );
}

static char *savhome;

static void
cleanup( void )
{
	char buf[128];

	if (strcmp( savhome, getenv( "HOME" ) ) || memcmp( savhome, "/tmp/", 5 ))
		LogError( "Internal error: memory corruption detected\n" ); /* no panic: recursion */
	else {
		sprintf( buf, "rm -rf %s", savhome );
		system( buf );
	}
}

extern void kg_main( const char *argv0 );

int
main( int argc ATTR_UNUSED, char **argv )
{
	char *ci;
	int i;
	char qtrc[40];

	if (!(ci = getenv( "CONINFO" ))) {
		fprintf( stderr, "This program is part of kdm and should not be run manually.\n" );
		return 1;
	}
	if (sscanf( ci, "%d %d %d %d", &srfd, &swfd, &mrfd, &mwfd ) != 4)
		return 1;
	fcntl( srfd, F_SETFD, FD_CLOEXEC );
	fcntl( swfd, F_SETFD, FD_CLOEXEC );
	fcntl( mrfd, F_SETFD, FD_CLOEXEC );
	fcntl( mwfd, F_SETFD, FD_CLOEXEC );
	GSet( 0 );

	InitLog();

	if ((debugLevel = GRecvInt()) & DEBUG_WGREET)
		sleep( 100 );

	signal( SIGTERM, sigterm );

	dname = getenv( "DISPLAY" );

	init_config();

	/* for QSettings */
	srand( time( 0 ) );
	for (i = 0; i < 10000; i++) {
		sprintf( qtrc, "/tmp/%010d", rand() );
		if (!mkdir( qtrc, 0700 ))
			goto okay;
	}
	LogPanic( "Cannot create $HOME\n" );
  okay:
	if (setenv( "HOME", qtrc, 1 ))
		LogPanic( "Cannot set $HOME\n" );
	if (!(savhome = strdup( qtrc )))
		LogPanic( "Cannot save $HOME\n" );
	atexit( cleanup );

	setenv( "LC_ALL", _language, 1 );

	kg_main( argv[0] );

	return EX_NORMAL;
}
