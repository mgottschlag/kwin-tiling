/* $TOG: session.c /main/79 1998/02/09 13:56:17 kaleb $ */
/* $Id$ */
/*

Copyright 1988, 1998  The Open Group

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
IN NO EVENT SHALL THE OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from The Open Group.

*/
/* $XFree86: xc/programs/xdm/session.c,v 3.23 2000/06/17 00:27:34 dawes Exp $ */

/*
 * xdm - display manager daemon
 * Author:  Keith Packard, MIT X Consortium
 *
 * session.c
 */

#include "dm.h"
#include "dm_error.h"

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>

#include <signal.h>
#include <stdio.h>
#include <ctype.h>

static Jmp_buf	pingTime;

/* ARGSUSED */
static SIGVAL
catchAlrm (int n ATTR_UNUSED)
{
    Longjmp (pingTime, 1);
}

static Jmp_buf	tenaciousClient;

/* ARGSUSED */
static SIGVAL
waitAbort (int n ATTR_UNUSED)
{
    Longjmp (tenaciousClient, 1);
}

#if defined(_POSIX_SOURCE) || defined(SYSV) || defined(SVR4)
# define killpg(pgrp, sig) kill(-(pgrp), sig)
#endif

static void
AbortClient (int pid)
{
    int	sig = SIGTERM;
    volatile int	i;
    int	retId;

    for (i = 0; i < 4; i++) {
	if (killpg (pid, sig) == -1) {
	    switch (errno) {
	    case EPERM:
		LogError ("Can't kill client\n");
	    case EINVAL:
	    case ESRCH:
		return;
	    }
	}
	if (!Setjmp (tenaciousClient)) {
	    (void) Signal (SIGALRM, waitAbort);
	    (void) alarm ((unsigned) 10);
	    retId = wait ((waitType *) 0);
	    (void) alarm ((unsigned) 0);
	    (void) Signal (SIGALRM, SIG_DFL);
	    if (retId == pid)
		break;
	} else
	    (void) Signal (SIGALRM, SIG_DFL);
	sig = SIGKILL;
    }
}


static void
ResetUser(void)
{
    if (curuser) {
	free (curuser);
	curuser = 0;
    }
    if (curdmrc) {
	free (curdmrc);
	curdmrc = 0;
    }
    if (newdmrc) {
	free (newdmrc);
	newdmrc = 0;
    }
}

static int clientPid;

static int
AutoLogon (struct display *d)
{
    const char	*name, *pass;
    Time_t	tdiff;

    ResetUser();
    tdiff = time (0) - d->hstent->lastExit - d->openDelay;
Debug ("autoLogon, tdiff = %d, rLogin = %d, goodexit = %d, user = %s\n", 
	tdiff, d->hstent->rLogin, d->hstent->goodExit, d->hstent->nuser);
    if (d->hstent->rLogin >= 1) {
	if (d->hstent->rLogin == 1 &&
	    (d->hstent->goodExit || d->hstent->lock || 
	     !d->hstent->nuser[0] || tdiff > 0))
	    return 0;
	name = d->hstent->nuser;
	pass = d->hstent->npass;
	StrDup (&newdmrc, d->hstent->nargs);
    } else if (d->autoUser[0] != '\0') {
	if (tdiff <= 0 && d->hstent->goodExit)
	    return 0;
	name = d->autoUser;
	pass = d->autoPass;
    } else
	return 0;

    if (Verify (d, name, pass) != V_OK)
	return 0;
    clientPid = StartClient (d);
    if (!clientPid)
	LogError ("Session start failed\n");
    return clientPid;
}

static int greeter;
GProc grtproc;
GTalk grttalk;
GTalk mstrtalk;	/* make static; see dm.c */

