/* $TOG: dm.c /main/73 1998/04/09 15:12:03 barstow $ */
/* $Id$ */
/*

Copyright 1988, 1998  The Open Group

All Rights Reserved.

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
/* $XFree86: xc/programs/xdm/dm.c,v 3.10 2000/04/27 16:26:50 eich Exp $ */

/*
 * xdm - display manager daemon
 * Author:  Keith Packard, MIT X Consortium
 *
 * display manager
 */

#include "dm.h"
#include "dm_auth.h"
#include "dm_error.h"

#include <stdio.h>
#include <string.h>
#ifdef X_POSIX_C_SOURCE
# define _POSIX_C_SOURCE X_POSIX_C_SOURCE
# include <signal.h>
# undef _POSIX_C_SOURCE
#else
# if defined(X_NOT_POSIX) || defined(_POSIX_SOURCE)
#  include <signal.h>
# else
#  define _POSIX_SOURCE
#  include <signal.h>
#  undef _POSIX_SOURCE
# endif
#endif
#ifdef __NetBSD__
# include <sys/param.h>
#endif

#ifndef sigmask
# define sigmask(m)  (1 << ((m - 1)))
#endif

#include <sys/stat.h>
#include <errno.h>
#include <X11/Xfuncproto.h>
#include <stdarg.h>

#ifndef F_TLOCK
# ifndef X_NOT_POSIX
#  include <unistd.h>
# endif
#endif

#include <utmp.h>
#ifdef linux
# include <sys/ioctl.h>
# include <linux/vt.h>
#endif

#ifdef X_NOT_STDC_ENV
extern int errno;
#endif


#if defined(SVR4) && !defined(SCO)
extern FILE    *fdopen();
#endif

static SIGVAL	StopAll (int n), UtmpNotify (int n), RescanNotify (int n);
static void	RescanServers (void);
static void	RestartDisplay (struct display *d, int forceReserver);
static void	ScanServers (void);
static void	SetAccessFileTime (void);
static void	StartDisplays (void);
static void	TerminateProcess (int pid, int signal);
static void	ExitDisplay (struct display *d, int doRestart, int forceReserver, int goodExit);
static int	Reader (int fd, void *buf, int len);
static int	CheckUtmp (void);
static void	SwitchToTty (struct display *d);

int	Rescan;
int	ChkUtmp;
long	ServersModTime, AccessFileModTime, ConfigModTime, Config2ParseModTime;

int nofork_session = 0;

#ifndef NOXDMTITLE
static char *Title;
static int TitleLen;
#endif

#ifndef UNRELIABLE_SIGNALS
static SIGVAL ChildNotify (int n);
#endif

static int StorePid (void);

static int parent_pid = -1; 	/* PID of parent xdm process */

char prog[16];

