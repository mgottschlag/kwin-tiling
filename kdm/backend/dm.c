/* $TOG: dm.c /main/73 1998/04/09 15:12:03 barstow $ */
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
/* $XFree86: xc/programs/xdm/dm.c,v 3.10 2000/04/27 16:26:50 eich Exp $ */

/*
 * xdm - display manager daemon
 * Author:  Keith Packard, MIT X Consortium
 *
 * display manager
 */

#define NEED_SIGNAL
#include "dm.h"
#include "dm_auth.h"
#include "dm_error.h"

#include <stdio.h>
#include <string.h>
#ifdef __NetBSD__
# include <sys/param.h>
#endif

#ifndef sigmask
# define sigmask(m)  (1 << ((m - 1)))
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

#if defined(CSRG_BASED) || defined(__DARWIN__)
# define BSD_UTMP
#endif

#if defined(BSD_UTMP) || !defined(sun)
# include <utmp.h>
# ifndef UTMP_FILE
#  define UTMP_FILE _PATH_UTMP
# endif
# define LOGSTAT_FILE UTMP_FILE
#else
# include <utmpx.h>
# ifndef ut_time
#  define ut_time ut_tv.tv_sec
# endif
# ifndef UTMPX_FILE
#  define UTMPX_FILE _PATH_UTMPX
# endif
# define LOGSTAT_FILE UTMPX_FILE
#endif
#ifdef linux
# include <sys/ioctl.h>
# include <linux/vt.h>
#endif

#if defined(SVR4) && !defined(SCO)
extern FILE    *fdopen();
#endif

static SIGVAL	SigHandler (int n);
static void	RescanConfigs (void);
static void	StartDisplays (void);
#define XS_KEEP 0
#define XS_RESTART 1
#define XS_RETRY 2
static void	ExitDisplay (struct display *d, int endState, int serverCmd, int goodExit);
static void	rStopDisplay (struct display *d, int endState);
static int	CheckUtmp (void);
static void	SwitchToTty (struct display *d);
static void	openFifo (int *fifofd, char **fifoPath, const char *dname);
static void	closeFifo (int *fifofd, const char *fifoPath);
static void	stoppen (int force);
static void	ReapChildren (void);
static void	MainLoop (void);

static int signalFds[2];

#if !defined(HAS_SETPROCTITLE) && !defined(NOXDMTITLE)
static char *Title;
static int TitleLen;
#endif

static int StorePid (void);

static int fifoFd = -1;
static char *fifoPath;

static int sdAction, Stopping;

char *prog, *progpath;