int
CtrlGreeterWait (struct display *d, int wreply)
{
    int		i, j, cmd, type, exitCode;
    char	*name, *pass, **avptr;
#ifdef XDMCP
    ARRAY8Ptr	aptr;
#endif

    if (Setjmp (mstrtalk.errjmp) || Setjmp (grttalk.errjmp)) {
	CloseGreeter (TRUE);
	SessionExit (d, EX_RESERVER_DPY);
    }
    while (GRecvCmd (&cmd)) {
	switch (cmd)
	{
	case G_Ready:
	    Debug ("G_Ready\n");
	    return 0;
	case G_Login:	    /* XXX HACK! */
	    Debug ("G_Login\n");
	    return 0;
	case G_GetCfg:
	    Debug ("G_GetCfg\n");
	    type = GRecvInt ();
	    Debug (" index %#x\n", type);
	    if (!(avptr = FindCfgEnt (d, type))) {
		Debug (" -> not found\n");
		GSendInt (GE_NoEnt);
		break;
	    }
	    switch (type & C_TYPE_MASK)
	    {
	    default:
		Debug (" -> unknown type\n");
		GSendInt (GE_BadType);
		break;
	    case C_TYPE_INT:
	    case C_TYPE_STR:
	    case C_TYPE_ARGV:
#ifdef XDMCP
	    case C_TYPE_ARR:
#endif
		GSendInt (GE_Ok);
		switch (type & C_TYPE_MASK)
		{
		case C_TYPE_INT:
		    Debug (" -> int %#x (%d)\n", *(int *)avptr, *(int *)avptr);
		    GSendInt (*(int *)avptr);
		    break;
		case C_TYPE_STR:
		    Debug (" -> string %\"s\n", *avptr);
		    GSendStr (*avptr);
		    break;
		case C_TYPE_ARGV:
		    Debug (" -> sending argv %\"[{s\n", *(char ***)avptr);
		    GSendArgv (*(char ***)avptr);
		    break;
#ifdef XDMCP
		case C_TYPE_ARR:
		    aptr = *(ARRAY8Ptr *)avptr;
		    Debug (" -> sending array %02[*:hhx\n", 
			   aptr->length, aptr->data);
		    GSendArr (aptr->length, (char *)aptr->data);
		    break;
#endif
		}
		break;
	    }
	    break;
	case G_ReadDmrc:
	    Debug ("G_ReadDmrc\n");
	    name = GRecvStr ();
	    Debug (" user %\"s\n", name);
	    if (StrCmp (curuser, name)) {
		ResetUser();
		curuser = name;
		i = ReadDmrc ();
		Debug (" -> status %d\n", i);
		GSendInt (i);
		Debug (" => %\"s\n", curdmrc);
	    } else {
		free (name);
		Debug (" -> status " stringify(GE_Ok) "\n");
		GSendInt (GE_Ok);
		Debug (" => keeping old\n");
	    }
	    break;
	case G_GetDmrc:
	    Debug ("G_GetDmrc\n");
	    name = GRecvStr ();
	    Debug (" key %\"s\n", name);
	    pass = iniEntry (curdmrc, "Desktop", name, 0);
	    Debug (" -> %\"s\n", pass);
	    GSendStr (pass);
	    if (pass)
		free (pass);
	    free (name);
	    break;
	case G_PutDmrc:
	    Debug ("G_PutDmrc\n");
	    name = GRecvStr ();
	    Debug (" key %\"s\n", name);
	    pass = GRecvStr ();
	    Debug (" value %\"s\n", pass);
	    newdmrc = iniEntry (newdmrc, "Desktop", name, pass);
	    free (pass);
	    free (name);
	    break;
	case G_SessionExit:
	    Debug ("G_SessionExit\n");
	    exitCode = GRecvInt ();
	    Debug (" code %d\n", exitCode);
	    /* CloseGreeter (FALSE); not really necessary, init will reap it */
	    SessionExit (d, exitCode);
	    break;
	case G_Verify:
	    Debug ("G_Verify\n");
	    name = GRecvStr ();
	    Debug (" user %\"s\n", name);
	    pass = GRecvStr ();
	    Debug (pass[0] ? " password\n" : " no password\n");
	    GSendInt (i = Verify (d, name, pass));
	    Debug (" -> return %d\n", i);
	    WipeStr (pass);
	    free (name);
	    break;
	case G_Restrict:
	    Debug ("G_Restrict(...)\n");
	    Restrict (d);
	    break;
	case G_Shutdown:
	    i = GRecvInt ();
	    j = GRecvInt ();
#ifdef nofork_session
	    if (nofork_session)
		break;
#endif
	    GSet (&mstrtalk);
	    GSendInt (D_Shutdown);
	    GSendInt (i);
	    GSendInt (j);
	    GSet (&grttalk);
	    break;
	case G_SetupDpy:
	    Debug ("G_SetupDpy\n");
	    SetupDisplay (d);
	    GSendInt (0);
	    break;
	default:
	    return cmd;
	}
	if (!wreply)
	    return -1;
    }
    LogError ("Greeter exited unexpectedly\n");
    CloseGreeter (FALSE);
    SessionExit (d, EX_RESERVER_DPY);
}