int
main (int argc, char **argv)
{
    int	oldpid, oldumask;
    char *pptr, cmdbuf[1024];

    /*
     * Fix $HOME (if xdm is run by init)
     * XXX this is WRONG for some/many systems (e.g., solaris)
     */
    const char *home = getenv("HOME");
    if (!home || !strcmp("/", home))
	putenv("HOME=/root");

    /* make sure at least world write access is disabled */
    if (((oldumask = umask(022)) & 002) == 002)
	(void) umask (oldumask);

    strncpy(prog, (pptr = strrchr(argv[0], '/')) ? pptr + 1 : argv[0], 
	    sizeof(prog) - 1);

#ifndef NOXDMTITLE
    Title = argv[0];
    TitleLen = (argv[argc - 1] + strlen(argv[argc - 1])) - Title;
#endif

    /*
     * Step 1 - load configuration parameters
     */
    InitResources (argc, argv);
    LoadDMResources ();

    /*
     * Only allow root to run in non-debug mode to avoid problems
     */
    if (debugLevel == 0 && getuid() != 0)
    {
	fprintf (stderr, "Only root wants to run %s\n", argv[0]);
	exit (1);
    }
    if (debugLevel >= 10)
	nofork_session = 1;

    InitErrorLog ();
    if (daemonMode
#ifndef USE_SYSLOG
	&& debugLevel == 0
#endif
    )
	BecomeDaemon ();

    /* SUPPRESS 560 */
    if ((oldpid = StorePid ()))
    {
	if (oldpid == -1)
	    LogError ("Can't create/lock pid file %s\n", pidFile);
	else
	    LogError ("Can't lock pid file %s, another xdm is running (pid %d)\n",
		 pidFile, oldpid);
	exit (1);
    }

    if (nofork_session == 0) {
	/* Clean up any old Authorization files */
#ifndef MINIX
	sprintf(cmdbuf, "/bin/rm -f %s/authdir/authfiles/A*", authDir);
#else
	sprintf(cmdbuf, "/usr/bin/rm -f %s/authdir/authfiles/A*", authDir);
#endif
	system(cmdbuf);
    }

#ifdef XDMCP
    init_session_id ();
    CreateWellKnownSockets ();
#else
    Debug ("not compiled for XDMCP\n");
#endif
    parent_pid = getpid ();
    (void) Signal (SIGTERM, StopAll);
    (void) Signal (SIGINT, StopAll);

    /*
     * Step 2 - Read /etc/Xservers and set up
     *	    the socket.
     *
     *	    Keep a sub-daemon running
     *	    for each entry
     */
    SetAccessFileTime ();
#ifdef XDMCP
    ScanAccessDatabase ();
#endif
    ScanServers ();
    StartDisplays ();
    (void) Signal (SIGHUP, RescanNotify);
#ifndef UNRELIABLE_SIGNALS
    (void) Signal (SIGCHLD, ChildNotify);
#endif
    while (
#ifdef XDMCP
	   AnyWellKnownSockets() ||
#endif
	   AnyDisplaysLeft ())
    {
	if (Rescan)
	{
	    RescanServers ();
	    Rescan = 0;
	}
	if (ChkUtmp)
	{
	    CheckUtmp ();
	    ChkUtmp = 0;
	}
#if defined(UNRELIABLE_SIGNALS) || !defined(XDMCP)
	WaitForChild ();
#else
	WaitForSomething ();
#endif
    }
    Debug ("Nothing left to do, exiting\n");
    exit(0);
    /*NOTREACHED*/
}

static int
Reader (int fd, void *buf, int count)
{
    int ret;
    do {
	ret = read (fd, buf, count);
    } while (ret < 0 && errno == EINTR);
    return ret;
}

/* ARGSUSED */
static SIGVAL
UtmpNotify (int n)
{
    Debug ("Caught SIGALRM\n");
    ChkUtmp = 1;
}

enum utState { UtWait, UtActive };

#ifndef UT_LINESIZE
#define UT_LINESIZE 32
#endif

struct utmps {
    struct utmps *next;
    struct display *d;
    char line[UT_LINESIZE];
    time_t time;
    enum utState state;
    int hadSess;
#ifdef CSRG_BASED
    int checked;
#endif
};

#define TIME_LOG 40
#define TIME_RELOG 10

static struct utmps *utmpList;