int
main (int argc, char **argv)
{
    int	oldpid, oldumask, fd, noDaemonMode;
    char *pt, *errorLogFile, **opts;

    /* make sure at least world write access is disabled */
    if (((oldumask = umask(022)) & 002) == 002)
	(void) umask (oldumask);

    /* give /dev/null as stdin */
    if ((fd = open ("/dev/null", O_RDONLY)) > 0) {
	dup2 (fd, 0);
	close (fd);
    }

    if (argv[0][0] == '/') {
	if (!StrDup (&progpath, argv[0]))
	    Panic ("Out of memory");
    } else
#ifdef linux
    {
	/* note that this will resolve symlinks ... */
	int len;
	char fullpath[PATH_MAX];
	if ((len = readlink ("/proc/self/exe", fullpath, sizeof(fullpath))) < 0)
	    Panic ("Invoke with full path specification or mount /proc");
	if (!StrNDup (&progpath, fullpath, len))
	    Panic ("Out of memory");
    }
#else
# if 0
	Panic ("Must be invoked with full path specification");
# else
    {
	char directory[PATH_MAX+1];
#  if !defined(X_NOT_POSIX) || defined(SYSV) || defined(WIN32)
	if (!getcwd(directory, sizeof(directory)))
	    Panic ("Can't find myself (getcwd failed)");
#  else
	if (!getwd(directory))
	    Panic ("Can't find myself (getwd failed)");
#  endif
	if (strchr(argv[0], '/'))
	    StrApp (&progpath, directory, "/", argv[0], (char *)0);
	else
	{
	    int len;
	    char *path, *pathe, *name, *thenam, nambuf[PATH_MAX+1];

	    if (!(path = getenv ("PATH")))
		Panic ("Can't find myself (no PATH)");
	    len = strlen (argv[0]);
	    name = nambuf + PATH_MAX - len;
	    memcpy (name, argv[0], len + 1);
	    *--name = '/';
	    do {
		if (!(pathe = strchr (path, ':')))
		    pathe = path + strlen (path);
		len = pathe - path;
		if (!len || (len == 1 && *path == '.'))
		{
		    len = strlen (directory);
		    path = directory;
		}
		thenam = name - len;
		if (thenam >= nambuf)
		{
		    memcpy (thenam, path, len);
		    if (!access (thenam, X_OK))
			goto found;
		}
		path = pathe;
	    } while (*path++ != '\0');
	    Panic ("Can't find myself (not in PATH)");
	  found:
	    if (!StrDup (&progpath, thenam))
		Panic ("Out of memory");
	}
    }
# endif
#endif
    prog = strrchr(progpath, '/') + 1;

#if !defined(HAS_SETPROCTITLE) && !defined(NOXDMTITLE)
    Title = argv[0];
    TitleLen = (argv[argc - 1] + strlen(argv[argc - 1])) - Title;
#endif

    /*
     * Parse command line options
     */
    noDaemonMode = getppid();
    errorLogFile = 0;
    opts = 0; *extStrArr (&opts) = (char *)"";
    while (*++argv) {
	if (**argv != '-')
	    break;
	pt = *argv + 1;
	if (*pt == '-')
	    pt++;
	if (!strcmp (pt, "help") || !strcmp (pt, "h")) {
	    printf ("Usage: %s [options] [tty]\n"
"  -daemon        - daemonize even when stared by init\n"
"  -nodaemon      - don't daemonize even when stared from command line\n"
"  -config <file> - use alternative master configuration file\n"
"  -xrm <res>     - override frontend-specific resource\n"
"  -debug <num>   - debug option bitfield\n"
"  -error <file>  - use alternative log file\n", prog);
	    exit (0);
	} else if (!strcmp (pt, "daemon"))
	    noDaemonMode = 0;
	else if (!strcmp (pt, "nodaemon"))
	    noDaemonMode = 1;
	else if (argv[1] && !strcmp (pt, "config"))
	    StrDup (opts, *++argv);
	else if (argv[1] && !strcmp (pt, "xrm"))
	    StrDup (extStrArr (&opts), *++argv);
	else if (argv[1] && !strcmp (pt, "debug"))
	    sscanf (*++argv, "%i", &debugLevel);
	else if (argv[1] && (!strcmp (pt, "error") || !strcmp (pt, "logfile")))
	    errorLogFile = *++argv;
	else {
	    fprintf (stderr, "\"%s\" is an unknown option or is missing a parameter\n", *argv);
	    exit (1);
	}
    }

    /*
     * Only allow root to run in non-debug mode to avoid problems
     */
    if (!debugLevel && getuid())
    {
	fprintf (stderr, "Only root wants to run %s\n", prog);
	exit (1);
    }

    InitErrorLog (errorLogFile);

    if (noDaemonMode != 1)
	BecomeDaemon ();

    /*
     * Step 1 - load configuration parameters
     */
    if (!InitResources (opts) || !LoadDMResources (TRUE))
	LogPanic ("Config reader failed. Aborting ...\n");

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

#if !defined(ARC4_RANDOM) && !defined(DEV_RANDOM)
    AddOtherEntropy ();
#endif

    /*
     * We used to clean up old authorization files here. As authDir is
     * supposed to be /var/run/xauth or /tmp, we needn't to care for it.
     */

#ifdef XDMCP
    init_session_id ();
    CreateWellKnownSockets ();
#else
    Debug ("not compiled for XDMCP\n");
#endif
    if (pipe (signalFds))
	LogPanic ("Unable to create signal notification pipe.\n");
    RegisterInput (signalFds[0]);
    RegisterCloseOnFork (signalFds[0]);
    RegisterCloseOnFork (signalFds[1]);
    (void) Signal (SIGTERM, SigHandler);
    (void) Signal (SIGINT, SigHandler);
    (void) Signal (SIGHUP, SigHandler);
    (void) Signal (SIGCHLD, SigHandler);
    (void) Signal (SIGALRM, SigHandler);

    /*
     * Step 2 - run a sub-daemon for each entry
     */
    openFifo (&fifoFd, &fifoPath, 0);
#ifdef XDMCP
    ScanAccessDatabase (0);
#endif
    ScanServers (0);
    StartDisplays ();
    MainLoop ();
    closeFifo (&fifoFd, fifoPath);
    if (sdAction)
    {
	if (Fork() <= 0)
	{
	    char *cmd = sdAction == SHUT_HALT ? cmdHalt : cmdReboot;
	    execute (parseArgs ((char **)0, cmd), (char **)0);
	    LogError ("Failed to execute shutdown command %\"s\n", cmd);
	    exit (1);
	}
	else
	{
#ifndef X_NOT_POSIX
	    sigset_t mask;
	    sigemptyset(&mask);
	    sigaddset(&mask, SIGCHLD);
	    sigaddset(&mask, SIGHUP);
	    sigaddset(&mask, SIGALRM);
	    sigsuspend(&mask);
#else
	    sigpause (sigmask (SIGCHLD) | sigmask (SIGHUP) | sigmask (SIGALRM));
#endif
	}
    }
    Debug ("nothing left to do, exiting\n");
    return 0;
}


enum utState { UtWait, UtActive };

#ifndef UT_LINESIZE
#define UT_LINESIZE 32
#endif

#define UT_LINESIZE_S stringify(UT_LINESIZE)

struct utmps {
    struct utmps *next;
    struct display *d;
    char line[UT_LINESIZE];
    time_t time;
    enum utState state;
    int hadSess;
#ifdef BSD_UTMP
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
    int nck, nextChk;
    time_t now;
    struct utmps *utp, **utpp;
    struct stat st;
#ifdef BSD_UTMP
    struct utmp ut[1];
#elif defined(sun)
    struct utmpx *ut;
#else
    struct utmp *ut;
#endif

