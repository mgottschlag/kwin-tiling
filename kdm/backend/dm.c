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

#include "dm.h"
#include "dm_auth.h"
#include "dm_error.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <signal.h>
#include <sys/stat.h>

#ifdef HAVE_VTS
# include <sys/ioctl.h>
# include <sys/vt.h>
#endif

static void sigHandler(int n);
static int scanConfigs(int force);
static void startDisplay(struct display *d);
static void startDisplays(void);
#define XS_KEEP 0
#define XS_RESTART 1
#define XS_RETRY 2
static void exitDisplay(struct display *d, int endState, int serverCmd, int goodExit);
static void rStopDisplay(struct display *d, int endState);
static void mainLoop(void);

static int signalFds[2];

#if !defined(HAVE_SETPROCTITLE) && !defined(NOXDMTITLE)
static char *title;
static int titleLen;
#endif

static int storePid(void);

static int stopping;
SdRec sdRec = { 0, 0, 0, TO_INF, TO_INF, 0, 0, 0 };

time_t now;
#ifndef nowMonotonic
int nowMonotonic;
#endif

#if KDM_LIBEXEC_STRIP != -1
char *progpath;
#endif
#if KDM_LIBEXEC_STRIP
char *progname;
#endif
char *prog;

int
main(int argc, char **argv)
{
    int oldpid, oldumask, fd, parentPid;
    char *pt, *errorLogFile, **opts;

    /* make sure at least world write access is disabled */
    if (((oldumask = umask(022)) & 002) == 002)
        (void)umask(oldumask);

    /* give /dev/null as stdin */
    if ((fd = open("/dev/null", O_RDONLY)) > 0) {
        dup2(fd, 0);
        close(fd);
    }
    if (fcntl(1, F_GETFD) < 0)
        dup2(0, 1);
    if (fcntl(2, F_GETFD) < 0)
        dup2(0, 2);

#ifndef nowMonotonic
    nowMonotonic = sysconf(_SC_MONOTONIC_CLOCK) >= 200112L;
#endif

#if KDM_LIBEXEC_STRIP == -1
    prog = strrchr(argv[0], '/');
    progname = prog = prog ? prog + 1 : argv[0];
#else
    if (argv[0][0] == '/') {
        if (!strDup(&progpath, argv[0]))
            panic("Out of memory");
    } else
# ifdef __linux__
    {
        /* note that this will resolve symlinks ... */
        int len;
        char fullpath[PATH_MAX];
        if ((len = readlink("/proc/self/exe", fullpath, sizeof(fullpath))) < 0)
            panic("Invoke with full path specification or mount /proc");
        if (!strNDup(&progpath, fullpath, len))
            panic("Out of memory");
    }
# else
#  if 0
        panic("Must be invoked with full path specification");
#  else
    {
        char directory[PATH_MAX+1];
        if (!getcwd(directory, sizeof(directory)))
            panic("Can't find myself (getcwd failed)");
        if (strchr(argv[0], '/')) {
            strApp(&progpath, directory, "/", argv[0], (char *)0);
        } else {
            int len;
            char *path, *pathe, *name, *thenam, nambuf[PATH_MAX+1];

            if (!(path = getenv("PATH")))
                panic("Can't find myself (no PATH)");
            len = strlen(argv[0]);
            name = nambuf + PATH_MAX - len;
            memcpy(name, argv[0], len + 1);
            *--name = '/';
            do {
                if (!(pathe = strchr(path, ':')))
                    pathe = path + strlen(path);
                len = pathe - path;
                if (!len || (len == 1 && *path == '.')) {
                    len = strlen(directory);
                    path = directory;
                }
                thenam = name - len;
                if (thenam >= nambuf) {
                    memcpy(thenam, path, len);
                    if (!access(thenam, X_OK))
                        goto found;
                }
                path = pathe;
            } while (*path++ != '\0');
            panic("Can't find myself (not in PATH)");
          found:
            if (!strDup(&progpath, thenam))
                panic("Out of memory");
        }
    }
#  endif
# endif
    prog = strrchr(progpath, '/') + 1;
# if KDM_LIBEXEC_STRIP
    for (progname = pt = prog, fd = 0; fd < KDM_LIBEXEC_STRIP + 1; fd++) {
        for (;;) {
            pt--;
            if (pt == progpath)
                panic("Executable is obviously located outside BINDIR");
            if (*pt == '/')
                break;
        }
    }
    *pt = 0;
# endif
#endif

#if !defined(HAVE_SETPROCTITLE) && !defined(NOXDMTITLE)
    title = argv[0];
    titleLen = (argv[argc - 1] + strlen(argv[argc - 1])) - title;
#endif

    /*
     * Parse command line options
     */
    parentPid = getppid();
    errorLogFile = 0;
    if (!(opts = Malloc(2 * sizeof(char *))))
        return 1;
    opts[0] = (char *)"";
    opts[1] = 0;
    while (*++argv) {
        if (**argv != '-')
            break;
        pt = *argv + 1;
        if (*pt == '-')
            pt++;
        if (!strcmp(pt, "help") || !strcmp(pt, "h")) {
            printf("Usage: %s [options] [tty]\n"
"  -daemon\t  - Daemonize even when started by init\n"
"  -nodaemon\t  - Do not daemonize even when started from command line\n"
"  -config <file>  - Use alternative master configuration file\n"
"  -xrm <res>\t  - Override frontend-specific resource\n"
"  -error <file>\t  - Use alternative log file\n"
"  -debug <num>\t  - Debug option bitfield:\n"
"\t\t\t0x1 - core log\n"
"\t\t\t0x2 - config reader log\n"
"\t\t\t0x4 - greeter log\n"
"\t\t\t0x8 - IPC log\n"
"\t\t\t0x10 - session sub-daemon post-fork delay\n"
"\t\t\t0x20 - config reader post-start delay\n"
"\t\t\t0x40 - greeter post-start delay\n"
"\t\t\t0x80 - do not use syslog\n"
"\t\t\t0x100 - core Xauth log\n"
"\t\t\t0x200 - debug greeter theming\n"
"\t\t\t0x400 - valgrind config reader and greeter\n"
"\t\t\t0x800 - strace config reader and greeter\n"
                    , prog);
            exit(0);
        } else if (!strcmp(pt, "daemon")) {
            parentPid = 0;
        } else if (!strcmp(pt, "nodaemon")) {
            parentPid = 1;
        } else if (argv[1] && !strcmp(pt, "config")) {
            strDup(opts, *++argv);
        } else if (argv[1] && !strcmp(pt, "xrm")) {
            opts = addStrArr(opts, *++argv, -1);
        } else if (argv[1] && !strcmp(pt, "debug")) {
            sscanf(*++argv, "%i", &debugLevel);
        } else if (argv[1] && (!strcmp(pt, "error") || !strcmp(pt, "logfile"))) {
            errorLogFile = *++argv;
        } else {
            fprintf(stderr, "\"%s\" is an unknown option or is missing a parameter\n", *argv);
            exit(1);
        }
    }

    /*
     * Only allow root to run in non-debug mode to avoid problems
     */
    if (!debugLevel && getuid()) {
        fprintf(stderr, "Only root wants to run %s\n", prog);
        exit(1);
    }

    initErrorLog(errorLogFile);

    if (parentPid != 1)
        becomeDaemon();

    /*
     * Step 1 - load configuration parameters
     */
    if (!initResources(opts) || scanConfigs(False) < 0)
        logPanic("Config reader failed. Aborting ...\n");

    /* SUPPRESS 560 */
    if ((oldpid = storePid())) {
        if (oldpid == -1)
            logError("Cannot create/lock pid file %s\n", pidFile);
        else
            logError("Cannot lock pid file %s, another xdm is running (pid %d)\n",
                     pidFile, oldpid);
        exit(1);
    }

#ifdef NEED_ENTROPY
    addOtherEntropy();
#endif

    /*
     * We used to clean up old authorization files here. As authDir is
     * supposed to be /var/run/xauth or /tmp, we needn't to care for it.
     */

#ifdef XDMCP
    initXdmcp();
#else
    debug("not compiled for XDMCP\n");
#endif
    if (pipe(signalFds))
        logPanic("Unable to create signal notification pipe.\n");
    registerInput(signalFds[0]);
    registerCloseOnFork(signalFds[0]);
    registerCloseOnFork(signalFds[1]);
    (void)Signal(SIGTERM, sigHandler);
    (void)Signal(SIGINT, sigHandler);
    (void)Signal(SIGHUP, sigHandler);
    (void)Signal(SIGCHLD, sigHandler);
    (void)Signal(SIGUSR1, sigHandler);

    /*
     * Step 2 - run a sub-daemon for each entry
     */
    openCtrl(0);
#ifdef XDMCP
    updateListenSockets();
#endif
    mainLoop();
    closeCtrl(0);
    if (sdRec.how) {
        int pid;
        commitBootOption();
        if (Fork(&pid) <= 0) {
            char *cmd = sdRec.how == SHUT_HALT ? cmdHalt : cmdReboot;
            execute(parseArgs((char **)0, cmd), (char **)0);
            logError("Failed to execute shutdown command %\"s\n", cmd);
            exit(1);
        } else {
            sigset_t mask;
            sigemptyset(&mask);
            sigaddset(&mask, SIGCHLD);
            sigaddset(&mask, SIGHUP);
            sigsuspend(&mask);
        }
    }
    debug("nothing left to do, exiting\n");
    return 0;
}