static int
CheckUtmp (void)
{
    static time_t modtim;
    int cnt, nck, nextChk;
    time_t now;
    struct utmps *utp, **utpp;
    struct stat st;
    struct utmp ut;

    if (stat(UTMP_FILE, &st))
    {
	LogError (UTMP_FILE " not found - cannot use console mode\n");
	return 0;
    }
    if (!utmpList)
	return 1;
    time(&now);
    if (modtim != st.st_mtime)
    {
	int fd;

	Debug ("Rescanning " UTMP_FILE "\n");
#ifdef CSRG_BASED
	for (utp = utmpList; utp; utp = utp->next)
	    utp->checked = 0;
#endif
	if ((fd = open (UTMP_FILE, O_RDONLY)) < 0)
	{
	    LogError ("Cannot open " UTMP_FILE " - cannot use console mode\n");
	    return 0;
	}
	while ((cnt = Reader (fd, &ut, sizeof(ut))) == sizeof(ut))
	{
	    for (utp = utmpList; utp; utp = utp->next)
		if (!strncmp(utp->line, ut.ut_line, UT_LINESIZE))
		{
#ifdef CSRG_BASED
		    utp->checked = 1;
#else
		    if (ut.ut_type != USER_PROCESS)
		    {
			Debug ("utmp entry for %s marked waiting\n", utp->line);
			utp->state = UtWait;
		    }
		    else
#endif
		    {
			utp->hadSess = 1;
			Debug ("utmp entry for %s marked active\n", utp->line);
			utp->state = UtActive;
		    }
		    if (utp->time < ut.ut_time)
			utp->time = ut.ut_time;
		    break;
		}
	}
	close (fd);
#ifdef CSRG_BASED
	for (utp = utmpList; utp; utp = utp->next)
	    if (!utp->checked && utp->state == UtActive)
	    {
		utp->state = UtWait;
		utp->time = now;
		Debug ("utmp entry for %s marked waiting\n", utp->line);
	    }
#endif
	modtim = st.st_mtime;
    }
    nextChk = 1000;
    for (utpp = &utmpList; (utp = *utpp); )
    {
	if (utp->state == UtWait)
	{
	    time_t remains = utp->time + (utp->hadSess ? TIME_RELOG : TIME_LOG) 
			     - now;
	    if (remains <= 0)
	    {
		struct display *d = utp->d;
		Debug ("console login for %s at %s timed out\n", 
		       utp->d->name, utp->line);
		*utpp = utp->next;
		free (utp);
		ExitDisplay (d, TRUE, TRUE, TRUE);
		StartDisplays ();
		continue;
	    }
	    else
		nck = remains;
	}
	else
#ifdef CSRG_BASED
	    nck = (TIME_RELOG + 5) / 3;
#else
	    nck = TIME_RELOG;
#endif
	if (nck < nextChk)
	    nextChk = nck;
	utpp = &(*utpp)->next;
    }
    if (nextChk < 1000)
    {
	Signal (SIGALRM, UtmpNotify);
	alarm (nextChk);
    }
    return 1;
}

static void
SwitchToTty (struct display *d)
{
    struct utmps *utp;

    if (!d->console || !d->console[0])
    {
	Debug("No console for %s specified", d->name);
	d->status = notRunning;
	return;
    }
    if (!(utp = malloc (sizeof(*utp))))
    {
	LogOutOfMem("SwitchToTty");
	d->status = notRunning;
	return;
    }
    strncpy (utp->line, d->console, UT_LINESIZE);
    utp->d = d;
    utp->time = time(0);
    utp->hadSess = 0;
    utp->next = utmpList;
    utmpList = utp;
#ifdef linux	/* chvt */
    if (!memcmp(d->console, "tty", 3))
    {
	int con = open ("/dev/console", O_RDONLY);
	if (con >= 0)
	{
	    ioctl (con, VT_ACTIVATE, atoi (d->console + 3));
	    close (con);
	}
    }
#endif
    if (!CheckUtmp ())
    {
	utmpList = utp->next;
	free (utmpList);
	d->status = notRunning;
	return;
    }
}

/* ARGSUSED */
static SIGVAL
RescanNotify (int n)
{
    Debug ("Caught SIGHUP\n");
    Rescan = 1;
#ifdef SIGNALS_RESET_WHEN_CAUGHT
    (void) Signal (SIGHUP, RescanNotify);
#endif
}

static void
ScanServers (void)
{
    char	lineBuf[10240];
    FILE	*serversFile;
    struct stat	statb;

    if (servers[0] == '/')
    {
	serversFile = fopen (servers, "r");
	if (serversFile == NULL)
 	{
	    LogError ("cannot access servers file %s\n", servers);
	    return;
	}
	if (ServersModTime == 0)
	{
	    fstat (fileno (serversFile), &statb);
	    ServersModTime = statb.st_mtime;
	}
	while (fgets (lineBuf, sizeof (lineBuf)-1, serversFile))
	    ParseDisplay (lineBuf);
	fclose (serversFile);
    }
    else
    {
	ParseDisplay (servers);
    }
}

static void
MarkDisplay (struct display *d)
{
    d->state = MissingEntry;
}

static void
RescanServers (void)
{
    Debug ("rescanning servers\n");
    LogInfo ("Rescanning both config and servers files\n");
    ForEachDisplay (MarkDisplay);
    ReinitResources ();
    LoadDMResources ();
    ScanServers ();
    SetAccessFileTime ();
#ifdef XDMCP
    ScanAccessDatabase ();
#endif
    StartDisplays ();
}

static void
SetAccessFileTime (void)
{
    struct stat	statb;

    if (stat (accessFile, &statb) != -1)
	AccessFileModTime = statb.st_mtime;
}

