/* $TOG: session.c /main/79 1998/02/09 13:56:17 kaleb $ */
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

#define NEED_SIGNAL
#include "dm.h"
#include "dm_error.h"

#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>


SIGVAL (*Signal (int sig, SIGFUNC handler))(int)
{
#if !defined(X_NOT_POSIX) && !defined(__EMX__)
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
TerminateProcess (int pid, int sig)
{
    kill (pid, sig);
#ifdef SIGCONT
    kill (pid, SIGCONT);
#endif
}


static FD_TYPE	CloseMask;
static int	max = -1;

void
RegisterCloseOnFork (int fd)
{
    FD_SET (fd, &CloseMask);
    if (fd > max)
	max = fd;
}

void
CloseNClearCloseOnFork (int fd)
{
    close (fd);
    FD_CLR (fd, &CloseMask);
}

static void
CloseOnFork (void)
{
    int	fd;

    for (fd = 0; fd <= max; fd++)
	if (FD_ISSET (fd, &CloseMask))
	    close (fd);
    FD_ZERO (&CloseMask);
    max = -1;
}

int
Fork ()
{
    int pid;

#ifndef X_NOT_POSIX
    sigset_t ss, oss; 
    sigfillset(&ss);
    sigprocmask(SIG_SETMASK, &ss, &oss);
#else
    int omask = sigsetmask (-1);
#endif

    if (!(pid = fork())) {
#ifdef SIGCHLD
	(void) Signal (SIGCHLD, SIG_DFL);
#endif
	(void) Signal (SIGTERM, SIG_DFL);
	(void) Signal (SIGINT, SIG_IGN);	/* for -nodaemon */
	(void) Signal (SIGPIPE, SIG_DFL);
	(void) Signal (SIGALRM, SIG_DFL);
	(void) Signal (SIGHUP, SIG_DFL);
#ifndef X_NOT_POSIX
	sigemptyset(&ss);
	sigprocmask(SIG_SETMASK, &ss, NULL);
#else
	sigsetmask (0);
#endif
	CloseOnFork ();
	return 0;
    }

#ifndef X_NOT_POSIX
    sigprocmask(SIG_SETMASK, &oss, 0);
#else
    sigsetmask (omask);
#endif

    return pid;
}

int
Wait4 (int pid)
{
    waitType	result;

#ifndef X_NOT_POSIX
    while (waitpid (pid, &result, 0) < 0)
#else
    while (wait4 (pid, &result, 0, (struct rusage *)0) < 0)
#endif
	if (errno != EINTR)
	{
	    Debug ("Wait4(%d) failed: %s\n", pid, SysErrorMsg() );
	    return 0;
	}
    return waitVal (result);
}


void
execute (char **argv, char **env)
{
    Debug ("execute: %[s ; %[s\n", argv, env);
    execve (argv[0], argv, env);
    /*
     * In case this is a shell script which hasn't been
     * made executable (or this is a SYSV box), do
     * a reasonable thing
     */
    if (errno != ENOENT) {
	char	*e, **newargv;
	FILE	*f;
	char	program[1024];

	/*
	 * emulate BSD kernel behaviour -- read
	 * the first line; check if it starts
	 * with "#!", in which case it uses
	 * the rest of the line as the name of
	 * program to run.  Else use "/bin/sh".
	 */
	if (!(f = fopen (argv[0], "r")))
	    return;
	if (!fgets (program, sizeof(program), f))
	{
	    fclose (f);
	    return;
	}
	fclose (f);
	e = program + strlen (program) - 1;
	if (*e == '\n')
	    *e = '\0';
	if (!strncmp (program, "#!", 2))
	    newargv = parseArgs (0, program + 2);
	else
	    newargv = addStrArr (0, "/bin/sh", 7);
	mergeStrArrs (&newargv, argv);
	Debug ("shell script execution: %[s\n", newargv);
	execve (newargv[0], newargv, env);
    }
}

int
runAndWait (char **args, char **env)
{
    int	pid, ret;

    switch (pid = Fork ()) {
    case 0:
	execute (args, env);
	LogError ("Can't execute %\"s: %s\n", args[0], SysErrorMsg());
	exit (1);
    case -1:
	LogError ("Can't fork to execute %\"s: %s\n", args[0], SysErrorMsg());
	return 1;
    }
    ret = Wait4 (pid);
    return waitVal (ret);
}


static GTalk *curtalk;

void
GSet (GTalk *tlk)
{
    curtalk = tlk;
}

int
GFork (GPipe *pajp, const char *pname, char *cname)
{
    int opipe[2], ipipe[2], pid;

    if (pipe (opipe)) {
	LogError ("Cannot start %s, pipe() failed", cname);
	if (cname)
	     free (cname);
	return -1;
    }
    if (pipe (ipipe)) {
	close (opipe[0]);
	close (opipe[1]);
	LogError ("Cannot start %s, pipe() failed", cname);
	if (cname)
	     free (cname);
	return -1;
    }
    RegisterCloseOnFork (opipe[1]);
    RegisterCloseOnFork (ipipe[0]);
    switch (pid = Fork()) {
    case -1:
	close (opipe[0]);
	close (ipipe[1]);
	CloseNClearCloseOnFork (opipe[1]);
	CloseNClearCloseOnFork (ipipe[0]);
	LogError ("Cannot start %s, fork() failed\n", cname);
	if (cname)
	     free (cname);
	return -1;
    case 0:
	pajp->wfd = ipipe[1];
	RegisterCloseOnFork (ipipe[1]);
	pajp->rfd = opipe[0];
	RegisterCloseOnFork (opipe[0]);
	pajp->who = (char *)pname;
	break;
    default:
	close (opipe[0]);
	close (ipipe[1]);
	pajp->rfd = ipipe[0];
	pajp->wfd = opipe[1];
	pajp->who = cname;
	break;
    }
    return pid;
}

int
GOpen (GProc *proc, char **argv, const char *what, char **env, char *cname)
{
    char **margv;
    int pip[2];
    char coninfo[20];

/* ###   GSet (proc->pipe); */
    if (proc->pid) {
	LogError ("%s already running\n", cname);
	if (cname)
	     free (cname);
	return -1;
    }
    if (!(margv = xCopyStrArr (1, argv))) {
	LogOutOfMem ("GOpen");
	if (cname)
	     free (cname);
	return -1;
    }
    if (!StrApp (margv, progpath, what, (char *)0)) {
	free (margv);
	LogOutOfMem ("GOpen");
	if (cname)
	     free (cname);
	return -1;
    }
    if (pipe (pip)) {
	LogError ("Cannot start %s, pipe() failed\n", cname);
	if (cname)
	     free (cname);
	goto fail;
    }
    switch (proc->pid = GFork (&proc->pipe, 0, cname)) {
    case -1:
	close (pip[1]);
      fail1:
	close (pip[0]);
      fail:
	free (margv[0]);
	free (margv);
	return -1;
    case 0:
	(void) Signal (SIGPIPE, SIG_IGN);
	close (pip[0]);
	fcntl (pip[1], F_SETFD, FD_CLOEXEC);
	sprintf (coninfo, "CONINFO=%d %d", proc->pipe.rfd, proc->pipe.wfd);
	env = putEnv (coninfo, env);
	execute (margv, env);
	write (pip[1], "", 1);
	exit (1);
    default:
	(void) Signal (SIGPIPE, SIG_IGN);
	close (pip[1]);
	if (Reader (pip[0], coninfo, 1)) {
	    Wait4 (proc->pid);
	    LogError ("Cannot execute %\"s (%s)\n", margv[0], cname);
	    GClosen (&proc->pipe);
	    goto fail1;
	}
	close (pip[0]);
	Debug ("started %s (%\"s), pid %d\n", cname, margv[0], proc->pid);
	free (margv[0]);
	free (margv);
	GSendInt (debugLevel);
	return 0;
    }
}

static void
iGClosen (GPipe *pajp)
{
    CloseNClearCloseOnFork (pajp->rfd);
    CloseNClearCloseOnFork (pajp->wfd);
    pajp->rfd = pajp->wfd = -1;
}

void
GClosen (GPipe *pajp)
{
    iGClosen (pajp);
    if (pajp->who)
	free (pajp->who);
    pajp->who = 0;
}

int
GClose (GProc *proc, int force)
{
    int	ret;

    if (!proc->pid) {
	Debug ("whoops, GClose while helper not running\n");
	return 0;
    }
    iGClosen (&proc->pipe);
    if (force)
	TerminateProcess (proc->pid, SIGTERM);
    ret = Wait4 (proc->pid);
    proc->pid = 0;
    if (WaitSig(ret) ? WaitSig(ret) != SIGTERM :
        (WaitCode(ret) < EX_NORMAL || WaitCode(ret) > EX_MAX))
	LogError ("Abnormal termination of %s, code %d, signal %d\n", 
		  proc->pipe.who, WaitCode(ret), WaitSig(ret));
    if (proc->pipe.who)
	free (proc->pipe.who);
    proc->pipe.who = 0;
    return ret;
}

static void ATTR_NORETURN
GErr (void)
{
    Longjmp (curtalk->errjmp, 1);
}

static void
GRead (void *buf, int len)
{
    if (Reader (curtalk->pipe->rfd, buf, len) != len) {
	LogError ("Cannot read from %s\n", curtalk->pipe->who);
	GErr ();
    }
}

static void
GWrite (const void *buf, int len)
{
#if 0
    int ret;
    do {
	ret = write (curtalk->pipe->wfd, buf, len);
    } while (ret < 0 && errno == EINTR);
    if (ret != len) {
#else
    if (write (curtalk->pipe->wfd, buf, len) != len) {
#endif
	LogError ("Cannot write to %s\n", curtalk->pipe->who);
	GErr ();
    }
}

void
GSendInt (int val)
{
    GDebug ("sending int %d (%#x) to %s\n", val, val, curtalk->pipe->who);
    GWrite (&val, sizeof(val));
}

int
GRecvInt ()
{
    int val;

    GDebug ("receiving int from %s ...\n", curtalk->pipe->who);
    GRead (&val, sizeof(val));
    GDebug (" -> %d (%#x)\n", val, val);
    return val;
}

int
GRecvCmd (int *cmd)
{
    GDebug ("receiving command from %s ...\n", curtalk->pipe->who);
    if (Reader (curtalk->pipe->rfd, cmd, sizeof(*cmd)) == sizeof(*cmd)) {
	GDebug (" -> %d\n", *cmd);
	return 1;
    }
    GDebug (" -> no data\n");
    return 0;
}

#ifdef XDMCP
void
GSendArr (int len, const char *data)
{
    GDebug ("sending array[%d] %02[*{hhx to %s\n", len, len, data, curtalk->pipe->who);
    GWrite (&len, sizeof(len));
    GWrite (data, len);
}
#endif

static char *
iGRecvArr (int *rlen)
{
    int len;
    char *buf;

    GRead (&len, sizeof (len));
    *rlen = len;
    GDebug (" -> %d bytes\n", len);
    if (!len)
	return (char *)0;
    if (!(buf = malloc (len)))
    {
	LogOutOfMem ("GRecvArr");
	GErr ();
    }
    GRead (buf, len);
    return buf;
}

char *
GRecvArr (int *rlen)
{
    char *buf;

    GDebug ("receiving array from %s ...\n", curtalk->pipe->who);
    buf = iGRecvArr (rlen);
    GDebug (" -> %02[*{hhx\n", *rlen, buf);
    return buf;
}

static int
iGRecvArrBuf (char *buf)
{
    int len;

    GRead (&len, sizeof (len));
    GDebug (" -> %d bytes\n", len);
    if (len)
	GRead (buf, len);
    return len;
}

int
GRecvArrBuf (char *buf)
{
    int len;

    GDebug ("receiving already allocated array from %s ...\n", curtalk->pipe->who);
    len = iGRecvArrBuf (buf);
    GDebug (" -> %02[*{hhx\n", len, buf);
    return len;
}

int
GRecvStrBuf (char *buf)
{
    int len;

    GDebug ("receiving already allocated string from %s ...\n", curtalk->pipe->who);
    len = iGRecvArrBuf (buf);
    GDebug (" -> %\".*s\n", len, buf);
    return len;
}

void
GSendStr (const char *buf)
{
    int len;

    GDebug ("sending string %\"s to %s\n", buf, curtalk->pipe->who);
    if (buf) {
	len = strlen (buf) + 1;
	GWrite (&len, sizeof(len));
	GWrite (buf, len);
    } else
	GWrite (&buf, sizeof(int));
}

void
GSendNStr (const char *buf, int len)
{
    int tlen = len + 1;
    GDebug ("sending string %\".*s to %s\n", len, buf, curtalk->pipe->who);
    GWrite (&tlen, sizeof(tlen));
    GWrite (buf, len);
    GWrite ("", 1);
}

char *
GRecvStr ()
{
    int len;
    char *buf;

    GDebug ("receiving string from %s ...\n", curtalk->pipe->who);
    buf = iGRecvArr (&len);
    GDebug (" -> %\".*s\n", len, buf);
    return buf;
}

static void
iGSendStrArr (int num, char **data)
{
    char **cdata;

    GWrite (&num, sizeof(num));
    for (cdata = data; --num >= 0; cdata++)
	GSendStr (*cdata);
}

/*
void
GSendStrArr (int num, char **data)
{
    GDebug ("sending string array[%d] to %s\n", num, curtalk->pipe->who);
    iGSendStrArr (num, data);
}
*/

char **
GRecvStrArr (int *rnum)
{
    int num;
    char **argv, **cargv;

    GDebug ("receiving string array from %s ...\n", curtalk->pipe->who);
    GRead (&num, sizeof(num));
    GDebug (" -> %d strings\n", num);
    *rnum = num;
    if (!num)
	return (char **)0;
    if (!(argv = malloc (num * sizeof(char *))))
    {
	LogOutOfMem ("GRecvStrArr");
	GErr ();
    }
    for (cargv = argv; --num >= 0; cargv++)
	*cargv = GRecvStr ();
    return argv;
}

void
GSendArgv (char **argv)
{
    int num;

    if (argv) {
	for (num = 0; argv[num]; num++);
	GDebug ("sending argv[%d] to %s ...\n", num, curtalk->pipe->who);
	iGSendStrArr (num + 1, argv);
    } else {
	GDebug ("sending NULL argv to %s\n", curtalk->pipe->who);
	GWrite (&argv, sizeof(int));
    }
}

char **
GRecvArgv ()
{
    int num;

    return GRecvStrArr (&num);
}

