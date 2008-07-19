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

#include <config-workspace.h>
#include <config-kdm.h>
#include <config-X11.h>

#include "kdm_greet.h"
#include "kdmconfig.h"

#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <signal.h>
#include <setjmp.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>
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
#include <X11/XKBlib.h>
#endif

extern void logOutOfMem( void );

static void *
Realloc( void *ptr, size_t size )
{
	void *ret;

	if (!(ret = realloc( ptr, size )) && size)
		logOutOfMem();
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
gDebug( const char *fmt, ... )
{
	va_list args;

	if (debugLevel & DEBUG_HLPCON) {
		va_start( args, fmt );
		logger( DM_DEBUG, fmt, args );
		va_end( args );
	}
}


char *dname;

int rfd, mrfd, mwfd, srfd, swfd;
static int wfd;
static const char *who;

void
gSet( int master )
{
	if (master)
		rfd = mrfd, wfd = mwfd, who = "core (master)";
	else
		rfd = srfd, wfd = swfd, who = "core";

}

static int
reader( void *buf, int count )
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
gRead( void *buf, int count )
{
	if (reader( buf, count ) != count)
		logPanic( "Cannot read from %s\n", who );
}

static void
gWrite( const void *buf, int count )
{
	if (write( wfd, buf, count ) != count)
		logPanic( "Cannot write to %s\n", who );
#ifdef _POSIX_PRIORITY_SCHEDULING
	if ((debugLevel & DEBUG_HLPCON))
		sched_yield();
#endif
}

void
gSendInt( int val )
{
	gDebug( "Sending int %d (%#x) to %s\n", val, val, who );
	gWrite( &val, sizeof(val) );
}

void
gSendStr( const char *buf )
{
	int len = buf ? strlen( buf ) + 1 : 0;
	gDebug( "Sending string %'s to %s\n", buf, who );
	gWrite( &len, sizeof(len) );
	gWrite( buf, len );
}

/*
static void
gSendNStr( const char *buf, int len )
{
	int tlen = len + 1;
	gDebug( "Sending string %'.*s to %s\n", len, buf, who );
	gWrite( &tlen, sizeof(tlen) );
	gWrite( buf, len );
	gWrite( "", 1 );
}
*/

void
gSendArr( int len, const char *buf )
{
	gDebug( "Sending array %02[:*hhx to %s\n", len, buf, who );
	gWrite( &len, sizeof(len) );
	gWrite( buf, len );
}

int
gRecvInt()
{
	int val;

	gDebug( "Receiving int from %s ...\n", who );
	gRead( &val, sizeof(val) );
	gDebug( " -> %d (%#x)\n", val, val );
	return val;
}

static char *
igRecvArr( int *rlen )
{
	int len;
	char *buf;

	gRead( &len, sizeof(len) );
	*rlen = len;
	gDebug( " -> %d bytes\n", len );
	if (!len)
		return (char *)0;
	if (!(buf = malloc( len )))
		logPanic( "No memory for read buffer\n" );
	gRead( buf, len );
	return buf;
}

char *
gRecvStr()
{
	int len;
	char *buf;

	gDebug( "Receiving string from %s ...\n", who );
	buf = igRecvArr( &len );
	gDebug( " -> %'.*s\n", len, buf );
	return buf;
}

char **
gRecvStrArr( int *rnum )
{
	int num;
	char **argv, **cargv;

	gDebug( "Receiving string array from %s ...\n", who );
	gRead( &num, sizeof(num) );
	gDebug( " -> %d strings\n", num );
	if (rnum)
		*rnum = num;
	if (!num)
		return (char **)0;
	if (!(argv = malloc( num * sizeof(char *) )))
		logPanic( "No memory for read buffer\n" );
	for (cargv = argv; --num >= 0; cargv++)
		*cargv = gRecvStr();
	return argv;
}

char *
gRecvArr( int *num )
{
	char *arr;

	gDebug( "Receiving array from %s ...\n", who );
	gRead( num, sizeof(*num) );
	gDebug( " -> %d bytes\n", *num );
	if (!*num)
		return (char *)0;
	if (!(arr = malloc( *num )))
		logPanic( "No memory for read buffer\n" );
	gRead( arr, *num );
	gDebug( " -> %02[*hhx\n", *num, arr );
	return arr;
}

static void
reqCfg( int id )
{
	gSendInt( G_GetCfg );
	gSendInt( id );
	switch (gRecvInt()) {
	case GE_NoEnt:
		logPanic( "Config value %#x not available\n", id );
	case GE_BadType:
		logPanic( "Core does not know type of config value %#x\n", id );
	}
}

int
getCfgInt( int id )
{
	reqCfg( id );
	return gRecvInt();
}

char *
getCfgStr( int id )
{
	reqCfg( id );
	return gRecvStr();
}

char **
getCfgStrArr( int id, int *len )
{
	reqCfg( id );
	return gRecvStrArr( len );
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
	debug( "ignoring X error\n" );
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
			debug( "XKillClient 0x%lx\n", (unsigned long)children[child] );
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
		logError( "pseudoReset timeout\n" );
	} else {
		(void)signal( SIGALRM, abortReset );
		(void)alarm( 30 );
		XSetErrorHandler( ignoreErrors );
		for (screen = 0; screen < ScreenCount( dpy ); screen++) {
			debug( "pseudoReset screen %d\n", screen );
			killWindows( dpy, RootWindow( dpy, screen ) );
		}
		debug( "before XSync\n" );
		XSync( dpy, False );
		(void)alarm( 0 );
	}
	signal( SIGALRM, SIG_DFL );
	XSetErrorHandler( (XErrorHandler)0 );
	debug( "pseudoReset done\n" );
}


