    /*

    KDE Greeter module for xdm
    $Id$

    Copyright (C) 2001 Oswald Buddenhagen <ossi@kde.org>

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

#include "kdm_greet.h"

/*#include <X11/Xlib.h>*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <signal.h>
#include <setjmp.h>
#include <ctype.h>

char *dname;
int disLocal;

static int rfd, wfd;

void
GSendInt (int val)
{
    write (wfd, &val, sizeof(val));
}

void
GSendStr (const char *buf)
{
    int len = strlen (buf) + 1;
    write (wfd, &len, sizeof(len));
    write (wfd, buf, len);
}

static void
GSendNStr (const char *buf, int len)
{
    int tlen = len + 1;
    write (wfd, &tlen, sizeof(tlen));
    write (wfd, buf, len);
    write (wfd, "", 1);
}

int
GRecvInt ()
{
    int val;

    if (read (rfd, &val, sizeof(val)) != sizeof(val))
	exit (1);
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
	exit (2);
    if (read (rfd, buf, len) != len)
	exit (1);
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
	exit (2);
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

void
SessionExit (int ret)
{
    GSendInt (G_SessionExit);
    GSendInt (ret);
    exit (0);
}

static void
LogOutOfMem (const char *fkt)
{
    char buf[100];

    GSendInt (G_Log);
    GSendInt (DM_ERR);
    sprintf (buf, "Out of memory in %s()", fkt);
    GSendStr (buf);
}

#define OCLBufMisc
#define OCLBufInit
#define OCLBufPrint \
	GSendInt (G_Log); \
	GSendInt (oclbp->type); \
	GSendNStr (oclbp->buf, oclbp->clen);
#include <printf.c>

static int debugLevel;

void
Debug (const char *fmt, ...)
{
    va_list args;

    if (debugLevel & DEBUG_GREET)
    {
	va_start(args, fmt);
	Logger (DM_DEBUG, fmt, args);
	va_end(args);
    }
}

void 
LogInfo(const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    Logger (DM_INFO, fmt, args);
    va_end(args);
}

void
LogError (const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    Logger (DM_ERR, fmt, args);
    va_end(args);
}

void
LogPanic (const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    Logger (DM_PANIC, fmt, args);
    va_end(args);
    SessionExit (EX_UNMANAGE_DPY);
}


static int
ignoreErrors (Display *dpy, XErrorEvent *event)
{
    Debug ("ignoring error\n");
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
    int		child;
    unsigned	nchildren = 0;
	
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
 
void
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


extern void kg_main(int, char **);

int
main (int argc, char **argv)
{
    char *ci;

    if (!(ci = getenv("CONINFO"))) {
	fprintf(stderr, "This program is part of kdm and should not be run manually.\n");
	return 1;
    }
    if (sscanf (ci, "%d %d", &rfd, &wfd) != 2)
	return 1;

    if ((debugLevel = GRecvInt ()) & DEBUG_GDB)
	sleep (100);

    dname = GetCfgStr (C_name);
    dgrabServer = GetCfgInt (C_grabServer);
    dgrabTimeout = GetCfgInt (C_grabTimeout);
/*    dpingInterval = GetCfgInt (C_pingInterval);*/	/* XXX not here */
    dpingTimeout = GetCfgInt (C_pingTimeout);
    disLocal = (GetCfgInt (C_displayType) & location) == Local;

    Debug ("Invoking kg_main ...\n");
    kg_main(argc, argv);
    return 0;
}