static void
RescanIfMod (void)
{
    struct stat	statb;
    int rescan = 0;

    if (stat (config, &statb) != -1 && statb.st_mtime != ConfigModTime) {
	Debug ("Config file %s has changed, rereading\n", config);
	rescan = 1;
    }
    if (config2Parse[0] && stat (config2Parse, &statb) != -1 && 
	statb.st_mtime != Config2ParseModTime) 
    {
	Debug ("Config file %s has changed, rereading\n", config2Parse);
	rescan = 1;
    }
    if (rescan) {
	if (config2Parse[0])
	    LogInfo ("Rereading configuration files %s and %s\n", 
		     config, config2Parse);
	else
	    LogInfo ("Rereading configuration file %s\n", config);
	ReinitResources ();
	LoadDMResources ();
    }
    if (servers[0] == '/' && stat(servers, &statb) != -1)
    {
	if (statb.st_mtime != ServersModTime)
	{
	    Debug ("Servers file %s has changed, rescanning\n", servers);
	    LogInfo ("Rereading servers file %s\n", servers);
	    ServersModTime = statb.st_mtime;
	    ForEachDisplay (MarkDisplay);
	    ScanServers ();
	}
    }
#ifdef XDMCP
    if (accessFile && accessFile[0] && stat (accessFile, &statb) != -1)
    {
	if (statb.st_mtime != AccessFileModTime)
	{
	    Debug ("Access file %s has changed, rereading\n", accessFile);
	    LogInfo ("Rereading access file %s\n", accessFile);
	    AccessFileModTime = statb.st_mtime;
	    ScanAccessDatabase ();
	}
    }
#endif
}

/*
 * catch a SIGTERM, kill all displays and exit
 */

/* ARGSUSED */
static SIGVAL
StopAll (int n)
{
    if (parent_pid != getpid())
    {
	/* 
	 * We are a child xdm process that was killed by the
	 * master xdm before we were able to return from fork()
	 * and remove this signal handler.
	 *
	 * See defect XWSog08655 for more information.
	 */
	Debug ("Child xdm caught SIGTERM before it removed that signal.\n");
	(void) Signal (n, SIG_DFL);
	TerminateProcess (getpid(), SIGTERM);
	return;
    }
    Debug ("Shutting down entire manager\n");
#ifdef XDMCP
    DestroyWellKnownSockets ();
#endif
    ForEachDisplay (StopDisplay);
#ifdef SIGNALS_RESET_WHEN_CAUGHT
    /* to avoid another one from killing us unceremoniously */
    (void) Signal (SIGTERM, StopAll);
    (void) Signal (SIGINT, StopAll);
#endif
}

/*
 * notice that a child has died and may need another
 * sub-daemon started
 */

int	ChildReady;

#ifndef UNRELIABLE_SIGNALS
/* ARGSUSED */
static SIGVAL
ChildNotify (int n)
{
    ChildReady = 1;
#ifdef ISC
    (void) Signal (SIGCHLD, ChildNotify);
#endif
}
#endif