void
OpenGreeter (struct display *d)
{
    char	*name, **env;
    Cursor	xcursor;

    GSet (&grttalk);
    if (greeter)
	return;
    greeter = 1;

    /* Hourglass cursor */
    if ((xcursor = XCreateFontCursor (dpy, XC_watch)))
    {
	XDefineCursor (dpy, DefaultRootWindow (dpy), xcursor);
	XFreeCursor (dpy, xcursor);
    }
    XFlush (dpy);

    /* Load system default Resources (if any) */
    LoadXloginResources (d);

    grttalk.pipe = &grtproc.pipe;
    env = systemEnv (d, 0, 0);
    ASPrintf (&name, "greeter for display %s", d->name);
    if (GOpen (&grtproc, (char **)0, "_greet", env, name))
	SessionExit (d, EX_RESERVER_DPY);
    freeStrArr (env);
    CtrlGreeterWait (d, 1);
}

void
CloseGreeter (int force)
{
    if (!greeter)
	return;
    (void) GClose (&grtproc, force);
    DeleteXloginResources ();
    greeter = 0;
}

/* XXX all extremely hacky */

static Jmp_buf	idleTOJmp;

/* ARGSUSED */
static SIGVAL
IdleTOJmp (int n ATTR_UNUSED)
{
    Longjmp (idleTOJmp, 1);
}

static int
DoGreet (struct display *d)
{
    int		cmd;

    if (Setjmp (idleTOJmp)) {
	CloseGreeter (TRUE);
	SessionExit (d, EX_RESERVE);
    }
    Signal (SIGALRM, IdleTOJmp);
    alarm (d->idleTimeout);

    OpenGreeter (d);
    GSendInt (G_Greet);
    cmd = CtrlGreeterWait (d, 1);

    alarm (0);

    if (cmd == G_Ready) {
	CloseGreeter (FALSE);
	clientPid = StartClient (d);
	if (!clientPid)
	    LogError ("Session start failed\n");
    }

    return cmd;
}


static Jmp_buf	abortSession;