void
updateNow(void)
{
#if (_POSIX_MONOTONIC_CLOCK >= 0)
    if (nowMonotonic) {
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        /* Linux' monotonic clock starts at zero, but this is assumed to mean "long ago". */
        now = ts.tv_sec + 10000;
    } else
#endif
        time(&now);
}


#ifdef HAVE_VTS
int
TTYtoVT(const char *tty)
{
    return memcmp(tty, "tty", 3) ? 0 : atoi(tty + 3);
}

int
activateVT(int vt)
{
    int ret = False;
    int con = open("/dev/console", O_RDONLY);
    if (con >= 0) {
        if (!ioctl(con, VT_ACTIVATE, vt))
            ret = True;
        close(con);
    }
    return ret;
}


static void
wakeDisplay(struct display *d)
{
    if (d->status == textMode)
        d->status = (d->displayType & d_lifetime) == dReserve ? reserve : notRunning;
}
#endif

enum utState {
    UtDead,   /* no such entry */
    UtWait,   /* waiting for user login */
    UtActive  /* user logged in */
};

struct utmps {
#ifndef HAVE_VTS
    struct utmps *next;
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

void
wakeDisplays(void)
{
#ifdef HAVE_VTS
    forEachDisplay(wakeDisplay);
    free(utmpList);
    utmpList = 0;
#else
    struct utmps *utp;
    while ((utp = utmpList)) {
        utp->d->status = notRunning;
        utmpList = utp->next;
        free(utp);
    }
#endif
}

static void
checkUtmp(void)
{
    static time_t modtim;
    time_t nck;
    time_t ends;
    struct utmps *utp;
#ifndef HAVE_VTS
    struct utmps **utpp;
#endif
    struct stat st;
#ifdef BSD_UTMP
    int fd;
    struct utmp ut[1];
#else
    STRUCTUTMP *ut;
#endif

    if (!utmpList)
        return;
    if (stat(UTMP_FILE, &st)) {
        logError(UTMP_FILE " not found - cannot use console mode\n");
        wakeDisplays();
        return;
    }
    if (modtim != st.st_mtime) {
        debug("rescanning " UTMP_FILE "\n");
#ifdef HAVE_VTS
        utp = utmpList;
#else
        for (utp = utmpList; utp; utp = utp->next)
#endif
            utp->state = UtDead;
#ifdef BSD_UTMP
        if ((fd = open(UTMP_FILE, O_RDONLY)) < 0) {
            logError("Cannot open " UTMP_FILE " - cannot use console mode\n");
            wakeDisplays();
            return;
        }
        while (reader(fd, ut, sizeof(ut[0])) == sizeof(ut[0]))
#else
        SETUTENT();
        while ((ut = GETUTENT()))
#endif
        {
            /* first, match the tty to a utemps */
#ifdef HAVE_VTS
            char **line;
            for (line = consoleTTYs; *line; line++)
                if (!strncmp(*line, ut->ut_line, sizeof(ut->ut_line)))
#else
            for (utp = utmpList; utp; utp = utp->next)
                if (!strncmp(utp->d->console, ut->ut_line, sizeof(ut->ut_line)))
#endif
                    goto hitlin;
            continue;
          hitlin:
            /* then, update the utemps accordingly */
#ifdef BSD_UTMP
            if (!*ut->ut_user) {
#else
            if (ut->ut_type != USER_PROCESS) {
#endif
#ifdef HAVE_VTS
                /* don't allow "downgrading" the singular utemps */
                if (utp->state == UtActive)
                    continue;
#endif
                utp->state = UtWait;
            } else {
                utp->hadSess = True;
                utp->state = UtActive;
            }
#ifdef HAVE_VTS
            /* tty with latest activity wins */
            if (utp->time < ut->ut_time)
#endif
                utp->time = ut->ut_time;
        }
#ifdef BSD_UTMP
        close(fd);
#else
        ENDUTENT();
#endif
        modtim = st.st_mtime;
    }
#ifdef HAVE_VTS
    utp = utmpList;
#else
    for (utpp = &utmpList; (utp = *utpp);) {
#endif
        if (utp->state != UtActive) {
            if (utp->state == UtDead) /* shouldn't happen ... */
                utp->time = 0;
            ends = utp->time + (utp->hadSess ? TIME_RELOG : TIME_LOG);
            if (ends <= now) {
#ifdef HAVE_VTS
                wakeDisplays();
                debug("console login timed out\n");
                return;
#else
                utp->d->status = notRunning;
                debug("console login for %s at %s timed out\n",
                       utp->d->name, utp->d->console);
                *utpp = utp->next;
                free(utp);
                continue;
#endif
            } else
                nck = ends;
        } else
            nck = TIME_RELOG + now;
        if (nck < utmpTimeout)
            utmpTimeout = nck;
#ifndef HAVE_VTS
        utpp = &(*utpp)->next;
    }
#endif
}

static void
#ifdef HAVE_VTS
switchToTTY(void)
#else
switchToTTY(struct display *d)
#endif
{
    struct utmps *utp;
#ifdef HAVE_VTS
    int vt;
#endif

    if (!(utp = Malloc(sizeof(*utp)))) {
#ifdef HAVE_VTS
        wakeDisplays();
#else
        d->status = notRunning;
#endif
        return;
    }
#ifndef HAVE_VTS
    d->status = textMode;
    utp->d = d;
    utp->next = utmpList;
#endif
    utp->time = now;
    utp->hadSess = False;
    utmpList = utp;
    checkUtmp();

#ifdef HAVE_VTS
    if ((vt = TTYtoVT(*consoleTTYs)))
        activateVT(vt);
#endif
}

#ifdef HAVE_VTS
static void
stopToTTY(struct display *d)
{
    if ((d->displayType & d_location) == dLocal)
        switch (d->status) {
        default:
            rStopDisplay(d, DS_TEXTMODE | DS_SCHEDULE);
        case reserve:
        case textMode:
            break;
        }
}

static void
checkTTYMode(void)
{
    struct display *d;

    for (d = displays; d; d = d->next)
        if (d->status == zombie)
            return;

    switchToTTY();
}

#else

void
switchToX(struct display *d)
{
    struct utmps *utp, **utpp;

    for (utpp = &utmpList; (utp = *utpp); utpp = &(*utpp)->next)
        if (utp->d == d) {
            *utpp = utp->next;
            free(utp);
            d->status = notRunning;
            return;
        }
}
#endif

#ifdef XDMCP
static void
startRemoteLogin(struct display *d)
{
    char **argv;

    debug("startRemoteLogin for %s\n", d->name);
    /* HACK: omitting loadDisplayResources(d) here! */
    switch (Fork(&d->serverPid)) {
    case 0:
        argv = prepareServerArgv(d, d->serverArgsRemote);
        if (!(argv = addStrArr(argv, "-once", 5)) ||
            !(argv = addStrArr(argv, "-query", 6)) ||
            !(argv = addStrArr(argv, d->remoteHost, -1)))
            exit(1);
        debug("exec %\"[s\n", argv);
        (void)execv(argv[0], argv);
        logError("X server %\"s cannot be executed\n", argv[0]);
        exit(1);
    case -1:
        logError("Forking X server for remote login failed: %m");
        d->status = notRunning;
        return;
    default:
        break;
    }
    debug("X server forked, pid %d\n", d->serverPid);

    d->status = remoteLogin;
}
#endif


static void
stopInactiveDisplay(struct display *d)
{
    if (d->status != remoteLogin && d->userSess < 0)
        stopDisplay(d);
}

static void
stoppen(int force)
{
#ifdef XDMCP
    requestPort = 0;
    updateListenSockets();
#endif
    if (force)
        forEachDisplay(stopDisplay);
    else
        forEachDisplay(stopInactiveDisplay);
    stopping = True;
}


static void
sessionDone(struct display *d)
{
    d->userSess = -1;
    free(d->userName);
    free(d->sessName);
    d->userName = d->sessName = 0;
}

void
setNLogin(struct display *d,
          const char *nuser, const char *npass, const char *nargs, int rl)
{
    struct disphist *he = d->hstent;
    he->rLogin =
        (reStr(&he->nuser, nuser) &&
         reStr(&he->npass, npass) &&
         reStr(&he->nargs, nargs)) ? rl : 0;
    debug("set next login for %s, level %d\n", nuser, rl);
}

static void
processDPipe(struct display *d)
{
    char *user, *pass, *args;
    int cmd;
    GTalk dpytalk;
#ifdef XDMCP
    int ct, len;
    ARRAY8 ca, cp, ha;
#endif

    dpytalk.pipe = &d->pipe;
    if (Setjmp(dpytalk.errjmp)) {
        stopDisplay(d);
        return;
    }
    gSet(&dpytalk);
    if (!gRecvCmd(&cmd)) {
        /* process already exited */
        unregisterInput(d->pipe.fd.r);
        return;
    }
    switch (cmd) {
    case D_User:
        d->userSess = gRecvInt();
        d->userName = gRecvStr();
        d->sessName = gRecvStr();
        break;
    case D_UnUser:
        sessionDone(d);
        if (d->sdRec.how) {
            if (d->sdRec.force == SHUT_ASK &&
                (anyUserLogins(-1) || d->allowShutdown == SHUT_ROOT))
            {
                gSendInt(True);
            } else {
                if (!sdRec.how || sdRec.force != SHUT_FORCE ||
                    !((d->allowNuke == SHUT_NONE && sdRec.uid != d->sdRec.uid) ||
                      (d->allowNuke == SHUT_ROOT && d->sdRec.uid)))
                {
                    free(sdRec.osname);
                    sdRec = d->sdRec;
                } else {
                    free(d->sdRec.osname);
                }
                d->sdRec.how = 0;
                d->sdRec.osname = 0;
                gSendInt(False);
            }
        } else {
            gSendInt(False);
        }
        break;
    case D_ReLogin:
        user = gRecvStr();
        pass = gRecvStr();
        args = gRecvStr();
        setNLogin(d, user, pass, args, 1);
        free(args);
        free(pass);
        free(user);
        break;
#ifdef XDMCP
    case D_ChooseHost:
        ca.data = (unsigned char *)gRecvArr(&len);
        ca.length = (CARD16)len;
        cp.data = (unsigned char *)gRecvArr(&len);
        cp.length = (CARD16)len;
        ct = gRecvInt();
        ha.data = (unsigned char *)gRecvArr(&len);
        ha.length = (CARD16)len;
        registerIndirectChoice(&ca, &cp, ct, &ha);
        XdmcpDisposeARRAY8(&ha);
        XdmcpDisposeARRAY8(&cp);
        XdmcpDisposeARRAY8(&ca);
        break;
    case D_RemoteHost:
        free(d->remoteHost);
        d->remoteHost = gRecvStr();
        break;
#endif
    case D_XConnOk:
        startingServer = 0;
        break;
    default:
        logError("Internal error: unknown D_* command %d\n", cmd);
        stopDisplay(d);
        break;
    }
}

static void
emitXSessG(struct display *di, struct display *d, void *ctx ATTR_UNUSED)
{
    gSendStr(di->name);
    gSendStr("");
#ifdef HAVE_VTS
    gSendInt(di->serverVT);
#endif
#ifdef XDMCP
    if (di->status == remoteLogin) {
        gSendStr("");
        gSendStr(di->remoteHost);
    } else
#endif
    {
        gSendStr(di->userName);
        gSendStr(di->sessName);
    }
    gSendInt(di == d ? isSelf : 0);
}

static void
emitTTYSessG(STRUCTUTMP *ut, struct display *d ATTR_UNUSED, void *ctx ATTR_UNUSED)
{
    gSendStrN(ut->ut_line, sizeof(ut->ut_line));
    gSendStrN(ut->ut_host, sizeof(ut->ut_host));
#ifdef HAVE_VTS
    gSendInt(TTYtoVT(ut->ut_line));
#endif
#ifdef BSD_UTMP
    gSendStrN(*ut->ut_user ? ut->ut_user : 0, sizeof(ut->ut_user));
#else
    gSendStrN(ut->ut_type == USER_PROCESS ? ut->ut_user : 0, sizeof(ut->ut_user));
#endif
    gSendStr(0); /* session type unknown */
    gSendInt(isTTY);
}

static void
processGPipe(struct display *d)
{
    char **opts, *option;
    int cmd, ret, dflt, curr;
    GTalk dpytalk;

    dpytalk.pipe = &d->gpipe;
    if (Setjmp(dpytalk.errjmp)) {
        stopDisplay(d);
        return;
    }
    gSet(&dpytalk);
    if (!gRecvCmd(&cmd)) {
        /* process already exited */
        unregisterInput(d->gpipe.fd.r);
        return;
    }
    switch (cmd) {
    case G_ListBootOpts:
        ret = getBootOptions(&opts, &dflt, &curr);
        gSendInt(ret);
        if (ret == BO_OK) {
            gSendArgv(opts);
            freeStrArr(opts);
            gSendInt(dflt);
            gSendInt(curr);
        }
        break;
    case G_Shutdown:
        sdRec.how = gRecvInt();
        sdRec.start = gRecvInt();
        sdRec.timeout = gRecvInt();
        sdRec.force = gRecvInt();
        sdRec.uid = gRecvInt();
        option = gRecvStr();
        setBootOption(option, &sdRec);
        free(option);
        break;
    case G_QueryShutdown:
        gSendInt(sdRec.how);
        gSendInt(sdRec.start);
        gSendInt(sdRec.timeout);
        gSendInt(sdRec.force);
        gSendInt(sdRec.uid);
        gSendStr(sdRec.osname);
        break;
    case G_QryDpyShutdown:
        gSendInt(d->sdRec.how);
        gSendInt(d->sdRec.uid);
        gSendStr(d->sdRec.osname);
        break;
    case G_List:
        listSessions(gRecvInt(), d, 0, emitXSessG, emitTTYSessG);
        gSendInt(0);
        break;
#ifdef HAVE_VTS
    case G_Activate:
        activateVT(gRecvInt());
        break;
#endif
    case G_Console:
#ifdef HAVE_VTS
        if (*consoleTTYs) { /* sanity check against greeter */
            forEachDisplay(stopToTTY);
            checkTTYMode();
        }
#else
        if (*d->console) /* sanity check against greeter */
            rStopDisplay(d, DS_TEXTMODE);
#endif
        break;
    default:
        logError("Internal error: unknown G_* command %d\n", cmd);
        stopDisplay(d);
        break;
    }
}


static int
scanConfigs(int force)
{
    int ret;

    if ((ret = loadDMResources(force)) <= 0)
        return ret;
    scanServers();
#ifdef XDMCP
    scanAccessDatabase(force);
#endif
    return 1;
}

static void
markDisplay(struct display *d)
{
    d->stillThere = False;
}

static void
rescanConfigs(int force)
{
    if (scanConfigs(force) > 0) {
#ifdef XDMCP
        updateListenSockets();
#endif
        updateCtrl();
    }
}

void
cancelShutdown(void)
{
    sdRec.how = 0;
    free(sdRec.osname);
    sdRec.osname = 0;
    stopping = False;
    rescanConfigs(True);
}


static void
reapChildren(void)
{
    int pid;
    struct display *d;
    int status;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        debug("manager wait returns  pid %d  sig %d  core %d  code %d\n",
              pid, waitSig(status), waitCore(status), waitCode(status));
        /* SUPPRESS 560 */
        if ((d = findDisplayByPid(pid))) {
            d->pid = -1;
            unregisterInput(d->pipe.fd.r);
            gClosen(&d->pipe);
            unregisterInput(d->gpipe.fd.r);
            gClosen(&d->gpipe);
            closeCtrl(d);
            switch (wcFromWait(status)) {
#ifdef XDMCP
            case EX_REMOTE:
                debug("display exited with EX_REMOTE\n");
                exitDisplay(d, DS_REMOTE, 0, False);
                break;
#endif
            case EX_NORMAL:
                /* (any type of) session ended */
                debug("display exited with EX_NORMAL\n");
                if ((d->displayType & d_lifetime) == dReserve)
                    exitDisplay(d, DS_RESERVE, 0, False);
                else
                    exitDisplay(d, DS_RESTART, XS_KEEP, True);
                break;
            case EX_RESERVE:
                debug("display exited with EX_RESERVE\n");
                exitDisplay(d, DS_RESERVE, 0, False);
                break;
#if 0
            case EX_REMANAGE_DPY:
                /* user session ended */
                debug("display exited with EX_REMANAGE_DPY\n");
                exitDisplay(d, DS_RESTART, XS_KEEP, True);
                break;
#endif
            case EX_OPENFAILED_DPY:
                /* waitForServer() failed */
                logError("Display %s cannot be opened\n", d->name);
#ifdef XDMCP
                /*
                 * no display connection was ever made, tell the
                 * terminal that the open attempt failed
                 */
                if ((d->displayType & d_origin) == dFromXDMCP)
                    sendFailed(d, "cannot open display");
#endif
                exitDisplay(d, DS_RESTART, XS_RETRY, False);
                break;
            case wcCompose(SIGTERM, 0, 0):
                /* killed before/during waitForServer()
                   - local Xserver died
                   - display stopped (is zombie)
                   - "login now" and "suicide" pipe commands (is raiser)
                */
                debug("display exited on SIGTERM\n");
                exitDisplay(d, DS_RESTART, XS_RETRY, False);
                break;
            case EX_AL_RESERVER_DPY:
                /* - killed after waitForServer()
                   - Xserver dead after remote session exit
                */
                debug("display exited with EX_AL_RESERVER_DPY\n");
                exitDisplay(d, DS_RESTART, XS_RESTART, False);
                break;
            case EX_RESERVER_DPY:
                /* induced by greeter:
                   - could not secure display
                   - requested by user
                */
                debug("display exited with EX_RESERVER_DPY\n");
                exitDisplay(d, DS_RESTART, XS_RESTART, True);
                break;
            case EX_UNMANAGE_DPY:
                /* some fatal error */
                debug("display exited with EX_UNMANAGE_DPY\n");
                exitDisplay(d, DS_REMOVE, 0, False);
                break;
            default:
                /* prolly crash */
                logError("Unknown session exit code %d (sig %d) from manager process\n",
                         waitCode(status), waitSig(status));
                exitDisplay(d, DS_REMOVE, 0, False);
                break;
            }
        } else if ((d = findDisplayByServerPid(pid))) {
            d->serverPid = -1;
            switch (d->status) {
            case zombie:
                debug("zombie X server for display %s reaped\n", d->name);
#ifdef HAVE_VTS
                if (d->serverVT && d->zstatus != DS_REMOTE) {
                    if (d->follower) {
                        d->follower->serverVT = d->serverVT;
                        d->follower = 0;
                    } else {
                        int con = open("/dev/console", O_RDONLY);
                        if (con >= 0) {
                            struct vt_stat vtstat;
                            ioctl(con, VT_GETSTATE, &vtstat);
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
                                ioctl(con, VT_ACTIVATE, vt);
                            }
                            ioctl(con, VT_DISALLOCATE, d->serverVT);
                            close(con);
                        }
                    }
                    d->serverVT = 0;
                }
#endif
                rStopDisplay(d, d->zstatus);
                break;
            case phoenix:
                debug("phoenix X server arises, restarting display %s\n",
                      d->name);
                d->status = notRunning;
                break;
            case remoteLogin:
                debug("remote login X server for display %s exited\n",
                      d->name);
                d->status = ((d->displayType & d_lifetime) == dReserve) ?
                            reserve : notRunning;
                break;
            case raiser:
                logError("X server for display %s terminated unexpectedly\n",
                         d->name);
                /* don't kill again */
                break;
            case running:
                if (startingServer == d && d->serverStatus != ignore) {
                    if (d->serverStatus == starting && waitCode(status) != 47)
                        logError("X server died during startup\n");
                    startServerFailed();
                    break;
                }
                logError("X server for display %s terminated unexpectedly\n",
                         d->name);
                if (d->pid != -1) {
                    debug("terminating session pid %d\n", d->pid);
                    terminateProcess(d->pid, SIGTERM);
                }
                break;
            case notRunning:
            case textMode:
            case reserve:
                /* this cannot happen */
                debug("X server exited for passive (%d) session on display %s\n",
                      (int)d->status, d->name);
                break;
            }
        } else {
            debug("unknown child termination\n");
        }
    }
#ifdef NEED_ENTROPY
    addOtherEntropy();
#endif
}