void
WaitForChild (void)
{
    int		pid;
    struct display	*d;
    waitType	status;
#if !defined(X_NOT_POSIX) && !defined(__EMX__)
    sigset_t mask, omask;
#else
    int		omask;
#endif

#ifdef UNRELIABLE_SIGNALS
    /* XXX classic System V signal race condition here with RescanNotify */
    if ((pid = wait (&status)) != -1)
#else
# ifndef X_NOT_POSIX
    sigemptyset(&mask);
    sigaddset(&mask, SIGCHLD);
    sigaddset(&mask, SIGHUP);
    sigaddset(&mask, SIGALRM);
    sigprocmask(SIG_BLOCK, &mask, &omask);
    Debug ("signals blocked\n");
# else
    omask = sigblock (sigmask (SIGCHLD) | sigmask (SIGHUP) | sigmask (SIGALRM));
    Debug ("signals blocked, mask was 0x%x\n", omask);
# endif
    if (!ChildReady && !Rescan)
# ifndef X_NOT_POSIX
	sigsuspend(&omask);
# else
	sigpause (omask);
# endif
    ChildReady = 0;
# ifndef X_NOT_POSIX
    sigprocmask(SIG_SETMASK, &omask, (sigset_t *)NULL);
    while ((pid = waitpid (-1, &status, WNOHANG)) > 0)
# else
    sigsetmask (omask);
    while ((pid = wait3 (&status, WNOHANG, (struct rusage *) 0)) > 0)
# endif
#endif
    {
	Debug ("Manager wait returns pid: %d sig %d core %d code %d\n",
	       pid, waitSig(status), waitCore(status), waitCode(status));
	if (autoRescan)
	    RescanIfMod ();
	/* SUPPRESS 560 */
	if ((d = FindDisplayByPid (pid))) {
	    d->pid = -1;
	    switch (waitVal (status)) {
	    case UNMANAGE_DISPLAY:
		Debug ("Display exited with UNMANAGE_DISPLAY\n");
		ExitDisplay (d, FALSE, FALSE, FALSE);
		break;
	    case ALTMODE_DISPLAY:
		Debug ("Display exited with ALTMODE_DISPLAY\n");
		d->status = suspended;
		TerminateProcess (d->serverPid, d->termSignal);
		break;
	    case OBEYSESS_DISPLAY:
		Debug ("Display exited with OBEYSESS_DISPLAY\n");
		ExitDisplay (d, TRUE, FALSE, TRUE);
		break;
	    default:
		Debug ("Display exited with unknown status %d\n", waitVal(status));
		LogError ("Unknown session exit code %d from process %d\n",
			  waitVal (status), pid);
		ExitDisplay (d, FALSE, FALSE, TRUE);
		break;
	    case OPENFAILED_DISPLAY:
		Debug ("Display exited with OPENFAILED_DISPLAY, try %d of %d\n",
		       d->startTries, d->startAttempts);
		LogError ("Display %s cannot be opened\n", d->name);
		/*
		 * no display connection was ever made, tell the
		 * terminal that the open attempt failed
 		 */
#ifdef XDMCP
		if (d->displayType.origin == FromXDMCP)
		    SendFailed (d, "cannot open display");
#endif
		ExitDisplay (d, TRUE, TRUE, FALSE);
		break;
	    case RESERVER_DISPLAY:
		Debug ("Display exited with RESERVER_DISPLAY\n");
		ExitDisplay (d, TRUE, TRUE, TRUE);
		break;
	    case RESERVER_AL_DISPLAY:
		Debug ("Display exited with RESERVER_AL_DISPLAY\n");
		ExitDisplay (d, TRUE, TRUE, FALSE);
		break;
	    case waitCompose (SIGTERM,0,0):
		Debug ("Display exited on SIGTERM\n");
		ExitDisplay (d, TRUE, TRUE, FALSE);
		break;
	    case REMANAGE_DISPLAY:
		Debug ("Display exited with REMANAGE_DISPLAY\n");
		/*
 		 * XDMCP will restart the session if the display
		 * requests it
		 */
		ExitDisplay (d, TRUE, FALSE, TRUE);
		break;
	    }
	}
	/* SUPPRESS 560 */
	else if ((d = FindDisplayByServerPid (pid)))
	{
	    d->serverPid = -1;
	    switch (d->status)
	    {
	    case suspended:
		Debug ("Server for suspended display %s exited\n", d->name);
		SwitchToTty (d);
		break;
	    case zombie:
		Debug ("Zombie server reaped, removing display %s\n", d->name);
		RemoveDisplay (d);
		break;
	    case phoenix:
		Debug ("Phoenix server arises, restarting display %s\n", d->name);
		d->status = notRunning;
		break;
	    case running:
		Debug ("Server for display %s terminated unexpectedly, status %d\n", d->name, waitVal (status));
		LogError ("Server for display %s terminated unexpectedly: %d\n", d->name, waitVal (status));
		if (d->pid != -1)
		{
		    Debug ("Terminating session pid %d\n", d->pid);
		    TerminateProcess (d->pid, SIGTERM);
		}
		break;
	    case notRunning:
		Debug ("Server exited for notRunning session on display %s\n", d->name);
		break;
	    }
	}
	else
	{
	    Debug ("Unknown child termination, status %d\n", waitVal (status));
	}
    }
    StartDisplays ();
}

