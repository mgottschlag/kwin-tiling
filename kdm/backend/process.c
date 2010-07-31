/*

Copyright 1988, 1998  The Open Group
Copyright 2001-2004 Oswald Buddenhagen <ossi@kde.org>

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
 * subdaemon and external process management and communication
 */

#include "dm.h"
#include "dm_error.h"
#if defined(SINGLE_PIPE) && !defined(__FreeBSD__)
# include "dm_socket.h"
#endif

#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>
#include <signal.h>
#include <unistd.h>
#ifdef _POSIX_PRIORITY_SCHEDULING
# include <sched.h>
#endif


SIGFUNC Signal(int sig, SIGFUNC handler)
{
#ifndef __EMX__
    struct sigaction sigact, osigact;
    sigact.sa_handler = handler;
    sigemptyset(&sigact.sa_mask);
# ifdef SA_RESTART
    sigact.sa_flags = SA_RESTART;
# else
    sigact.sa_flags = 0;
# endif
    sigaction(sig, &sigact, &osigact);
    return osigact.sa_handler;
#else
    return signal(sig, handler);
#endif
}

void
blockTerm(void)
{
    sigset_t ss;

    sigemptyset(&ss);
    sigaddset(&ss, SIGTERM);
    sigprocmask(SIG_BLOCK, &ss, 0);
}

void
unblockTerm(void)
{
    sigset_t ss;

    sigemptyset(&ss);
    sigaddset(&ss, SIGTERM);
    sigprocmask(SIG_UNBLOCK, &ss, 0);
}

void
terminateProcess(int pid, int sig)
{
    kill(pid, sig);
#ifdef SIGCONT
    kill(pid, SIGCONT);
#endif
}


static fd_set closeMask;
static int max = -1;

void
registerCloseOnFork(int fd)
{
    FD_SET(fd, &closeMask);
    if (fd > max)
        max = fd;
}

void
clearCloseOnFork(int fd)
{
    FD_CLR(fd, &closeMask);
}

void
closeNclearCloseOnFork(int fd)
{
    close(fd);
    FD_CLR(fd, &closeMask);
}

static void
closeOnFork(void)
{
    int fd;

    for (fd = 0; fd <= max; fd++)
        if (FD_ISSET(fd, &closeMask))
            close(fd);
    FD_ZERO(&closeMask);
    max = -1;
}

int
Fork(volatile int *pidr)
{
    int pid;

    sigset_t ss, oss;
    sigfillset(&ss);
    sigprocmask(SIG_SETMASK, &ss, &oss);

    if (!(pid = fork())) {
#ifdef SIGCHLD
        (void)Signal(SIGCHLD, SIG_DFL);
#endif
        (void)Signal(SIGTERM, SIG_DFL);
        (void)Signal(SIGINT, SIG_IGN); /* for -nodaemon */
        (void)Signal(SIGPIPE, SIG_DFL);
        (void)Signal(SIGALRM, SIG_DFL);
        (void)Signal(SIGHUP, SIG_DFL);
        sigemptyset(&ss);
        sigprocmask(SIG_SETMASK, &ss, 0);
        closeOnFork();
        return 0;
    }
    *pidr = pid;

    sigprocmask(SIG_SETMASK, &oss, 0);

    return pid;
}

static void
cldCatcher(int n ATTR_UNUSED)
{
}

int
Wait4(volatile int *pid)
{
    int result;
    sigset_t ss, oss;
    struct sigaction osa;

    sigfillset(&ss);
    sigprocmask(SIG_SETMASK, &ss, &oss);
    sigaction(SIGCHLD, 0, &osa);
    if (osa.sa_handler == SIG_DFL)
        Signal(SIGCHLD, cldCatcher);
    for (;;) {
        switch (waitpid(*pid, &result, WNOHANG)) {
        case 0:
            sigsuspend(&oss);
            break;
        case -1:
            debug("Wait4(%d) failed: %m\n", *pid);
            result = 0;
            /* fallthrough */
        default:
            *pid = 0;
            if (osa.sa_handler == SIG_DFL)
                Signal(SIGCHLD, SIG_DFL);
            sigprocmask(SIG_SETMASK, &oss, 0);
            return wcFromWait(result);
        }
    }
}