    if (stat(LOGSTAT_FILE, &st))
    {
	LogError (LOGSTAT_FILE " not found - cannot use console mode\n");
	return 0;
    }
    if (!utmpList)
	return 1;
    time(&now);
    if (modtim != st.st_mtime)
    {
#ifdef BSD_UTMP
	int fd;
#endif

	Debug ("rescanning " LOGSTAT_FILE "\n");
#ifdef BSD_UTMP
	for (utp = utmpList; utp; utp = utp->next)
	    utp->checked = 0;
	if ((fd = open (UTMP_FILE, O_RDONLY)) < 0)
	{
	    LogError ("Cannot open " UTMP_FILE " - cannot use console mode\n");
	    return 0;
	}
	while (Reader (fd, ut, sizeof(ut[0])) == sizeof(ut[0]))
#elif defined(sun)
	setutxent();
	while ((ut = getutxent()))
#else
	setutent();
	while ((ut = getutent()))
#endif
	{
	    for (utp = utmpList; utp; utp = utp->next)
		if (!strncmp(utp->line, ut->ut_line, UT_LINESIZE))
		{
#ifdef BSD_UTMP
		    utp->checked = 1;
#else
		    if (ut->ut_type == LOGIN_PROCESS)
		    {
			Debug ("utmp entry for %." UT_LINESIZE_S "s marked waiting\n", utp->line);
			utp->state = UtWait;
		    }
		    else if (ut->ut_type != USER_PROCESS)
			break;
		    else
#endif
		    {
			utp->hadSess = 1;
			Debug ("utmp entry for %." UT_LINESIZE_S "s marked active\n", utp->line);
			utp->state = UtActive;
		    }
		    if (utp->time < ut->ut_time)
			utp->time = ut->ut_time;
		    break;
		}
	}
#ifdef BSD_UTMP
	close (fd);
	for (utp = utmpList; utp; utp = utp->next)
	    if (!utp->checked && utp->state == UtActive)
	    {
		utp->state = UtWait;
		utp->time = now;
		Debug ("utmp entry for %." UT_LINESIZE_S "s marked waiting\n", utp->line);
	    }
#elif defined(sun)
	endutxent();
#else
	endutent();
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
		Debug ("console login for %s at %." UT_LINESIZE_S "s timed out\n", 
		       d->name, utp->line);
		*utpp = utp->next;
		free (utp);
		d->status = notRunning;
		StartDisplays ();
		continue;
	    }
	    else
		nck = remains;
	}
	else
#ifdef BSD_UTMP
	    nck = (TIME_RELOG + 5) / 3;
#else
	    nck = TIME_RELOG;
#endif
	if (nck < nextChk)
	    nextChk = nck;
	utpp = &(*utpp)->next;
    }
    if (nextChk < 1000)
	alarm (nextChk);
    return 1;
}