static void
CheckDisplayStatus (struct display *d)
{
    if (d->displayType.origin == FromFile)
    {
	switch (d->state) {
	case MissingEntry:
	    StopDisplay (d);
	    break;
	case NewEntry:
	    d->state = OldEntry;
	case OldEntry:
	    if (d->status == notRunning)
		StartDisplay (d);
	    break;
	}
    }
}

static void
StartDisplays (void)
{
    ForEachDisplay (CheckDisplayStatus);
}

void
StartDisplay (struct display *d)
{
    char	buf[100];
    int		pid;

    Debug ("StartDisplay %s\n", d->name);
    LoadServerResources (d);
    if (d->displayType.location == Local)
    {
	/* don't bother pinging local displays; we'll
	 * certainly notice when they exit
	 */
	d->pingInterval = 0;
	if (d->authorize)
	{
	    Debug ("SetLocalAuthorization %s, auth %s\n",
		    d->name, d->authNames[0]);
	    SetLocalAuthorization (d);
	    /*
	     * reset the server after writing the authorization information
	     * to make it read the file (for compatibility with old
	     * servers which read auth file only on reset instead of
	     * at first connection)
	     */
	    if (d->serverPid != -1 && d->resetForAuth && d->resetSignal)
		kill (d->serverPid, d->resetSignal);
	}
	if (d->serverPid == -1 && !StartServer (d))
	{
	    LogError ("Server for display %s can't be started, session disabled\n", d->name);
	    RemoveDisplay (d);
	    return;
	}
    }
    else
    {
	/* this will only happen when using XDMCP */
	if (d->authorizations)
	    SaveServerAuthorizations (d, d->authorizations, d->authNum);
    }
    if (d->fifoCreate && d->fifofd < 0) {
	sprintf (buf, "/tmp/xlogin-%s", d->name);
	unlink (buf);
	if (mkfifo (buf, 0) < 0)
	    LogError ("cannot create login data fifo %s\n", buf);
	else {
	    chown (buf, d->fifoOwner, d->fifoGroup);
	    if (d->fifoOwner >= 0)
		chmod (buf, 0600);
	    else if (d->fifoGroup >= 0)
		chmod (buf, 0620);
	    else
		chmod (buf, 0622);
	    if ((d->fifofd = open (buf, O_RDONLY | O_NONBLOCK)) < 0)
		unlink (buf);
	    else
		RegisterCloseOnFork (d->fifofd);
	}
    }
    if (d->pipefd[0] < 0) {
	if (!pipe (d->pipefd)) {
	    (void) fcntl (d->pipefd[0], F_SETFL, O_NONBLOCK);
	    RegisterCloseOnFork (d->pipefd[0]);
	}
    }
    if (!nofork_session)
	pid = fork ();
    else
	pid = 0;
    switch (pid)
    {
    case 0:
	if (!nofork_session) {
	    CleanUpChild ();
	    (void) Signal (SIGPIPE, SIG_IGN);
	}
	if (d->pipefd[0] >= 0)
	    RegisterCloseOnFork (d->pipefd[1]);
	LoadSessionResources (d);
	SetAuthorization (d);
	if (!WaitForServer (d))
	    exit (OPENFAILED_DISPLAY);
#ifdef XDMCP
	if (d->useChooser)
	    RunChooser (d);
	else
#endif
	    ManageSession (d);
	exit (REMANAGE_DISPLAY);
    case -1:
	break;
    default:
	Debug ("pid: %d\n", pid);
	d->pid = pid;
	d->status = running;
	break;
    }
}

static void
TerminateProcess (int pid, int signal)
{
    kill (pid, signal);
#ifdef SIGCONT
    kill (pid, SIGCONT);
#endif
}

static void
ClrnLog (struct display *d)
{
    if (d->hstent->nLogPipe) {
	bzero (d->hstent->nLogPipe, strlen (d->hstent->nLogPipe));
	free (d->hstent->nLogPipe);
	d->hstent->nLogPipe = NULL;
    }
}

