    /*

    KDE Greeter module for xdm
    $Id$

    Copyright (C) 2001-2002 Oswald Buddenhagen <ossi@kde.org>

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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    */

#include <config.h>

#include "kdm_greet.h"

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
#include <sys/stat.h>

#if defined(HAVE_XTEST) || defined(HAVE_XKB)
# include <X11/Xlib.h>
# include <X11/keysym.h>
#endif

#ifdef HAVE_XTEST
# include <X11/extensions/XTest.h>
#endif

#ifdef HAVE_XKB
# define explicit myexplicit
# include <X11/XKBlib.h>
# undef explicit
#endif

#ifdef HAVE_VSYSLOG
# define USE_SYSLOG
#endif

#define LOG_NAME "kdm_greet"
#define LOG_DEBUG_MASK DEBUG_GREET
#define LOG_PANIC_EXIT EX_UNMANAGE_DPY
#define STATIC
#include <printf.c>


char *dname;
int disLocal;
int dhasConsole;

static int rfd, wfd;

static int
Reader (void *buf, int count)
{
    int ret, rlen;

    for (rlen = 0; rlen < count; ) {
      dord:
	ret = read (rfd, (void *)((char *)buf + rlen), count - rlen);
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
GRead (void *buf, int count)
{
    if (Reader (buf, count) != count)
	LogPanic ("Can't read from core\n");
}

static void
GWrite (const void *buf, int count)
{
    if (write (wfd, buf, count) != count)
	LogPanic ("Can't write to core\n");
}

void
GSendInt (int val)
{
    GWrite (&val, sizeof(val));
}

void
GSendStr (const char *buf)
{
    int len = buf ? strlen (buf) + 1 : 0;
    GWrite (&len, sizeof(len));
    GWrite (buf, len);
}

/*
static void
GSendNStr (const char *buf, int len)
{
    int tlen = len + 1;
    GWrite (&tlen, sizeof(tlen));
    GWrite (buf, len);
    GWrite ("", 1);
}
*/

int
GRecvInt ()
{
    int val;

    GRead (&val, sizeof(val));
    return val;
}

char *
GRecvStr ()
{
    int len;
    char *buf;

    len = GRecvInt ();
    if (!len)
	return NULL;
    if (!(buf = malloc (len)))
	LogPanic ("No memory for read buffer\n");
    GRead (buf, len);
    return buf;
}

char **
GRecvStrArr (int *rnum)
{
    int num;
    char **argv, **cargv;

    *rnum = num = GRecvInt ();
    if (!num)
	return (char **)0;
    if (!(argv = malloc (num * sizeof(char *))))
	LogPanic ("No memory for read buffer\n");
    for (cargv = argv; --num >= 0; cargv++)
	*cargv = GRecvStr ();
    return argv;
}

static void
ReqCfg (int id)
{
    GSendInt (G_GetCfg);
    GSendInt (id);
    switch (GRecvInt ()) {
    case GE_NoEnt:
	LogPanic ("Config value 0x%x not available\n", id);
    case GE_BadType:
	LogPanic ("Core does not know type of config value 0x%x\n", id);
    }
}

int
GetCfgInt (int id)
{
    ReqCfg (id);
    return GRecvInt ();
}

char *
GetCfgStr (int id)
{
    ReqCfg (id);
    return GRecvStr ();
}

char **
GetCfgStrArr (int id, int *len)
{
    ReqCfg (id);
    return GRecvStrArr (len);
}

static void ATTR_NORETURN
exitGreeter (void)
{
    char buf[128];

    sprintf (buf, "rm -rf %s", getenv ("HOME"));
    system (buf);
    exit (0);
}

void
SessionExit (int ret)
{
    GSendInt (G_SessionExit);
    GSendInt (ret);
    exitGreeter();
}


static int
ignoreErrors (Display *dpy ATTR_UNUSED, XErrorEvent *event ATTR_UNUSED)
{
    Debug ("ignoring X error\n");
    return 0;
}

/*
 * this is mostly bogus -- but quite useful.  I wish the protocol
 * had some way of enumerating and identifying clients, that way
 * this code wouldn't have to be this kludgy.
 */

static void
killWindows (Display *dpy, Window window)
{
    Window	root, parent, *children;
    unsigned	child, nchildren = 0;
	
    while (XQueryTree (dpy, window, &root, &parent, &children, &nchildren)
	   && nchildren > 0)
    {
	for (child = 0; child < nchildren; child++) {
	    Debug ("XKillClient 0x%lx\n", (unsigned long)children[child]);
	    XKillClient (dpy, children[child]);
	}
	XFree ((char *)children);
    }
}

static jmp_buf	resetJmp;

static void
abortReset (int n ATTR_UNUSED)
{
    longjmp (resetJmp, 1);
}

/*
 * this display connection better not have any windows...
 */
 
static void
pseudoReset (Display *dpy)
{
    int		screen;

    if (setjmp (resetJmp)) {
	LogError ("pseudoReset timeout\n");
    } else {
	(void) signal (SIGALRM, abortReset);
	(void) alarm (30);
	XSetErrorHandler (ignoreErrors);
	for (screen = 0; screen < ScreenCount (dpy); screen++) {
	    Debug ("pseudoReset screen %d\n", screen);
	    killWindows (dpy, RootWindow (dpy, screen));
	}
	Debug ("before XSync\n");
	XSync (dpy, False);
	(void) alarm (0);
    }
    signal (SIGALRM, SIG_DFL);
    XSetErrorHandler ((XErrorHandler)0 );
    Debug ("pseudoReset done\n");
}


static jmp_buf syncJump;

static void
syncTimeout (int n ATTR_UNUSED)
{
    longjmp (syncJump, 1);
}

static int dgrabTimeout;
int dgrabServer;

void
SecureDisplay (Display *dpy)
{
    Debug ("SecureDisplay %s\n", dname);
    (void) signal (SIGALRM, syncTimeout);
    if (setjmp (syncJump)) {
	LogError ("Display %s could not be secured\n", dname);
	SessionExit (EX_RESERVER_DPY);
    }
    (void) alarm ((unsigned) dgrabTimeout);
    Debug ("Before XGrabServer %s\n", dname);
    XGrabServer (dpy);
    Debug ("XGrabServer succeeded %s\n", dname);
    if (XGrabKeyboard (dpy, DefaultRootWindow (dpy), True, GrabModeAsync,
		       GrabModeAsync, CurrentTime) != GrabSuccess)
    {
	(void) alarm (0);
	(void) signal (SIGALRM, SIG_DFL);
	LogError ("Keyboard on display %s could not be secured\n", dname);
	SessionExit (EX_RESERVER_DPY);
    }
    (void) alarm (0);
    (void) signal (SIGALRM, SIG_DFL);
    pseudoReset (dpy);
    if (!dgrabServer)
    {
	XUngrabServer (dpy);
	XSync (dpy, 0);
    }
    Debug ("done secure %s\n", dname);
}

void
UnsecureDisplay (Display *dpy)
{
    Debug ("Unsecure display %s\n", dname);
    if (dgrabServer)
    {
	XUngrabServer (dpy);
	XSync (dpy, 0);
    }
}

/*
static jmp_buf	pingTime;

static int
PingLostIOErr (Display *dpy ATTR_UNUSED)
{
    longjmp (pingTime, 1);
}

static void
PingLostSig (int n ATTR_UNUSED)
{
    longjmp (pingTime, 1);
}

static int dpingTimeout;

int
PingServer (Display *dpy)
{
    int		(*oldError)(Display *);
    void	(*oldSig)(int);
    int		oldAlarm;
    
    oldError = XSetIOErrorHandler (PingLostIOErr);
    oldAlarm = alarm (0);
    oldSig = signal (SIGALRM, PingLostSig);
    (void) alarm (dpingTimeout * 60);
    if (!setjmp (pingTime))
    {
	Debug ("Ping server\n");
	XSync (dpy, 0);
    }
    else
    {
	Debug ("Server dead\n");
	(void) alarm (0);
	(void) signal (SIGALRM, SIG_DFL);
	XSetIOErrorHandler (oldError);
	return 0;
    }
    (void) alarm (0);
    (void) signal (SIGALRM, oldSig);
    (void) alarm (oldAlarm);
    Debug ("Server alive\n");
    XSetIOErrorHandler (oldError);
    return 1;
}
*/

/*
 * Modifier changing code based on kdebase/kxkb/kcmmisc.cpp
 *
 * XTest part: Copyright (C) 2000-2001 Lubos Lunak        <l.lunak@kde.org>
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

    if ((xkb = XkbGetKeyboard( dpy, XkbAllComponentsMask, XkbUseCoreKbd))) {
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
    XkbGetState( dpy, XkbUseCoreKbd, &state);
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
	if (xkb_get_modifier_state( dpy, "NumLock") == newnumstate)
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

extern void kg_main(int, char **);

int
main (int argc, char **argv)
{
    char *ci;
    FILE *f;
    int i;
    char qtrc[40];

    if (!(ci = getenv("CONINFO"))) {
	fprintf(stderr, "This program is part of kdm and should not be run manually.\n");
	return 1;
    }
    if (sscanf (ci, "%d %d", &rfd, &wfd) != 2)
	return 1;

    InitLog();

    if ((debugLevel = GRecvInt ()) & DEBUG_WGREET)
	sleep (100);

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
    strcat( qtrc, "/.qt" );
    mkdir( qtrc, 0700 );
    strcat( qtrc, "/qtrc" );
    if (!(f = fopen( qtrc, "w" )))
	LogPanic( "Cannot create qt config\n" );
    fprintf( f, "[General]\nuseXft=%s\n",
		GetCfgInt (C_AntiAliasing) ? "true" : "false" );
    /* XXX add plugin path, etc. */
    fclose( f );

    dname = GetCfgStr (C_name);
    dgrabServer = GetCfgInt (C_grabServer);
    dgrabTimeout = GetCfgInt (C_grabTimeout);
/*    dpingInterval = GetCfgInt (C_pingInterval);*/	/* XXX not here */
/*    dpingTimeout = GetCfgInt (C_pingTimeout);*/
    disLocal = (GetCfgInt (C_displayType) & d_location) == dLocal;
    if ((ci = GetCfgStr (C_console))) {
	dhasConsole = ci[0] != 0;
	free (ci);
    }

    kg_main(argc, argv);

    exitGreeter();

    return 0;
}