static jmp_buf syncJump;

static void
syncTimeout( int n ATTR_UNUSED )
{
	longjmp( syncJump, 1 );
}

void
secureDisplay( Display *dpy )
{
	debug( "secureDisplay %s\n", dname );
	(void)alarm( (unsigned)_grabTimeout );
	(void)signal( SIGALRM, syncTimeout );
	if (setjmp( syncJump )) {
		logError( "Display %s could not be secured\n", dname );
		sleep( 10 );
		exit( EX_RESERVER_DPY );
	}
	debug( "Before XGrabServer %s\n", dname );
	XGrabServer( dpy );
	debug( "XGrabServer succeeded %s\n", dname );
	(void)alarm( 0 );
	(void)signal( SIGALRM, SIG_DFL );
	pseudoReset( dpy );
	if (!_grabServer)
	{
		XUngrabServer( dpy );
		XSync( dpy, False );
	}
	debug( "secureDisplay %s done\n", dname );
}

void
unsecureDisplay( Display *dpy )
{
	debug( "Unsecure display %s\n", dname );
	if (_grabServer) {
		XUngrabServer( dpy );
		XSync( dpy, False );
	}
}

#define GRABEVENTS \
	ButtonPressMask | ButtonReleaseMask | PointerMotionMask | \
	EnterWindowMask | LeaveWindowMask

void
secureKeyboard( Display *dpy )
{
	(void)alarm( (unsigned)_grabTimeout );
	(void)signal( SIGALRM, syncTimeout );
	if (setjmp( syncJump ) ||
	    XGrabKeyboard( dpy, DefaultRootWindow( dpy ), True,
	                   GrabModeAsync, GrabModeAsync,
	                   CurrentTime ) != GrabSuccess)
	{
		(void)alarm( 0 );
		(void)signal( SIGALRM, SIG_DFL );
		logError( "Keyboard on display %s could not be secured\n", dname );
		exit( EX_RESERVER_DPY );
	}
	(void)alarm( 0 );
	(void)signal( SIGALRM, SIG_DFL );
}

void
securePointer( Display *dpy )
{
	(void)alarm( (unsigned)_grabTimeout );
	(void)signal( SIGALRM, syncTimeout );
	if (setjmp( syncJump ) ||
	    XGrabPointer( dpy, DefaultRootWindow( dpy ), True, GRABEVENTS,
	                  GrabModeAsync, GrabModeAsync,
	                  None, None, CurrentTime ) != GrabSuccess)
	{
		(void)alarm( 0 );
		(void)signal( SIGALRM, SIG_DFL );
		logError( "Pointer on display %s could not be secured\n", dname );
		exit( EX_RESERVER_DPY );
	}
	(void)alarm( 0 );
	(void)signal( SIGALRM, SIG_DFL );
}

void
secureInputs( Display *dpy )
{
	debug( "secureInputs %s\n", dname );
	secureKeyboard( dpy );
	securePointer( dpy );
	XSetInputFocus( dpy, None, None, CurrentTime );
	debug( "secureInputs %s done\n", dname );
}