void
execute(char **argv, char **env)
{
    debug("execute: %[s ; %[s\n", argv, env);
    execve(argv[0], argv, env);
    /*
     * In case this is a shell script which hasn't been
     * made executable (or this is a SYSV box), do
     * a reasonable thing
     */
    if (errno != ENOENT) {
        char **newargv;
        FILE *f;
        int nu;
        char program[1024];

        /*
         * emulate BSD kernel behaviour -- read
         * the first line; check if it starts
         * with "#!", in which case it uses
         * the rest of the line as the name of
         * program to run. Else use "/bin/sh".
         */
        if (!(f = fopen(argv[0], "r")))
            return;
        if (!fGets(program, sizeof(program), f)) {
            fclose(f);
            return;
        }
        fclose(f);
        if (!strncmp(program, "#!", 2))
            newargv = parseArgs(0, program + 2);
        else
            newargv = addStrArr(0, "/bin/sh", 7);
        if (!newargv)
            return;
        nu = arrLen(newargv);
        if (!(argv = xCopyStrArr(nu, argv)))
            return;
        memcpy(argv, newargv, sizeof(char *) * nu);
        debug("shell script execution: %[s\n", argv);
        execve(argv[0], argv, env);
    }
}

int
runAndWait(char **args, char **env)
{
    int pid, ret;

    switch (Fork(&pid)) {
    case 0:
        execute(args, env);
        logError("Cannot execute %\"s: %m\n", args[0]);
        exit(127);
    case -1:
        logError("Cannot fork to execute %\"s: %m\n", args[0]);
        return wcCompose(0, 0, 127);
    }
    ret = Wait4(&pid);
    return wcFromWait(ret);
}

FILE *
pOpen(char **what, char m, volatile int *pid)
{
    int dp[2];

    if (pipe(dp))
        return 0;
    switch (Fork(pid)) {
    case 0:
        if (m == 'r')
            dup2(dp[1], 1);
        else
            dup2(dp[0], 0);
        close(dp[0]);
        close(dp[1]);
        execute(what, environ);
        logError("Cannot execute %\"s: %m\n", what[0]);
        exit(127);
    case -1:
        close(dp[0]);
        close(dp[1]);
        logError("Cannot fork to execute %\"s: %m\n", what[0]);
        return 0;
    }
    if (m == 'r') {
        close(dp[1]);
        return fdopen(dp[0], "r");
    } else {
        close(dp[0]);
        return fdopen(dp[1], "w");
    }
}

int
pClose(FILE *f, volatile int *pid)
{
    fclose(f);
    return Wait4(pid);
}

char *
locate(const char *exe)
{
    int len;
    char *path, *pathe, *name, *thenam, nambuf[PATH_MAX+1];

    if (!(path = getenv("PATH"))) {
        logError("Cannot execute %'s: $PATH not set.\n", exe);
        return 0;
    }
    len = strlen(exe);
    name = nambuf + PATH_MAX - len;
    memcpy(name, exe, len + 1);
    *--name = '/';
    do {
        if (!(pathe = strchr(path, ':')))
            pathe = path + strlen(path);
        len = pathe - path;
        if (len && !(len == 1 && *path == '.')) {
            thenam = name - len;
            if (thenam >= nambuf) {
                memcpy(thenam, path, len);
                if (!access(thenam, X_OK)) {
                    strDup(&name, thenam);
                    return name;
                }
            }
        }
        path = pathe;
    } while (*path++ != '\0');
    logError("Cannot execute %'s: not in $PATH.\n", exe);
    return 0;
}


static GTalk *curtalk;

void
gSet(GTalk *tlk)
{
    curtalk = tlk;
}

void
gCloseOnExec(GPipe *pajp)
{
    fcntl(pajp->fd.r, F_SETFD, FD_CLOEXEC);
#ifndef SINGLE_PIPE
    fcntl(pajp->fd.w, F_SETFD, FD_CLOEXEC);
#endif
}

#if !defined(SINGLE_PIPE) || defined(__FreeBSD__)
# define make_pipe(h) pipe(h)
#else
# define make_pipe(h) socketpair(AF_UNIX, SOCK_STREAM, 0, h)
#endif