static int
wouldShutdown(void)
{
    switch (sdRec.force) {
    case SHUT_FORCE:
        return True;
    case SHUT_FORCEMY:
        return !anyUserLogins(sdRec.uid);
    case SHUT_CANCEL:
    default:
        return !anyUserLogins(-1);
    }
}

fd_set wellKnownSocketsMask;
int wellKnownSocketsMax;
int wellKnownSocketsCount;

void
registerInput(int fd)
{
    /* can be omited, as it is always called right after opening a socket
    if (!FD_ISSET(fd, &wellKnownSocketsMask))
    */
    {
        FD_SET(fd, &wellKnownSocketsMask);
        if (fd > wellKnownSocketsMax)
            wellKnownSocketsMax = fd;
        wellKnownSocketsCount++;
    }
}

void
unregisterInput(int fd)
{
    /* the check _is_ necessary, as some handles are unregistered before
       the regular close sequence.
    */
    if (FD_ISSET(fd, &wellKnownSocketsMask)) {
        FD_CLR(fd, &wellKnownSocketsMask);
        wellKnownSocketsCount--;
    }
}

static void
sigHandler(int n)
{
    int olderrno = errno;
    char buf = (char)n;
    /* debug("caught signal %d\n", n); this hangs in syslog() */
    write(signalFds[1], &buf, 1);
#ifdef __EMX__
    (void)Signal(n, sigHandler);
#endif
    errno = olderrno;
}