static void
ReadnLog (struct display *d, int fd)
{
    char	buf[100], *p;
    int		bpos, bend, ll, rt, ign;

    for (bpos = 0, bend = 0, ign = 0;;) {
	for (;;) {
	    if ((p = memchr(buf + bpos, 10, bend - bpos)) != 0) {
		ll = (p - buf) - bpos + 1;
		break;
	    }
	    if (bpos == 0 && bend == sizeof (buf)) {
		ign = 1;
		bend = 0;
	    } else {
		memcpy (buf, buf + bpos, bend - bpos);
		bend -= bpos;
	    }
	    bpos = 0;
	    rt = Reader (fd, buf + bend, sizeof (buf) - bend);
	    if (rt < 0) {
		bzero (buf, sizeof(buf));
		return;
	    } else if (!rt) {
		if (bend) {
		    ll = bend;
		    break;
		} else {
		    bzero (buf, sizeof(buf));
		    return;
		}
	    } else
		bend += rt;
	}
	if (ign) {
	    ign = 0;
	} else {
	    ClrnLog (d);
	    if ((d->hstent->nLogPipe = malloc (ll + 1)) != NULL) {
		memcpy (d->hstent->nLogPipe, buf + bpos, ll);
		d->hstent->nLogPipe[ll] = '\0';
	    }
	}
	bpos += ll;
    }
}

static void
ExitDisplay (
    struct display	*d, 
    int			doRestart,
    int			forceReserver,
    int			goodExit)
{
    Time_t	curtime;

    Debug ("Recording exit of %s (GoodExit=%d)\n", d->name, goodExit);

    time (&curtime);
    if (d->hstent->lastExit + d->startInterval < curtime)
	d->hstent->startTries = 0;
    else if (++d->hstent->startTries >= d->startAttempts) {
	LogError ("Display %s is being disabled (exit frequency too high)\n", d->name);
	doRestart = FALSE;
    }
    d->hstent->lastExit = curtime;

    d->hstent->goodExit = goodExit;
    ClrnLog (d);
    if (d->pipefd[0] >= 0)
	ReadnLog (d, d->pipefd[0]);
    if (goodExit)
	ClrnLog (d);
    if (d->fifofd >= 0)
	ReadnLog (d, d->fifofd);

    if (!doRestart ||
	d->displayType.lifetime != Permanent ||
	d->status == zombie)
	StopDisplay (d);
    else
	RestartDisplay (d, forceReserver);
}

/*
 * transition from running to zombie or deleted
 */

void
StopDisplay (struct display *d)
{
    if (d->fifofd >= 0) {
	char buf[100];
	close (d->fifofd);
	ClearCloseOnFork (d->fifofd);
	d->fifofd = -1;
	sprintf (buf, "/tmp/xlogin-%s", d->name);
	unlink (buf);
    }
    if (d->pipefd[0] >= 0) {
	int i;
	for (i = 0; i < 2; i++) {
	    ClearCloseOnFork(d->pipefd[i]); 
	    close (d->pipefd[i]); 
	    d->pipefd[i] = -1;
	}
    }
    if (d->serverPid != -1)
	d->status = zombie; /* be careful about race conditions */
    if (d->pid != -1)
	TerminateProcess (d->pid, SIGTERM);
    if (d->serverPid != -1)
	TerminateProcess (d->serverPid, d->termSignal);
    else
	RemoveDisplay (d);
}

/*
 * transition from running to phoenix or notRunning
 */

static void
RestartDisplay (struct display *d, int forceReserver)
{
    if (d->serverPid != -1 && (forceReserver || d->terminateServer))
    {
	TerminateProcess (d->serverPid, d->termSignal);
	d->status = phoenix;
    }
    else
    {
	d->status = notRunning;
    }
}

static FD_TYPE	CloseMask;
static int	max;

void
RegisterCloseOnFork (int fd)
{
    FD_SET (fd, &CloseMask);
    if (fd > max)
	max = fd;
}

void
ClearCloseOnFork (int fd)
{
    FD_CLR (fd, &CloseMask);
    if (fd == max) {
	while (--fd >= 0)
	    if (FD_ISSET (fd, &CloseMask))
		break;
	max = fd;
    }
}

void
CloseOnFork (void)
{
    int	fd;

    for (fd = 0; fd <= max; fd++)
	if (FD_ISSET (fd, &CloseMask))
	{
#ifdef MINIX
	    nbio_unregister(fd);
#endif
	    close (fd);
        }
    FD_ZERO (&CloseMask);
    max = 0;
#ifdef MINIX
    { extern int chooserFd; nbio_unregister(chooserFd); }
#endif
}