/* ARGSUSED */
static SIGVAL
catchTerm (int n ATTR_UNUSED)
{
    Longjmp (abortSession, EX_AL_RESERVER_DPY);
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
IOErrorHandler (Display *dspl ATTR_UNUSED)
{
    LogError("Fatal X server IO error: %s\n", SysErrorMsg());
    Longjmp (abortSession, EX_AL_RESERVER_DPY);	/* XXX EX_RESERVER_DPY */
    /*NOTREACHED*/
    return 0;
}

/*ARGSUSED*/
static int
ErrorHandler(Display *dspl ATTR_UNUSED, XErrorEvent *event)
{
    LogError("X error\n");
    if (event->error_code == BadImplementation)
	Longjmp (abortSession, EX_UNMANAGE_DPY); /* XXX EX_RETRY_ONCE */
    return 0;
}

void
ManageSession (struct display *d)
{
    int ex, cmd;

    Debug ("ManageSession %s\n", d->name);
    if ((ex = Setjmp (abortSession))) {
	CloseGreeter (TRUE);
	if (clientPid)
	    AbortClient (clientPid);
	SessionExit (d, ex);
	/* NOTREACHED */
    }
    (void)XSetIOErrorHandler (IOErrorHandler);
    (void)XSetErrorHandler (ErrorHandler);
    (void)Signal (SIGTERM, catchTerm);
#ifndef HAS_SETPROCTITLE
    SetTitle(d->name, (char *) 0);
#else
    setproctitle("-%s", d->name);
#endif

    if (Setjmp (grttalk.errjmp))
	Longjmp (abortSession, EX_RESERVER_DPY);	/* EX_RETRY_ONCE */

#ifdef XDMCP
    if (d->useChooser)
	DoChoose (d);
	/* NOTREACHED */
#endif

    if (!AutoLogon(d)) {
	if (((d->displayType & d_location) == dLocal) &&
	    d->loginMode >= LOGIN_DEFAULT_REMOTE)
	    goto choose;
	for (;;) {
	    cmd = DoGreet(d);
	  recmd:
	    if (cmd == G_DChoose) {
	      choose:
		cmd = DoChoose (d);
		goto recmd;
	    }
	    if (cmd == G_DGreet)
		continue;
	    if (cmd == G_Ready)
		break;
	    LogError ("Received unknown command %d from greeter\n", cmd);
	    CloseGreeter (TRUE);
	    SessionExit (d, EX_RESERVER_DPY);	/* XXX hmpf ... EX_DELAYED_RETRY_ONCE */
	}

    }

    Debug ("client Started\n");
    /*
     * Wait for session to end,
     */
    for (;;) {
	if (!Setjmp (pingTime))
	{
	    (void) Signal (SIGALRM, catchAlrm);
	    (void) alarm (d->pingInterval * 60); /* may be 0 */
	    (void) Wait4 (clientPid);
	    (void) alarm (0);
	    break;
	}
	else
	{
	    (void) alarm (0);
	    if (!PingServer (d))
		catchTerm (SIGTERM);
	}
    }
    /* 
     * Sometimes the Xsession somehow manages to exit before
     * a server crash is noticed - so we sleep a bit and wait
     * for being killed.
     */
    if (!PingServer (d)) {
	Debug("X server dead upon session exit.\n");
	if ((d->displayType & d_location) == dLocal)
	    sleep (10);
	SessionExit (d, EX_AL_RESERVER_DPY);
    }
    SessionExit (d, EX_NORMAL); /* XXX maybe EX_REMANAGE_DPY? -- enable in dm.c! */
}

void
LoadXloginResources (struct display *d)
{
    char	**args;
    char	**env = 0;

    if (d->resources[0] && access (d->resources, 4) == 0) {
	env = systemEnv (d, (char *) 0, (char *) 0);
	args = parseArgs ((char **) 0, d->xrdb);
	args = parseArgs (args, d->resources);
	Debug ("loading resource file: %s\n", d->resources);
	(void) runAndWait (args, env);
	freeStrArr (args);
	freeStrArr (env);
    }
}

void
SetupDisplay (struct display *d)
{
    char	**env = 0;

    if (d->setup && d->setup[0])
    {
    	env = systemEnv (d, (char *) 0, (char *) 0);
    	(void) source (env, d->setup);
    	freeStrArr (env);
    }
}

void
DeleteXloginResources ()
{
    int i;
    Atom prop = XInternAtom(dpy, "SCREEN_RESOURCES", True);

    XDeleteProperty(dpy, RootWindow (dpy, 0), XA_RESOURCE_MANAGER);
    if (prop) {
	for (i = ScreenCount(dpy); --i >= 0; )
	    XDeleteProperty(dpy, RootWindow (dpy, i), prop);
    }
    XSync(dpy, 0);
}


int
source (char **env, char *file)
{
    char	**args, *args_safe[2];
    int		ret;

    if (file && file[0]) {
	Debug ("source %s\n", file);
	args = parseArgs ((char **) 0, file);
	if (!args)
	{
	    args = args_safe;
	    args[0] = file;
	    args[1] = NULL;
	    return runAndWait (args, env);
	}
	ret = runAndWait (args, env);
	freeStrArr (args);
	return ret;
    }
    return 0;
}

char **
inheritEnv (char **env, const char **what)
{
    char	*value;

    for (; *what; ++what)
	if ((value = getenv (*what)))
	    env = setEnv (env, *what, value);
    return env;
}

char **
defaultEnv (const char *user)
{
    char	**env;

    env = 0;

#ifdef AIXV3
    /* we need the tags SYSENVIRON: and USRENVIRON: in the call to setpenv() */
    env = setEnv(env, "SYSENVIRON:", 0);
#endif

    if (user) {
	env = setEnv (env, "USER", user);
#ifdef AIXV3
	env = setEnv (env, "LOGIN", user);
#endif
	env = setEnv (env, "LOGNAME", user);
    }

#ifdef AIXV3
    env = setEnv(env, "USRENVIRON:", 0);
#endif

    if (exportList)
	env = inheritEnv (env, (const char **)exportList);

    return env;
}

char **
systemEnv (struct display *d, const char *user, const char *home)
{
    char	**env;

    env = defaultEnv (user);
    if (home)
	env = setEnv (env, "HOME", home);
    env = setEnv (env, "DISPLAY", d->name);
    env = setEnv (env, "PATH", d->systemPath);
    env = setEnv (env, "SHELL", d->systemShell);
    if (d->authFile)
	env = setEnv (env, "XAUTHORITY", d->authFile);
    return env;
}