static void
mainLoop(void)
{
    struct display *d;
    struct timeval *tvp, tv;
    time_t to;
    int nready;
    char buf;
    fd_set reads;

    debug("mainLoop\n");
    updateNow();
    while (
#ifdef XDMCP
           anyListenSockets() ||
#endif
           (stopping ? anyRunningDisplays() : anyDisplaysLeft()))
    {
        if (!stopping)
            startDisplays();
#ifdef XDMCP
        to = disposeIndirectHosts();
#else
        to = TO_INF;
#endif
        if (sdRec.how) {
            if (sdRec.start != TO_INF && now < sdRec.start) {
                /*if (sdRec.start < to)*/
                    to = sdRec.start;
            } else {
                sdRec.start = TO_INF;
                if (now >= sdRec.timeout) {
                    sdRec.timeout = TO_INF;
                    if (wouldShutdown())
                        stoppen(True);
                    else
                        cancelShutdown();
                } else {
                    stoppen(False);
                    /*if (sdRec.timeout < to)*/
                        to = sdRec.timeout;
                }
            }
        }
        if (serverTimeout < to)
            to = serverTimeout;
        if (utmpTimeout < to)
            to = utmpTimeout;
        if (to == TO_INF) {
            tvp = 0;
        } else {
            to -= now;
            if (to < 0)
                to = 0;
            tv.tv_sec = to;
            tv.tv_usec = 0;
            tvp = &tv;
        }
        reads = wellKnownSocketsMask;
        nready = select(wellKnownSocketsMax + 1, &reads, 0, 0, tvp);
        debug("select returns %d\n", nready);
        updateNow();
#ifdef NEED_ENTROPY
        addTimerEntropy();
#endif
        if (now >= serverTimeout) {
            serverTimeout = TO_INF;
            startServerTimeout();
        }
        if (now >= utmpTimeout) {
            utmpTimeout = TO_INF;
            checkUtmp();
        }
        if (nready > 0) {
            /*
             * we restart after the first handled fd, as
             * a) it makes things simpler
             * b) the probability that multiple fds trigger at once is
             *    ridiculously small. we handle it in the next iteration.
             */
            /* XXX a cleaner solution would be a callback mechanism */
            if (FD_ISSET(signalFds[0], &reads)) {
                if (read(signalFds[0], &buf, 1) != 1)
                    logPanic("Signal notification pipe broken.\n");
                switch (buf) {
                case SIGTERM:
                case SIGINT:
                    debug("shutting down entire manager\n");
                    stoppen(True);
                    break;
                case SIGHUP:
                    logInfo("Rescanning all config files\n");
                    forEachDisplay(markDisplay);
                    rescanConfigs(True);
                    break;
                case SIGCHLD:
                    reapChildren();
                    if (!stopping && autoRescan)
                        rescanConfigs(False);
                    break;
                case SIGUSR1:
                    if (startingServer && startingServer->serverStatus == starting)
                        startServerSuccess();
                    break;
                }
                continue;
            }
#ifdef XDMCP
            if (processListenSockets(&reads))
                continue;
#endif
            if (handleCtrl(&reads, 0))
                continue;
            /* Must be last (because of the breaks)! */
          again:
            for (d = displays; d; d = d->next) {
                if (handleCtrl(&reads, d))
                    goto again;
                if (d->pipe.fd.r >= 0 && FD_ISSET(d->pipe.fd.r, &reads)) {
                    processDPipe(d);
                    break;
                }
                if (d->gpipe.fd.r >= 0 && FD_ISSET(d->gpipe.fd.r, &reads)) {
                    processGPipe(d);
                    break;
                }
            }
        }
    }
}