static int  pidFd;
static FILE *pidFilePtr;

static int
StorePid (void)
{
    int		oldpid;

    if (pidFile[0] != '\0') {
	pidFd = open (pidFile, O_RDWR);
	if (pidFd == -1 && errno == ENOENT)
	    pidFd = open (pidFile, O_RDWR|O_CREAT, 0666);
	if (pidFd == -1 || !(pidFilePtr = fdopen (pidFd, "r+")))
	{
	    LogError ("process-id file %s cannot be opened\n",
		      pidFile);
	    return -1;
	}
	if (fscanf (pidFilePtr, "%d\n", &oldpid) != 1)
	    oldpid = -1;
	fseek (pidFilePtr, 0l, 0);
	if (lockPidFile)
	{
#ifdef F_SETLK
# ifndef SEEK_SET
#  define SEEK_SET 0
# endif
	    struct flock lock_data;
	    lock_data.l_type = F_WRLCK;
	    lock_data.l_whence = SEEK_SET;
	    lock_data.l_start = lock_data.l_len = 0;
	    if (fcntl(pidFd, F_SETLK, &lock_data) == -1)
	    {
		if (errno == EAGAIN)
		    return oldpid;
		else
		    return -1;
	    }
#else
# ifdef LOCK_EX
	    if (flock (pidFd, LOCK_EX|LOCK_NB) == -1)
	    {
		if (errno == EWOULDBLOCK)
		    return oldpid;
		else
		    return -1;
	    }
# else
	    if (lockf (pidFd, F_TLOCK, 0) == -1)
	    {
		if (errno == EACCES)
		    return oldpid;
		else
		    return -1;
	    }
# endif
#endif
	}
	fprintf (pidFilePtr, "%d\n", getpid ());
	(void) fflush (pidFilePtr);
	RegisterCloseOnFork (pidFd);
    }
    return 0;
}

#if 0
void
UnlockPidFile (void)
{
    if (lockPidFile)
# ifdef F_SETLK
    {
	struct flock lock_data;
	lock_data.l_type = F_UNLCK;
	lock_data.l_whence = SEEK_SET;
	lock_data.l_start = lock_data.l_len = 0;
	(void) fcntl(pidFd, F_SETLK, &lock_data);
    }
# else
#  ifdef F_ULOCK
	lockf (pidFd, F_ULOCK, 0);
#  else
	flock (pidFd, LOCK_UN);
#  endif
# endif
    close (pidFd);
    fclose (pidFilePtr);
}
#endif

void SetTitle (char *name, ...)
{
#ifndef NOXDMTITLE
    char	*p = Title;
    int	left = TitleLen;
    char	*s;
    va_list	args;

    va_start(args, name);
    *p++ = '-';
    --left;
    s = name;
    while (s)
    {
	while (*s && left > 0)
	{
	    *p++ = *s++;
	    left--;
	}
	s = va_arg (args, char *);
    }
    while (left > 0)
    {
	*p++ = '\0';
	--left;
    }
    va_end(args);
#endif	
}

/* duplicate src; free old dst string */
int
ReStr (char **dst, const char *src)
{
    char *ndst = NULL;

    if (src) {
	int len = strlen (src) + 1;
	if (*dst && !memcmp (*dst, src, len))
	    return 1;
	if (!(ndst = malloc ((unsigned) len)))
	    return 0;
	memcpy (ndst, src, len);
    }
    if (*dst)
	free (*dst);
    *dst = ndst;
    return 1;
}

/* duplicate src */
int
StrDup (char **dst, const char *src)
{
    if (src) {
	int len = strlen (src) + 1;
	if (!(*dst = malloc ((unsigned) len)))
	    return 0;
	memcpy (*dst, src, len);
    } else
	*dst = NULL;
    return 1;
}

/* append src to dst */
int
StrApp(char **dst, const char *src)
{
    int olen, len;
    char *bk;

    if (*dst) {
	if (!src)
	    return 1;
	len = strlen(src) + 1;
	olen = strlen(*dst);
	if (!(bk = malloc (olen + len)))
	    return 0;
	memcpy(bk, *dst, olen);
	memcpy(bk + olen, src, len);
	free(*dst);
	*dst = bk;
	return 1;
    } else
	return StrDup (dst, src);
}