int
gFork(GPipe *pajp, const char *pname, char *cname,
      GPipe *ogp, char *cgname,
      GPipe *gp,
      volatile int *pidr)
{
    int opipe[2], ogpipe[2];
#ifndef SINGLE_PIPE
    int ipipe[2], igpipe[2];
#endif
    int pid;

    if (make_pipe(opipe))
        goto badp1;
#ifndef SINGLE_PIPE
    if (pipe(ipipe))
        goto badp2;
#endif
    if (ogp) {
        if (make_pipe(ogpipe))
#ifdef SINGLE_PIPE
        {
#else
            goto badp3;
        if (pipe(igpipe)) {
            close(ogpipe[0]);
            close(ogpipe[1]);
          badp3:
            close(ipipe[0]);
            close(ipipe[1]);
          badp2:
#endif
            close(opipe[0]);
            close(opipe[1]);
          badp1:
            logError("Cannot start %s, pipe() failed", cname);
            free(cname);
            return -1;
        }
    }
    registerCloseOnFork(opipe[1]);
#ifndef SINGLE_PIPE
    registerCloseOnFork(ipipe[0]);
#endif
    if (ogp) {
        registerCloseOnFork(ogpipe[1]);
#ifndef SINGLE_PIPE
        registerCloseOnFork(igpipe[0]);
#endif
    }
    if (gp) {
        clearCloseOnFork(gp->fd.r);
#ifndef SINGLE_PIPE
        clearCloseOnFork(gp->fd.w);
#endif
    }
    if ((pid = Fork(pidr))) {
        if (gp) {
            registerCloseOnFork(gp->fd.r);
#ifndef SINGLE_PIPE
            registerCloseOnFork(gp->fd.w);
#endif
        }
        if (ogp) {
            close(ogpipe[0]);
#ifndef SINGLE_PIPE
            close(igpipe[1]);
#endif
        }
        close(opipe[0]);
#ifndef SINGLE_PIPE
        close(ipipe[1]);
#endif
    }
    switch (pid) {
    case -1:
        closeNclearCloseOnFork(opipe[1]);
#ifndef SINGLE_PIPE
        closeNclearCloseOnFork(ipipe[0]);
#endif
        if (ogp) {
            closeNclearCloseOnFork(ogpipe[1]);
#ifndef SINGLE_PIPE
            closeNclearCloseOnFork(igpipe[0]);
#endif
        }
        logError("Cannot start %s, fork() failed\n", cname);
        free(cname);
        return -1;
    case 0:
#ifndef SINGLE_PIPE
        pajp->fd.w = ipipe[1];
        registerCloseOnFork(ipipe[1]);
#endif
        pajp->fd.r = opipe[0];
        registerCloseOnFork(opipe[0]);
        pajp->who = (char *)pname;
        if (ogp) {
#ifndef SINGLE_PIPE
            ogp->fd.w = igpipe[1];
            registerCloseOnFork(igpipe[1]);
#endif
            ogp->fd.r = ogpipe[0];
            registerCloseOnFork(ogpipe[0]);
            ogp->who = (char *)pname;
        }
        free(cname);
        return 0;
    default:
        pajp->fd.w = opipe[1];
#ifndef SINGLE_PIPE
        pajp->fd.r = ipipe[0];
#endif
        pajp->who = cname;
        if (ogp) {
            ogp->fd.w = ogpipe[1];
#ifndef SINGLE_PIPE
            ogp->fd.r = igpipe[0];
#endif
            ogp->who = cgname;
        }
        return pid;
    }
}

int
gOpen(GProc *proc, char **argv, const char *what, char **env, char *cname,
      const char *user, const char *authfile, GPipe *gp)
{
    char **margv;
    int pip[2];
    char coninfo[32];

    if (proc->pid > 0) {
        logError("%s already running\n", cname);
        free(cname);
        return -1;
    }
    if (!(margv = xCopyStrArr(1, argv))) {
        free(cname);
        return -1;
    }
#if KDM_LIBEXEC_STRIP == -1
    if (!strApp(margv, KDM_LIBEXEC_SUFFIX, progname, what, (char *)0))
#elif KDM_LIBEXEC_STRIP == 0
    if (!strApp(margv, progpath, what, (char *)0))
#else
    if (!strApp(margv, progpath, "/" KDM_LIBEXEC_SUFFIX, progname, what, (char *)0))
#endif
    {
        free(margv);
        free(cname);
        return -1;
    }
    if (pipe(pip)) {
        logError("Cannot start %s, pipe() failed\n", cname);
        free(cname);
        goto fail;
    }
    switch (gFork(&proc->pipe, 0, cname, 0, 0, gp, &proc->pid)) {
    case -1:
        close(pip[1]);
      fail1:
        close(pip[0]);
      fail:
        free(margv[0]);
        free(margv);
        return -1;
    case 0:
        (void)Signal(SIGPIPE, SIG_IGN);
        close(pip[0]);
        fcntl(pip[1], F_SETFD, FD_CLOEXEC);
        if (changeUser(user, authfile)) {
            if (gp)
                sprintf(coninfo, "CONINFO=%d %d %d %d",
                        proc->pipe.fd.r, proc->pipe.fd.w, gp->fd.r, gp->fd.w);
            else
                sprintf(coninfo, "CONINFO=%d %d",
                        proc->pipe.fd.r, proc->pipe.fd.w);
            env = putEnv(coninfo, env);
            if (debugLevel & DEBUG_VALGRIND) {
                char **nmargv = xCopyStrArr(1, margv);
                nmargv[0] = locate("valgrind");
                execute(nmargv, env);
            } else if (debugLevel & DEBUG_STRACE) {
                char **nmargv = xCopyStrArr(1, margv);
                nmargv[0] = locate("strace");
                execute(nmargv, env);
            } else {
                execute(margv, env);
            }
        }
        write(pip[1], "", 1);
        exit(1);
    default:
        (void)Signal(SIGPIPE, SIG_IGN);
        close(pip[1]);
        if (reader(pip[0], coninfo, 1)) {
            Wait4(&proc->pid);
            logError("Cannot execute %\"s (%s)\n", margv[0], cname);
            gClosen(&proc->pipe);
            goto fail1;
        }
        close(pip[0]);
        debug("started %s (%\"s), pid %d\n", cname, margv[0], proc->pid);
        free(margv[0]);
        free(margv);
        gSendInt(debugLevel);
        return 0;
    }
}

static void
_gClosen(GPipe *pajp)
{
    if (pajp->fd.r == -1)
        return;
    closeNclearCloseOnFork(pajp->fd.r);
#ifndef SINGLE_PIPE
    closeNclearCloseOnFork(pajp->fd.w);
    pajp->fd.w =
#endif
    pajp->fd.r = -1;
}

void
gClosen(GPipe *pajp)
{
    _gClosen(pajp);
    free(pajp->who);
    pajp->who = 0;
}

int
gClose(GProc *proc, GPipe *gp, int force)
{
    int ret;

    if (proc->pid <= 0) {
        debug("whoops, gClose while helper not running\n");
        return 0;
    }
    _gClosen(&proc->pipe);
    if (gp)
        gClosen(gp);
    if (force)
        terminateProcess(proc->pid, SIGTERM);
    ret = Wait4(&proc->pid);
    if (wcSig(ret) ? wcSig(ret) != SIGTERM :
                     (wcCode(ret) < EX_NORMAL || wcCode(ret) > EX_MAX))
        logError("Abnormal termination of %s, code %d, signal %d\n",
                 proc->pipe.who, wcCode(ret), wcSig(ret));
    debug("closed %s\n", proc->pipe.who);
    free(proc->pipe.who);
    proc->pipe.who = 0;
    return ret;
}

static void ATTR_NORETURN
gErr(void)
{
    gClosen(curtalk->pipe);
    Longjmp(curtalk->errjmp, 1);
}

static void
gRead(void *buf, int len)
{
    if (reader(curtalk->pipe->fd.r, buf, len) != len) {
        logError("Cannot read from %s\n", curtalk->pipe->who);
        gErr();
    }
}

static void
gWrite(const void *buf, int len)
{
    if (writer(curtalk->pipe->fd.w, buf, len) != len) {
        logError("Cannot write to %s\n", curtalk->pipe->who);
        gErr();
    }
#ifdef _POSIX_PRIORITY_SCHEDULING
    if ((debugLevel & DEBUG_HLPCON))
        sched_yield();
#endif
}

void
gSendInt(int val)
{
    gDebug("sending int %d (%#x) to %s\n", val, val, curtalk->pipe->who);
    gWrite(&val, sizeof(val));
}

int
gRecvInt()
{
    int val;

    gDebug("receiving int from %s ...\n", curtalk->pipe->who);
    gRead(&val, sizeof(val));
    gDebug(" -> %d (%#x)\n", val, val);
    return val;
}

int
gRecvCmd(int *cmd)
{
    gDebug("receiving command from %s ...\n", curtalk->pipe->who);
    if (reader(curtalk->pipe->fd.r, cmd, sizeof(*cmd)) == sizeof(*cmd)) {
        gDebug(" -> %d\n", *cmd);
        return 1;
    }
    gDebug(" -> no data\n");
    return False;
}

void
gSendArr(int len, const char *data)
{
    gDebug("sending array[%d] %02[*{hhx to %s\n",
           len, len, data, curtalk->pipe->who);
    gWrite(&len, sizeof(len));
    gWrite(data, len);
}

static char *
_gRecvArr(int *rlen)
{
    unsigned len;
    char *buf;

    gRead(&len, sizeof(len));
    *rlen = len;
    gDebug(" -> %d bytes\n", len);
    if (!len || len > 0x10000)
        return 0;
    if (!(buf = Malloc(len)))
        gErr();
    gRead(buf, len);
    return buf;
}

char *
gRecvArr(int *rlen)
{
    char *buf;

    gDebug("receiving array from %s ...\n", curtalk->pipe->who);
    buf = _gRecvArr(rlen);
    gDebug(" -> %02[*{hhx\n", *rlen, buf);
    return buf;
}

static int
_gRecvArrBuf(char *buf)
{
    unsigned len;

    gRead(&len, sizeof(len));
    gDebug(" -> %d bytes\n", len);
    if (len && len < 0x10000)
        gRead(buf, len);
    return len;
}

int
gRecvArrBuf(char *buf)
{
    int len;

    gDebug("receiving already allocated array from %s ...\n",
           curtalk->pipe->who);
    len = _gRecvArrBuf(buf);
    gDebug(" -> %02[*{hhx\n", len, buf);
    return len;
}

int
gRecvStrBuf(char *buf)
{
    int len;

    gDebug("receiving already allocated string from %s ...\n",
           curtalk->pipe->who);
    len = _gRecvArrBuf(buf);
    gDebug(" -> %\".*s\n", len, buf);
    return len;
}

void
gSendStr(const char *buf)
{
    int len;

    gDebug("sending string %\"s to %s\n", buf, curtalk->pipe->who);
    if (buf) {
        len = strlen(buf) + 1;
        gWrite(&len, sizeof(len));
        gWrite(buf, len);
    } else {
        gWrite(&buf, sizeof(int));
    }
}

void
gSendNStr(const char *buf, int len)
{
    int tlen = len + 1;
    gDebug("sending string %\".*s to %s\n", len, buf, curtalk->pipe->who);
    gWrite(&tlen, sizeof(tlen));
    gWrite(buf, len);
    gWrite("", 1);
}

void
gSendStrN(const char *buf, int len)
{
    if (buf)
        gSendNStr(buf, strnlen(buf, len));
    else
        gSendStr(buf);
}

char *
gRecvStr()
{
    int len;
    char *buf;

    gDebug("receiving string from %s ...\n", curtalk->pipe->who);
    buf = _gRecvArr(&len);
    gDebug(" -> %\".*s\n", len, buf);
    return buf;
}

static void
_gSendStrArr(int num, char **data)
{
    char **cdata;

    gWrite(&num, sizeof(num));
    for (cdata = data; --num >= 0; cdata++)
        gSendStr(*cdata);
}

/*
void
gSendStrArr (int num, char **data)
{
    gDebug("sending string array[%d] to %s\n", num, curtalk->pipe->who);
    _gSendStrArr(num, data);
}
*/

char **
gRecvStrArr(int *rnum)
{
    int num;
    char **argv, **cargv;

    gDebug("receiving string array from %s ...\n", curtalk->pipe->who);
    gRead(&num, sizeof(num));
    gDebug(" -> %d strings\n", num);
    *rnum = num;
    if (!num)
        return 0;
    if (!(argv = Malloc(num * sizeof(char *))))
        gErr();
    for (cargv = argv; --num >= 0; cargv++)
        *cargv = gRecvStr();
    return argv;
}

void
gSendArgv(char **argv)
{
    int num;

    if (argv) {
        for (num = 0; argv[num]; num++);
        gDebug("sending argv[%d] to %s ...\n", num, curtalk->pipe->who);
        _gSendStrArr(num + 1, argv);
    } else {
        gDebug("sending NULL argv to %s\n", curtalk->pipe->who);
        gWrite(&argv, sizeof(int));
    }
}

char **
gRecvArgv()
{
    int num;

    return gRecvStrArr(&num);
}