static void
checkDisplayStatus(struct display *d)
{
    if ((d->displayType & d_origin) == dFromFile && !d->stillThere)
        stopDisplay(d);
    else if (d->status == notRunning)
        if (loadDisplayResources(d) < 0) {
            logError("Unable to read configuration for display %s; "
                     "stopping it.\n", d->name);
            stopDisplay(d);
            return;
        }
}

static void
kickDisplay(struct display *d)
{
    if (d->status == notRunning)
        startDisplay(d);
    if (d->serverStatus == awaiting && !startingServer)
        startServer(d);
}

#ifdef HAVE_VTS
static int activeVTs;

static int
getBusyVTs(void)
{
    struct vt_stat vtstat;
    int con;

    if (activeVTs == -1) {
        vtstat.v_state = 0;
        if ((con = open("/dev/console", O_RDONLY)) >= 0) {
            ioctl(con, VT_GETSTATE, &vtstat);
            close(con);
        }
        activeVTs = vtstat.v_state;
    }
    return activeVTs;
}

static void
allocateVT(struct display *d)
{
    struct display *cd;
    int i, tvt, volun;

    if ((d->displayType & d_location) == dLocal &&
        d->status == notRunning && !d->serverVT && d->reqSrvVT >= 0)
    {
        if (d->reqSrvVT && d->reqSrvVT < 16) {
            d->serverVT = d->reqSrvVT;
        } else {
            for (i = tvt = 0;;) {
                if (serverVTs[i]) {
                    tvt = atoi(serverVTs[i++]);
                    volun = False;
                    if (tvt < 0) {
                        tvt = -tvt;
                        volun = True;
                    }
                    if (!tvt || tvt >= 16)
                        continue;
                } else {
                    if (++tvt >= 16)
                        break;
                    volun = True;
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
                if (!volun || !((1 << tvt) & getBusyVTs())) {
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
startDisplays(void)
{
    forEachDisplay(checkDisplayStatus);
    closeGetter();
#ifdef HAVE_VTS
    activeVTs = -1;
    forEachDisplayRev(allocateVT);
#endif
    forEachDisplay(kickDisplay);
}

static void
startDisplay(struct display *d)
{
    if (stopping) {
        debug("stopping display %s because shutdown is scheduled\n", d->name);
        stopDisplay(d);
        return;
    }

#ifdef HAVE_VTS
    if (d->serverVT < 0)
        return;
#endif

    d->status = running;
    if ((d->displayType & d_location) == dLocal) {
        debug("startDisplay %s\n", d->name);
        /* don't bother pinging local displays; we'll
         * certainly notice when they exit
         */
        d->pingInterval = 0;
        if (d->authorize) {
            setLocalAuthorization(d);
            /*
             * reset the server after writing the authorization information
             * to make it read the file (for compatibility with old
             * servers which read auth file only on reset instead of
             * at first connection)
             */
            if (d->serverPid != -1 && d->resetForAuth)
                kill(d->serverPid, SIGHUP);
        }
        if (d->serverPid == -1) {
            d->serverStatus = awaiting;
            return;
        }
    } else {
        debug("startDisplay %s, try %d\n", d->name, d->startTries + 1);
        /* this will only happen when using XDMCP */
        if (d->authorizations)
            saveServerAuthorizations(d, d->authorizations, d->authNum);
    }
    startDisplayP2(d);
}

void
startDisplayP2(struct display *d)
{
    char *cname, *cgname;

    openCtrl(d);
    debug("forking session\n");
    ASPrintf(&cname, "sub-daemon for display %s", d->name);
    ASPrintf(&cgname, "greeter for display %s", d->name);
    switch (gFork(&d->pipe, "master daemon", cname,
                  &d->gpipe, cgname, 0, &d->pid)) {
    case 0:
        td = d;
#ifndef NOXDMTITLE
        setproctitle("%s", d->name);
#endif
        ASPrintf(&prog, "%s: %s", prog, d->name);
        reInitErrorLog();
        if (debugLevel & DEBUG_WSESS)
            sleep(100);
        mstrtalk.pipe = &d->pipe;
        (void)Signal(SIGPIPE, SIG_IGN);
        setAuthorization(d);
        waitForServer(d);
        if ((d->displayType & d_location) == dLocal) {
            gSet(&mstrtalk);
            if (Setjmp(mstrtalk.errjmp))
                exit(EX_UNMANAGE_DPY);
            gSendInt(D_XConnOk);
        }
        manageSession();
        /* NOTREACHED */
    case -1:
        closeCtrl(d);
        d->status = notRunning;
        break;
    default:
        debug("forked session, pid %d\n", d->pid);

        /* (void) fcntl (d->pipe.fd.r, F_SETFL, O_NONBLOCK); */
        /* (void) fcntl (d->gpipe.fd.r, F_SETFL, O_NONBLOCK); */
        registerInput(d->pipe.fd.r);
        registerInput(d->gpipe.fd.r);

        d->hstent->lock = d->hstent->rLogin = d->hstent->goodExit =
            d->sdRec.how = 0;
        d->lastStart = now;
        break;
    }
}

/*
 * transition from running to zombie, textmode, reserve or deleted
 */

static void
rStopDisplay(struct display *d, int endState)
{
    debug("stopping display %s to state %d\n", d->name, endState);
    abortStartServer(d);
    if (d->serverPid != -1 || d->pid != -1) {
        if (d->pid != -1)
            terminateProcess(d->pid, SIGTERM);
        if (d->serverPid != -1)
            terminateProcess(d->serverPid, SIGTERM);
        d->status = zombie;
        d->zstatus = endState & DS_MASK;
        debug(" zombiefied\n");
    } else if (endState == DS_TEXTMODE) {
#ifdef HAVE_VTS
        d->status = textMode;
        checkTTYMode();
    } else if (endState == (DS_TEXTMODE | DS_SCHEDULE)) {
        d->status = textMode;
#else
        switchToTTY(d);
#endif
    } else if (endState == DS_RESERVE) {
        d->status = reserve;
        d->hstent->lastExit = 0;
#ifdef XDMCP
    } else if (endState == DS_REMOTE) {
        startRemoteLogin(d);
#endif
    } else {
#ifndef HAVE_VTS
        switchToX(d);
#endif
        removeDisplay(d);
    }
}

void
stopDisplay(struct display *d)
{
    rStopDisplay(d, DS_REMOVE);
}

static void
exitDisplay(struct display *d, int endState, int serverCmd, int goodExit)
{
    struct disphist *he;

    if (d->status == raiser) {
        serverCmd = XS_KEEP;
        goodExit = True;
    }

    debug("exitDisplay %s, "
          "endState = %d, serverCmd = %d, GoodExit = %d\n",
          d->name, endState, serverCmd, goodExit);

    sessionDone(d);
    he = d->hstent;
    he->lastExit = now;
    he->goodExit = goodExit;
    if (sdRec.how && sdRec.start == TO_INF)
        endState = DS_REMOVE;
    if (d->status == zombie) {
        rStopDisplay(d, d->zstatus);
    } else {
        if (stopping) {
            stopDisplay(d);
            return;
        }
        if (endState != DS_RESTART ||
            (d->displayType & d_lifetime) == dTransient)
        {
            rStopDisplay(d, endState);
        } else {
            if (serverCmd == XS_RETRY) {
                if ((d->displayType & d_location) == dLocal) {
                    if (he->lastExit - d->lastStart < 120) {
                        logError("Unable to fire up local display %s;"
                                 " disabling.\n", d->name);
                        stopDisplay(d);
                        return;
                    }
                } else {
                    if (++d->startTries > d->startAttempts) {
                        logError("Disabling foreign display %s"
                                 " (too many attempts)\n", d->name);
                        stopDisplay(d);
                        return;
                    }
                }
            } else {
                d->startTries = 0;
            }
            if (d->serverPid != -1 &&
                (serverCmd != XS_KEEP || d->terminateServer))
            {
                debug("killing X server for %s\n", d->name);
                terminateProcess(d->serverPid, SIGTERM);
                d->status = phoenix;
            } else {
                d->status = notRunning;
            }
        }
    }
}


static int pidFd;
static FILE *pidFilePtr;

static int
storePid(void)
{
    int oldpid;

    if (pidFile[0] != '\0') {
        pidFd = open(pidFile, O_RDWR);
        if (pidFd == -1 && errno == ENOENT)
            pidFd = open(pidFile, O_RDWR | O_CREAT, 0666);
        if (pidFd == -1 || !(pidFilePtr = fdopen(pidFd, "r+"))) {
            logError("process-id file %s cannot be opened\n",
                     pidFile);
            return -1;
        }
        if (fscanf(pidFilePtr, "%d\n", &oldpid) != 1)
            oldpid = -1;
        fseek(pidFilePtr, 0l, 0);
        if (lockPidFile) {
#ifdef F_SETLK
# ifndef SEEK_SET
#  define SEEK_SET 0
# endif
            struct flock lock_data;
            lock_data.l_type = F_WRLCK;
            lock_data.l_whence = SEEK_SET;
            lock_data.l_start = lock_data.l_len = 0;
            if (fcntl(pidFd, F_SETLK, &lock_data) == -1) {
                if (errno == EAGAIN)
                    return oldpid;
                else
                    return -1;
            }
#else
# ifdef LOCK_EX
            if (flock(pidFd, LOCK_EX | LOCK_NB) == -1) {
                if (errno == EWOULDBLOCK)
                    return oldpid;
                else
                    return -1;
            }
# else
            if (lockf(pidFd, F_TLOCK, 0) == -1) {
                if (errno == EACCES)
                    return oldpid;
                else
                    return -1;
            }
# endif
#endif
        }
        fprintf(pidFilePtr, "%ld\n", (long)getpid());
        if (fflush(pidFilePtr) == EOF)
            logError("Cannot write PID file %s: %m", pidFile);
        registerCloseOnFork(pidFd);
    }
    return 0;
}

#if 0
void
UnlockPidFile(void)
{
    if (lockPidFile)
# ifdef F_SETLK
    {
        struct flock lock_data;
        lock_data.l_type = F_UNLCK;
        lock_data.l_whence = SEEK_SET;
        lock_data.l_start = lock_data.l_len = 0;
        (void)fcntl(pidFd, F_SETLK, &lock_data);
    }
# else
#  ifdef F_ULOCK
        lockf(pidFd, F_ULOCK, 0);
#  else
        flock(pidFd, LOCK_UN);
#  endif
# endif
    close(pidFd);
    fclose(pidFilePtr);
}
#endif

#if !defined(HAVE_SETPROCTITLE) && !defined(NOXDMTITLE)
void
setproctitle(const char *fmt, ...)
{
    const char *name;
    char *oname;
    char *p = title;
    int left = titleLen;
    va_list args;

    va_start(args, fmt);
    VASPrintf(&oname, fmt, args);
    va_end(args);

    if ((name = oname)) {
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

        free(oname);
    }
}
#endif