static void
SwitchToTty (struct display *d)
{
    struct utmps *utp;

    if (!d->console || !d->console[0])
    {
	LogError("No console for %s specified - cannot use console mode\n",
		 d->name);
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
    d->status = textMode;
}

static void
StartRemoteLogin (struct display *d)
{
    char	**argv;
    int		pid;

    Debug ("StartRemoteLogin for %s\n", d->name);
    /* HACK: omitting LoadDisplayResources (d) here! */
    if (d->authorize)
	SetLocalAuthorization (d);
    switch (pid = Fork ()) {
    case 0:
	argv = d->serverArgv;
	if (d->authFile) {
	    argv = addStrArr (argv, "-auth", 5);
	    argv = addStrArr (argv, d->authFile, -1);
	}
	argv = addStrArr (argv, "-once", 5);
	argv = addStrArr (argv, "-query", 6);
	argv = addStrArr (argv, d->remoteHost, -1);
	if (!argv) {
	    LogError ("StartRemoteLogin: no arguments\n");
	    exit (1);
	}
	Debug ("exec %\"[s\n", argv);
	(void) execv (argv[0], argv);
	LogError ("X server %\"s cannot be executed\n", argv[0]);
	exit (1);
    case -1:
	LogError ("Forking X server for remote login failed: %s",
		  SysErrorMsg());
	d->status = notRunning;
	return;
    default:
	break;
    }
    Debug ("X server forked, pid %d\n", pid);
    d->serverPid = pid;

    d->status = remoteLogin;
}


static void
StopInactiveDisplay (struct display *d)
{
    if (d->userSess < 0)
	StopDisplay (d);
}

static void
stoppen (int force)
{
#ifdef XDMCP
    DestroyWellKnownSockets ();
#endif
    if (force)
	ForEachDisplay (StopDisplay);
    else
	ForEachDisplay (StopInactiveDisplay);
    Stopping = 1;
}


static void
doShutdown (int how, int when)
{
    switch (when) {
	case SHUT_TRYNOW:
	    if (AnyActiveDisplays ())
		return;
	    /* fallthrough */
	case SHUT_INTERACT:	/* XXX temp hack! */
	case SHUT_FORCENOW:
	    stoppen (TRUE);
	    break;
	default:
	    stoppen (FALSE);
	    break;
    }
    sdAction = how;
}


static void
openFifo (int *fifofd, char **fifopath, const char *dname)
{
    if (*fifoDir && *fifofd < 0) {
	if (mkdir (fifoDir, 0755)) {
	    if (errno != EEXIST) {
		LogError ("mkdir %\"s failed; no control FiFos will be available\n", 
			  fifoDir);
		return;
	    }
	} else
	    chmod (fifoDir, 0755); /* override umask */
	if (!*fifopath)
	    if (!StrApp (fifopath, fifoDir, dname ? "/xdmctl-" : "/xdmctl", 
			 dname, (char *)0))
		LogOutOfMem("openFifo");
	if (*fifopath) {
	    unlink (*fifopath);
	    if (mkfifo (*fifopath, 0) < 0)
		LogError ("Cannot create control FiFo %\"s\n", *fifopath);
	    else {
		chown (*fifopath, -1, fifoGroup);
		chmod (*fifopath, 0620);
		if ((*fifofd = open (*fifopath, O_RDWR | O_NONBLOCK)) >= 0) {
		    RegisterCloseOnFork (*fifofd);
		    RegisterInput (*fifofd);
		    return;
		}
		unlink (*fifopath);
		LogError ("Cannot open control FiFo %\"s\n", *fifopath);
	    }
	    free (*fifopath);
	    *fifopath = 0;
	}
    }
}

static void
closeFifo (int *fifofd, const char *fifopath)
{
    if (*fifofd >= 0) {
	UnregisterInput (*fifofd);
	CloseNClearCloseOnFork (*fifofd);
	*fifofd = -1;
	unlink (fifopath);
    }
}

static char **
splitCmd (const char *string, int len)
{
    const char *word;
    char **argv;

    argv = initStrArr (0);
    for (word = string; ; string++, len--)
	if (!len || *string == '\t') {
	    argv = addStrArr (argv, word, string - word);
	    if (!len)
		return argv;
	    word = string + 1;
	}
}

static void
setNLogin (struct display *d, 
	   const char *nuser, const char *npass, char *nargs, int rl)
{
    struct disphist *he = d->hstent;
    ReStr (&he->nuser, nuser);
    ReStr (&he->npass, npass);
    ReStr (&he->nargs, nargs);
    he->rLogin = rl;
    Debug ("set next login for %s, level %d\n", nuser, rl);
}

static void
processDPipe (struct display *d)
{
    char *user, *pass, *args;
    int cmd, how, ct, len;
    GTalk dpytalk;
    ARRAY8 ca, ha;

    dpytalk.pipe = &d->pipe;
    if (Setjmp (dpytalk.errjmp)) {
	StopDisplay (d);
	return;
    }
    GSet (&dpytalk);
    if (!GRecvCmd (&cmd)) {
	/* process already exited */
	UnregisterInput (d->pipe.rfd);
	return;
    }
    switch (cmd) {
    case D_User:
	d->userSess = GRecvInt ();
	break;
    case D_ReLogin:
	user = GRecvStr ();
	pass = GRecvStr ();
	args = GRecvStr ();
	setNLogin (d, user, pass, args, 1);
	free (args);
	free (pass);
	free (user);
	break;
    case D_Shutdown:
	how = GRecvInt ();
	doShutdown (how, GRecvInt ());
	/* XXX after this all displays could be gone */
	break;
    case D_ChooseHost:
	ca.data = (unsigned char *)GRecvArr (&len);
	ca.length = (CARD16) len;
	ct = GRecvInt ();
	ha.data = (unsigned char *)GRecvArr (&len);
	ha.length = (CARD16) len;
	RegisterIndirectChoice (&ca, ct, &ha);
	XdmcpDisposeARRAY8 (&ha);
	XdmcpDisposeARRAY8 (&ca);
	break;
    case D_RemoteHost:
	if (d->remoteHost)
	    free (d->remoteHost);
	d->remoteHost = GRecvStr ();
	break;
    default:
	LogError ("Internal error: unknown D_* command %d\n", cmd);
	StopDisplay (d);
	break;
    }
}

static int
parseSd (char **ar, int *how, int *when, int wdef)
{
    if (strcmp (ar[0], "shutdown"))
	return 0;
    *how = 0;
    if (!ar[1] || (!ar[2] && wdef < 0)) {
	LogInfo ("Missing argument(s) to FiFo command \"shutdown\"\n");
	return 1;
    }
    if (ar[2]) {
	if (!strcmp (ar[2], "forcenow"))
	    *when = SHUT_FORCENOW;
	else if (!strcmp (ar[2], "trynow"))
	    *when = SHUT_TRYNOW;
	else if (!strcmp (ar[2], "schedule"))
	    *when = SHUT_SCHEDULE;
	else {
	    LogInfo ("Invalid mode spec %\"s to FiFo command \"shutdown\"\n", ar[2]);
	    return 1;
	}
    } else
	*when = wdef;
    if (!strcmp (ar[1], "reboot"))
	*how = SHUT_REBOOT;
    else if (!strcmp (ar[1], "halt"))
	*how = SHUT_HALT;
    else
	LogInfo ("Invalid type spec %\"s to FiFo command \"shutdown\"\n", ar[1]);
    return 1;
}

static void
processDFifo (const char *buf, int len, void *ptr)
{
    struct display *d = (struct display *)ptr;
    char **ar = splitCmd (buf, len);
    int how, when;

    if (!ar[0])
	return;
    if (parseSd (ar, &how, &when, d->defSdMode))
    {
	if (!how)
	    return;
	if (d->allowShutdown == SHUT_NONE ||
	    (d->allowShutdown == SHUT_ROOT && d->userSess))
	{
	    LogInfo ("Display %s attempted FiFo command \"shutdown\"\n", d->name);
	    return;
	}
	if (when == SHUT_FORCENOW &&
	    (d->allowNuke == SHUT_NONE ||
	    (d->allowNuke == SHUT_ROOT && d->userSess)))
	{
	    LogInfo ("Display %s attempted FiFo command \"shutdown forcenow\"\n",
		     d->name);
	    return;
	}
	d->hstent->sd_how = how;
	d->hstent->sd_when = when;
    } else if (!strcmp (ar[0], "lock")) {
	d->hstent->lock = 1;
#ifdef AUTO_RESERVE
	if (AllLocalDisplaysLocked (0))
	    StartReserveDisplay (0);
#endif
    } else if (!strcmp (ar[0], "unlock")) {
	d->hstent->lock = 0;
#ifdef AUTO_RESERVE
	ReapReserveDisplays ();
#endif
    } else if (!strcmp (ar[0], "reserve")) {
	if ((d->displayType & d_location) == dLocal) {
	    int lt = 0;
	    if (ar[1])
		lt = atoi (ar[1]);
	    StartReserveDisplay (lt ? lt : 60); /* XXX maybe make configurable? */
	} else
	    LogInfo ("Remote display %s attempted FiFo command \"reserve\"\n", d->name);
    } else if (!strcmp (ar[0], "suicide")) {
	if (d->pid != -1) {
	    TerminateProcess (d->pid, SIGTERM);
	    d->status = raiser;
	}
    } else
	LogInfo ("Invalid FiFo command %\"s from display %s\n", ar[0], d->name);
    freeStrArr (ar);
}

static void
processFifo (const char *buf, int len, void *ptr ATTR_UNUSED)
{
    struct display *d;
    char **ar = splitCmd (buf, len), *args, *asp, *adp;
    int how, when;

    if (!ar[0])
	return;
    if (parseSd (ar, &how, &when, -1)) {
	if (!how)
	    return;
	if (!fifoAllowShutdown)
	{
	    LogInfo ("System shutdown via command FiFo forbidden\n");
	    return;
	}
	if (when == SHUT_FORCENOW && !fifoAllowNuke)
	{
	    LogInfo ("Forced system shutdown via command FiFo forbidden\n");
	    return;
	}
	doShutdown (how, when);
    } else if (!strcmp (ar[0], "login")) {
	if (arrLen (ar) < 5) {
	    LogInfo ("Missing argument(s) to FiFo command %s\"\n", ar[0]);
	    return;
	}
	if (!(d = FindDisplayByName (ar[1]))) {
	    LogInfo ("Display %s in FiFo command %\"s not found\n", ar[1], ar[0]);
	    return;
	}
	if (ar[5] && StrDup (&args, ar[5])) {
	    for (asp = adp = args; *asp; asp++, adp++)
		if (*asp == '\\')
		    switch (*++asp) {
		    case 0: asp--; /* fallthrough */
		    case '\\': *adp = '\\'; break;
		    case 'n': *adp = '\n'; break;
		    case 't': *adp = '\t'; break;
		    default: *adp++ = '\\'; *adp = *asp; break;
		    }
		else
		    *adp = *asp;
	    *adp = 0;
	    setNLogin (d, ar[3], ar[4], args, 2);
	    free (args);
	} else
	    setNLogin (d, ar[3], ar[4], 0, 2);
	if (d->pid != -1) {
	    if (d->userSess < 0 || !strcmp (ar[2], "now")) {
		TerminateProcess (d->pid, SIGTERM);
		d->status = raiser;
	    }
	} else
	    StartDisplay (d);
    } else
	LogInfo ("Invalid FiFo command %\"s\n", ar[0]);
    freeStrArr (ar);
}


static void
MarkDisplay (struct display *d)
{
    d->stillThere = 0;
}

static void
RescanConfigs (void)
{
    LogInfo ("Rescanning all config files\n");
    LoadDMResources (1);
    ForEachDisplay (MarkDisplay);
    ScanServers (1);
#ifdef XDMCP
    ScanAccessDatabase (1);
#endif
    StartDisplays ();
}

static void
RescanIfMod (void)
{
    LoadDMResources (0);
    ScanServers (0);
#ifdef XDMCP
    ScanAccessDatabase (0);
#endif
}


static void
ReapChildren (void)
{
    int		pid;
    struct display	*d;
    waitType	status;

#ifndef X_NOT_POSIX
    while ((pid = waitpid (-1, &status, WNOHANG)) > 0)
#else
    while ((pid = wait3 (&status, WNOHANG, (struct rusage *) 0)) > 0)
#endif
    {
	Debug ("manager wait returns  pid %d  sig %d  core %d  code %d\n",
	       pid, waitSig(status), waitCore(status), waitCode(status));
	if (autoRescan)
	    RescanIfMod ();
	/* SUPPRESS 560 */
	if ((d = FindDisplayByPid (pid))) {
	    d->pid = -1;
	    UnregisterInput (d->pipe.rfd);
	    GClosen (&d->pipe);
	    closeFifo (&d->fifofd, d->fifoPath);
	    switch (waitVal (status)) {
	    case EX_TEXTLOGIN:
		Debug ("display exited with EX_TEXTLOGIN\n");
		ExitDisplay (d, DS_TEXTMODE, 0, 0);
		break;
	    case EX_REMOTE:
		Debug ("display exited with EX_REMOTE\n");
		ExitDisplay (d, DS_REMOTE, 0, 0);
		break;
#ifdef AUTO_RESERVE
	    case EX_RESERVE:
		Debug ("display exited with EX_RESERVE\n");
		ExitDisplay (d, DS_RESERVE, 0, 0);
		break;
#endif
	    case EX_NORMAL:
		/* (any type of) session ended */
		Debug ("display exited with EX_NORMAL\n");
		if ((d->displayType & d_lifetime) == dReserve
#ifdef AUTO_RESERVE
		     && !AllLocalDisplaysLocked (d)
#endif
		   )
		    ExitDisplay (d, DS_RESERVE, 0, 0);
		else
		    ExitDisplay (d, DS_RESTART, XS_KEEP, TRUE);
		break;
#if 0
	    case EX_REMANAGE_DPY:
		/* user session ended */
		Debug ("display exited with EX_REMANAGE_DPY\n");
		ExitDisplay (d, DS_RESTART, XS_KEEP, TRUE);
		break;
#endif
	    case EX_OPENFAILED_DPY:
		/* WaitForServer() failed */
		LogError ("Display %s cannot be opened\n", d->name);
#ifdef XDMCP
		/*
		 * no display connection was ever made, tell the
		 * terminal that the open attempt failed
		 */
		if ((d->displayType & d_origin) == dFromXDMCP)
		    SendFailed (d, "cannot open display");
#endif
		ExitDisplay (d, DS_RESTART, XS_RETRY, FALSE);
		break;
	    case waitCompose (SIGTERM,0,0):
		/* killed before/during WaitForServer()
		   - local Xserver died
		   - display stopped (is zombie)
		   - "login now" and "suicide" pipe commands (is raiser)
		*/
		Debug ("display exited on SIGTERM\n");
		ExitDisplay (d, DS_RESTART, XS_RETRY, FALSE);
		break;
	    case EX_AL_RESERVER_DPY:
		/* - killed after WaitForServer()
		   - Xserver dead after remote session exit
		*/
		Debug ("display exited with EX_AL_RESERVER_DPY\n");
		ExitDisplay (d, DS_RESTART, XS_RESTART, FALSE);
		break;
	    case EX_RESERVER_DPY:
		/* induced by greeter:
		   - could not secure display
		   - requested by user
		*/
		Debug ("display exited with EX_RESERVER_DPY\n");
		ExitDisplay (d, DS_RESTART, XS_RESTART, TRUE);
		break;
	    case EX_UNMANAGE_DPY:
		/* some fatal error */
		Debug ("display exited with EX_UNMANAGE_DPY\n");
		ExitDisplay (d, DS_REMOVE, 0, 0);
		break;
	    default:
		/* prolly crash */
		LogError ("Unknown session exit code %d (sig %d) from manager process\n",
			  waitCode (status), waitSig (status));
		ExitDisplay (d, DS_REMOVE, 0, 0);
		break;
	    }
	}
	/* SUPPRESS 560 */
	else if ((d = FindDisplayByServerPid (pid)))
	{
	    d->serverPid = -1;
	    switch (d->status)
	    {
	    case zombie:
		Debug ("zombie X server for display %s reaped\n", d->name);
		rStopDisplay (d, d->zstatus);
		break;
	    case phoenix:
		Debug ("phoenix X server arises, restarting display %s\n",
		       d->name);
		d->status = notRunning;
		break;
	    case remoteLogin:
		Debug ("remote login X server for display %s exited,"
		       " restarting display\n", d->name);
		d->status = notRunning;
		break;
	    case raiser:
		/* this should not happen */
		d->status = notRunning;
		LogError ("X server for display %s terminated unexpectedly\n",
			  d->name);
		/* don't kill again */
		break;
	    case running:
		LogError ("X server for display %s terminated unexpectedly\n",
			  d->name);
		if (d->pid != -1)
		{
		    Debug ("terminating session pid %d\n", d->pid);
		    TerminateProcess (d->pid, SIGTERM);
		}
		break;
	    case notRunning:
	    case textMode:
	    case reserve:
		/* this cannot happen */
		Debug ("X server exited for passive (%d) session on display %s\n",
		       (int) d->status, d->name);
		break;
	    }
	}
	else
	{
	    Debug ("unknown child termination\n");
	}
    }
    StartDisplays ();
#if !defined(ARC4_RANDOM) && !defined(DEV_RANDOM)
    AddOtherEntropy ();
#endif
}

FD_TYPE	WellKnownSocketsMask;
int	WellKnownSocketsMax;
int	WellKnownSocketsCount;

void
RegisterInput (int fd)
{
    /* can be omited, as it is always called right after opening a socket
    if (!FD_ISSET (fd, &WellKnownSocketsMask))
    */
    {
	FD_SET (fd, &WellKnownSocketsMask);
	if (fd > WellKnownSocketsMax)
	    WellKnownSocketsMax = fd;
	WellKnownSocketsCount++;
    }
}

void
UnregisterInput (int fd)
{
    /* the check _is_ necessary, as some handles are unregistered before
       the regular close sequence.
    */
    if (FD_ISSET (fd, &WellKnownSocketsMask))
    {
	FD_CLR (fd, &WellKnownSocketsMask);
	WellKnownSocketsCount--;
    }
}

static SIGVAL
SigHandler (int n)
{
    int olderrno = errno;
    char buf = (char) n;
    Debug ("caught signal %d\n", n);
    write (signalFds[1], &buf, 1);
#ifdef SIGNALS_RESET_WHEN_CAUGHT
    (void) Signal (n, SigHandler);
#endif
    errno = olderrno;
}

static void
MainLoop (void)
{
    struct display *d;
    int nready;
    char buf;
    FD_TYPE reads;

    Debug ("MainLoop\n");
    while (
#ifdef XDMCP
	   AnyWellKnownSockets() ||
#endif
	   (Stopping ? AnyRunningDisplays() : AnyDisplaysLeft ()))
    {
	reads = WellKnownSocketsMask;
#ifdef hpux
	nready = select (WellKnownSocketsMax + 1, (int*)reads.fds_bits, 0, 0, 0);
#else
	nready = select (WellKnownSocketsMax + 1, &reads, 0, 0, 0);
#endif
	Debug ("select returns %d\n", nready);
#if !defined(ARC4_RANDOM) && !defined(DEV_RANDOM)
	AddTimerEntropy ();
#endif
	if (nready > 0)
	{
	    /*
	     * we restart after the first handled fd, as
	     * a) it makes things simpler
	     * b) the probability that multiple fds trigger at once is
	     *    ridiculously small. we handle it in the next iteration.
	     */
	    /* XXX a cleaner solution would be a callback mechanism */
	    if (FD_ISSET (signalFds[0], &reads))
	    {
		if (read (signalFds[0], &buf, 1) != 1)
		    LogPanic ("Signal notification pipe broken.\n");
		switch (buf) {
		case SIGTERM:
		case SIGINT:
		    Debug ("shutting down entire manager\n");
		    stoppen (TRUE);
		    break;
		case SIGHUP:
		    RescanConfigs ();
		    break;
		case SIGCHLD:
		    ReapChildren ();
		    break;
		case SIGALRM:
		    CheckUtmp ();
		    break;
		}
		continue;
	    }
#ifdef XDMCP
	    if (xdmcpFd >= 0 && FD_ISSET (xdmcpFd, &reads))
	    {
		ProcessRequestSocket ();
		continue;
	    }
#endif	/* XDMCP */
	    if (fifoFd >= 0 && FD_ISSET (fifoFd, &reads))
	    {
		FdGetsCall (fifoFd, processFifo, 0);
		continue;
	    }
	    /* Must be last (because of the breaks)! */
	    for (d = displays; d; d = d->next)
	    {
		if (d->fifofd >= 0 && FD_ISSET (d->fifofd, &reads))
		{
		    FdGetsCall (d->fifofd, processDFifo, d);
		    break;
		}
		if (d->pipe.rfd >= 0 && FD_ISSET (d->pipe.rfd, &reads))
		{
		    processDPipe (d);
		    break;
		}
	    }
	}
    }
}

static void
CheckDisplayStatus (struct display *d)
{
    if ((d->displayType & d_origin) == dFromFile)
    {
	if (d->stillThere) {
	    if (d->status == notRunning)
		StartDisplay (d);
	} else
	    StopDisplay (d);
    }
}

static void
StartDisplays (void)
{
    ForEachDisplay (CheckDisplayStatus);
    CloseGetter ();
}

void
StartDisplay (struct display *d)
{
    char	*cname;
    int		pid;

    if (sdAction)
    {
	Debug ("stopping display %s because shutdown is scheduled\n", d->name);
	StopDisplay (d);
	return;
    }

    Debug ("StartDisplay %s, try %d\n", d->name, d->startTries + 1);
    time (&d->lastStart);

    if (!LoadDisplayResources (d))
    {
	LogError ("Unable to read configuration for display %s; stopping it.\n", 
		  d->name);
	StopDisplay (d);
	return;
    }

    if ((d->displayType & d_location) == dLocal)
    {
	/* don't bother pinging local displays; we'll
	 * certainly notice when they exit
	 */
	d->pingInterval = 0;
	if (d->authorize)
	{
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
	    LogError ("X server for display %s can't be started, session disabled\n", d->name);
	    StopDisplay (d);
	    return;
	}
    }
    else
    {
	/* this will only happen when using XDMCP */
	if (d->authorizations)
	    SaveServerAuthorizations (d, d->authorizations, d->authNum);
    }
    openFifo (&d->fifofd, &d->fifoPath, d->name);
#ifdef nofork_session
    if (!nofork_session) {
#endif
	Debug ("forking session\n");
	ASPrintf (&cname, "sub-daemon for display %s", d->name);
	pid = GFork (&d->pipe, (char *)"master daemon", cname);
#ifdef nofork_session
    } else {
	Debug ("not forking session\n");
	CloseGetter ();
	pid = -2;
    }
#endif
    switch (pid)
    {
    case 0:
	SetTitle (d->name);
	if (debugLevel & DEBUG_WSESS)
	    sleep (100);
	mstrtalk.pipe = &d->pipe;
#ifdef nofork_session
    case -2:
#endif
	(void) Signal (SIGPIPE, SIG_IGN);
	SetAuthorization (d);
	WaitForServer (d);
	ManageSession (d);
	/* NOTREACHED */
    case -1:
	break;
    default:
	Debug ("forked session, pid %d\n", pid);

	/* (void) fcntl (d->pipe.rfd, F_SETFL, O_NONBLOCK); */
	RegisterInput (d->pipe.rfd);

	d->pid = pid;
	d->status = running;
	d->hstent->lock = d->hstent->rLogin = d->hstent->goodExit = 
	    d->hstent->sd_how = d->hstent->sd_when = 0;
	break;
    }
}

/*
 * transition from running to zombie, textmode, reserve or deleted
 */

static void
rStopDisplay (struct display *d, int endState)
{
    Debug ("stopping display %s to state %d\n", d->name, endState);
    if (d->serverPid != -1 || d->pid != -1)
    {
	if (d->pid != -1)
	    TerminateProcess (d->pid, SIGTERM);
	if (d->serverPid != -1)
	    TerminateProcess (d->serverPid, d->termSignal);
	d->status = zombie;
	d->zstatus = endState;
	Debug (" zombiefied\n");
    }
    else if (endState == DS_TEXTMODE)
	SwitchToTty (d);
    else if (endState == DS_RESERVE)
	d->status = reserve;
    else if (endState == DS_REMOTE)
	StartRemoteLogin (d);
    else
	RemoveDisplay (d);
}

void
StopDisplay (struct display *d)
{
    rStopDisplay (d, DS_REMOVE);
}

static void
ExitDisplay (
    struct display	*d, 
    int			endState,
    int			serverCmd,
    int			goodExit)
{
    struct disphist	*he;

    if (d->status == raiser)
    {
	serverCmd = XS_KEEP;
	goodExit = TRUE;
    }

    Debug ("ExitDisplay %s, "
	   "endState = %d, serverCmd = %d, GoodExit = %d\n",
	   d->name, endState, serverCmd, goodExit);

    d->userSess = -1;
    he = d->hstent;
    time (&he->lastExit);
    he->goodExit = goodExit;
    if (d->status == zombie)
	rStopDisplay (d, d->zstatus);
    else {
	if (endState != DS_RESTART ||
	    (d->displayType & d_origin) != dFromFile)
	{
	    rStopDisplay (d, endState);
	}
	else
	{
	    if (serverCmd == XS_RETRY)
	    {
		if ((d->displayType & d_location) == dLocal)
		{
		    if (he->lastExit - d->lastStart < 120)
		    {
			LogError ("Unable to fire up local display %s;"
				  " disabling.\n", d->name);
			StopDisplay (d);
			goto bork;
		    }
		}
		else
		{
		    if (++d->startTries > d->startAttempts)
		    {
			LogError ("Disabling foreign display %s"
				 " (too many attempts)\n", d->name);
			StopDisplay (d);
			goto bork;
		    }
		}
	    }
	    else
		d->startTries = 0;
	    if (d->serverPid != -1 && (serverCmd != XS_KEEP || d->terminateServer))
	    {
		Debug ("killing X server for %s\n", d->name);
		TerminateProcess (d->serverPid, d->termSignal);
		d->status = phoenix;
	    }
	    else
	    {
		d->status = notRunning;
	    }
	}
    }
  bork:
    if (he->sd_how)
    {
	doShutdown (he->sd_how, he->sd_when);
	he->sd_how = 0;
    }
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
	fprintf (pidFilePtr, "%ld\n", (long)getpid ());
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

void
SetTitle (const char *name)
{
#if !defined(HAS_SETPROCTITLE) && !defined(NOXDMTITLE)
    char *p;
    int left;
#endif

    ASPrintf (&prog, "%s: %s", prog, name);
    ReInitErrorLog ();
#ifdef HAS_SETPROCTITLE
    setproctitle ("%s", name);
#elif !defined(NOXDMTITLE)
    p = Title;
    left = TitleLen;

    *p++ = '-';
    --left;
    while (*name && left > 0)
    {
	*p++ = *name++;
	--left;
    }
    while (left > 0)
    {
	*p++ = '\0';
	--left;
    }
#endif
}