void
unsecureInputs( Display *dpy )
{
	debug( "unsecureInputs %s\n", dname );
	XSetInputFocus( dpy, PointerRoot, PointerRoot, CurrentTime );
	XUngrabKeyboard( dpy, CurrentTime );
	XUngrabPointer( dpy, CurrentTime );
	XSync( dpy, False );
}

static jmp_buf pingTime;

static int
pingLostIOErr( Display *dpy ATTR_UNUSED )
{
	longjmp( pingTime, 1 );
}

static void
pingLostSig( int n ATTR_UNUSED )
{
	longjmp( pingTime, 1 );
}

int
pingServer( Display *dpy )
{
	int (*oldError)( Display * );
	void (*oldSig)( int );
	int oldAlarm;

	oldError = XSetIOErrorHandler( pingLostIOErr );
	oldAlarm = alarm( 0 );
	oldSig = signal( SIGALRM, pingLostSig );
	(void)alarm( _pingTimeout * 60 );
	if (!setjmp( pingTime )) {
		debug( "Ping server\n" );
		XSync( dpy, False );
	} else {
		debug( "Server dead\n" );
		(void)alarm( 0 );
		(void)signal( SIGALRM, SIG_DFL );
		XSetIOErrorHandler( oldError );
		return False;
	}
	(void)alarm( 0 );
	(void)signal( SIGALRM, oldSig );
	(void)alarm( oldAlarm );
	debug( "Server alive\n" );
	XSetIOErrorHandler( oldError );
	return True;
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
xkbInit( Display *dpy )
{
	int xkb_opcode, xkb_event, xkb_error;
	int xkb_lmaj = XkbMajorVersion;
	int xkb_lmin = XkbMinorVersion;
	return XkbLibraryVersion( &xkb_lmaj, &xkb_lmin ) &&
	       XkbQueryExtension( dpy, &xkb_opcode, &xkb_event,
	                          &xkb_error, &xkb_lmaj, &xkb_lmin );
}

static unsigned int
xkbModifierMaskWorker( XkbDescPtr xkb, const char *name )
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
xkbModifierMask( Display *dpy, const char *name )
{
	XkbDescPtr xkb;

	if ((xkb = XkbGetKeyboard( dpy, XkbAllComponentsMask, XkbUseCoreKbd ))) {
		unsigned int mask = xkbModifierMaskWorker( xkb, name );
		XkbFreeKeyboard( xkb, 0, True );
		return mask;
	}
	return 0;
}

static int
xkbGetModifierState( Display *dpy, const char *name )
{
	unsigned int mask;
	XkbStateRec state;

	if (!(mask = xkbModifierMask( dpy, name )))
		return 0;
	XkbGetState( dpy, XkbUseCoreKbd, &state );
	return (mask & state.locked_mods) != 0;
}

static int
xkbSetModifier( Display *dpy, const char *name, int sts )
{
	unsigned int mask;

	if (!(mask = xkbModifierMask( dpy, name )))
		return False;
	XkbLockModifiers( dpy, XkbUseCoreKbd, mask, sts ? mask : 0 );
	return True;
}
#endif /* HAVE_XKB */

#ifdef HAVE_XTEST
static int
xtestGetModifierState( Display *dpy, int key )
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
xtestFakeKeypress( Display *dpy, int key )
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
setupModifiers( Display *mdpy, int numlock )
{
	if (numlock == 2)
		return;
	newnumstate = numlock;
	nummodified = True;
	dpy = mdpy;
#ifdef HAVE_XKB
	if (xkbInit( mdpy )) {
		havexkb = True;
		oldnumstate = xkbGetModifierState( mdpy, "NumLock" );
		xkbSetModifier( mdpy, "NumLock", numlock );
		return;
	}
#endif
#ifdef HAVE_XTEST
	oldnumstate = xtestGetModifierState( mdpy, XK_Num_Lock );
	if (oldnumstate != numlock)
		xtestFakeKeypress( mdpy, XK_Num_Lock );
#endif
}

void
restoreModifiers( void )
{
#ifdef HAVE_XTEST
	int numstat;
#endif

	if (!nummodified)
		return;
#ifdef HAVE_XKB
	if (havexkb) {
		if (xkbGetModifierState( dpy, "NumLock" ) == newnumstate)
			xkbSetModifier( dpy, "NumLock", oldnumstate );
		return;
	}
#endif
#ifdef HAVE_XTEST
	numstat = xtestGetModifierState( dpy, XK_Num_Lock );
	if (numstat == newnumstate && newnumstate != oldnumstate)
		xtestFakeKeypress( dpy, XK_Num_Lock );
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
