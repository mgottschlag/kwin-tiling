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

#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>
#include <signal.h>
#include <unistd.h>
#ifdef _POSIX_PRIORITY_SCHEDULING
# include <sched.h>
#endif

extern char **environ;


SIGFUNC Signal( int sig, SIGFUNC handler )
{
#ifndef __EMX__
	struct sigaction sigact, osigact;
	sigact.sa_handler = handler;
	sigemptyset( &sigact.sa_mask );
# ifdef SA_RESTART
	sigact.sa_flags = SA_RESTART;
# else
	sigact.sa_flags = 0;
# endif
	sigaction( sig, &sigact, &osigact );
	return osigact.sa_handler;
#else
	return signal( sig, handler );
#endif
}


void
TerminateProcess( int pid, int sig )
{
	kill( pid, sig );
#ifdef SIGCONT
	kill( pid, SIGCONT );
#endif
}


static FD_TYPE CloseMask;
static int max = -1;

void
RegisterCloseOnFork( int fd )
{
	FD_SET( fd, &CloseMask );
	if (fd > max)
		max = fd;
}

void
ClearCloseOnFork( int fd )
{
	FD_CLR( fd, &CloseMask );
}

void
CloseNClearCloseOnFork( int fd )
{
	close( fd );
	FD_CLR( fd, &CloseMask );
}

static void
CloseOnFork( void )
{
	int fd;

	for (fd = 0; fd <= max; fd++)
		if (FD_ISSET( fd, &CloseMask ))
			close( fd );
	FD_ZERO( &CloseMask );
	max = -1;
}

int
Fork()
{
	int pid;

	sigset_t ss, oss;
	sigfillset( &ss );
	sigprocmask( SIG_SETMASK, &ss, &oss );

	if (!(pid = fork())) {
#ifdef SIGCHLD
		(void)Signal( SIGCHLD, SIG_DFL );
#endif
		(void)Signal( SIGTERM, SIG_DFL );
		(void)Signal( SIGINT, SIG_IGN ); /* for -nodaemon */
		(void)Signal( SIGPIPE, SIG_DFL );
		(void)Signal( SIGALRM, SIG_DFL );
		(void)Signal( SIGHUP, SIG_DFL );
		sigemptyset( &ss );
		sigprocmask( SIG_SETMASK, &ss, NULL );
		CloseOnFork();
		return 0;
	}

	sigprocmask( SIG_SETMASK, &oss, 0 );

	return pid;
}

int
Wait4( int pid )
{
	waitType result;

	while (waitpid( pid, &result, 0 ) < 0)
		if (errno != EINTR) {
			Debug( "Wait4(%d) failed: %m\n", pid );
			return 0;
		}
	return waitVal( result );
}


void
execute( char **argv, char **env )
{
	Debug( "execute: %[s ; %[s\n", argv, env );
	execve( argv[0], argv, env );
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
		 * program to run.	Else use "/bin/sh".
		 */
		if (!(f = fopen( argv[0], "r" )))
			return;
		if (!fGets( program, sizeof(program), f )) {
			fclose( f );
			return;
		}
		fclose( f );
		if (!strncmp( program, "#!", 2 ))
			newargv = parseArgs( 0, program + 2 );
		else
			newargv = addStrArr( 0, "/bin/sh", 7 );
		if (!newargv)
			return;
		nu = arrLen( newargv );
		if (!(argv = xCopyStrArr( nu, argv )))
			return;
		memcpy( argv, newargv, sizeof(char *) * nu );
		Debug( "shell script execution: %[s\n", argv );
		execve( argv[0], argv, env );
	}
}

int
runAndWait( char **args, char **env )
{
	int pid, ret;

	switch (pid = Fork()) {
	case 0:
		execute( args, env );
		LogError( "Can't execute %\"s: %m\n", args[0] );
		exit( 127 );
	case -1:
		LogError( "Can't fork to execute %\"s: %m\n", args[0] );
		return 1;
	}
	ret = Wait4( pid );
	return waitVal( ret );
}

FILE *
pOpen( char **what, char m, int *pid )
{
	int dp[2];

	if (pipe( dp ))
		return 0;
	switch ((*pid = Fork())) {
	case 0:
		if (m == 'r')
			dup2( dp[1], 1 );
		else
			dup2( dp[0], 0 );
		close( dp[0] );
		close( dp[1] );
		execute( what, environ );
		LogError( "Can't execute %\"s: %m\n", what[0] );
		exit( 127 );
	case -1:
		close( dp[0] );
		close( dp[1] );
		LogError( "Can't fork to execute %\"s: %m\n", what[0] );
		return 0;
	}
	if (m == 'r') {
		close( dp[1] );
		return fdopen( dp[0], "r" );
	} else {
		close( dp[0] );
		return fdopen( dp[1], "w" );
	}
}

int
pClose( FILE *f, int pid )
{
	fclose( f );
	return Wait4( pid );
}

char *
locate( const char *exe )
{
	int len;
	char *path, *pathe, *name, *thenam, nambuf[PATH_MAX+1];

	if (!(path = getenv( "PATH" ))) {
		LogError( "Can't execute %'s: $PATH not set.\n", exe );
		return 0;
	}
	len = strlen( exe );
	name = nambuf + PATH_MAX - len;
	memcpy( name, exe, len + 1 );
	*--name = '/';
	do {
		if (!(pathe = strchr( path, ':' )))
			pathe = path + strlen( path );
		len = pathe - path;
		if (len && !(len == 1 && *path == '.')) {
			thenam = name - len;
			if (thenam >= nambuf) {
				memcpy( thenam, path, len );
				if (!access( thenam, X_OK )) {
					StrDup( &name, thenam );
					return name;
				}
			}
		}
		path = pathe;
	} while (*path++ != '\0');
	LogError( "Can't execute %'s: not in $PATH.\n", exe );
	return 0;
}


static GTalk *curtalk;

void
GSet( GTalk *tlk )
{
	curtalk = tlk;
}

int
GFork( GPipe *pajp, const char *pname, char *cname,
       GPipe *ogp, char *cgname )
{
	int opipe[2], ipipe[2], ogpipe[2], igpipe[2], pid;

	if (pipe( opipe ))
		goto badp1;
	if (pipe( ipipe ))
		goto badp2;
	if (ogp) {
		if (pipe( ogpipe ))
			goto badp3;
		if (pipe( igpipe )) {
			close( ogpipe[0] );
			close( ogpipe[1] );
		  badp3:
			close( ipipe[0] );
			close( ipipe[1] );
		  badp2:
			close( opipe[0] );
			close( opipe[1] );
		  badp1:
			LogError( "Cannot start %s, pipe() failed", cname );
			if (cname)
				free( cname );
			return -1;
		}
	}
	RegisterCloseOnFork( opipe[1] );
	RegisterCloseOnFork( ipipe[0] );
	if (ogp) {
		RegisterCloseOnFork( ogpipe[1] );
		RegisterCloseOnFork( igpipe[0] );
	}
	switch (pid = Fork()) {
	case -1:
		close( opipe[0] );
		close( ipipe[1] );
		CloseNClearCloseOnFork( opipe[1] );
		CloseNClearCloseOnFork( ipipe[0] );
		if (ogp) {
			close( ogpipe[0] );
			close( igpipe[1] );
			CloseNClearCloseOnFork( ogpipe[1] );
			CloseNClearCloseOnFork( igpipe[0] );
		}
		LogError( "Cannot start %s, fork() failed\n", cname );
		if (cname)
			 free( cname );
		return -1;
	case 0:
		pajp->wfd = ipipe[1];
		RegisterCloseOnFork( ipipe[1] );
		pajp->rfd = opipe[0];
		RegisterCloseOnFork( opipe[0] );
		pajp->who = (char *)pname;
		if (ogp) {
			ogp->wfd = igpipe[1];
			RegisterCloseOnFork( igpipe[1] );
			ogp->rfd = ogpipe[0];
			RegisterCloseOnFork( ogpipe[0] );
			ogp->who = (char *)pname;
		}
		break;
	default:
		close( opipe[0] );
		close( ipipe[1] );
		pajp->rfd = ipipe[0];
		pajp->wfd = opipe[1];
		pajp->who = cname;
		if (ogp) {
			close( ogpipe[0] );
			close( igpipe[1] );
			ogp->rfd = igpipe[0];
			ogp->wfd = ogpipe[1];
			ogp->who = cgname;
		}
		break;
	}
	return pid;
}

int
GOpen( GProc *proc, char **argv, const char *what, char **env, char *cname,
       GPipe *gp )
{
	char **margv;
	int pip[2];
	char coninfo[20];

/* ###	GSet (proc->pipe); */
	if (proc->pid) {
		LogError( "%s already running\n", cname );
		if (cname)
			free( cname );
		return -1;
	}
	if (!(margv = xCopyStrArr( 1, argv ))) {
		if (cname)
			free( cname );
		return -1;
	}
	if (!StrApp( margv, progpath, what, (char *)0 )) {
		free( margv );
		if (cname)
			free( cname );
		return -1;
	}
	if (pipe( pip )) {
		LogError( "Cannot start %s, pipe() failed\n", cname );
		if (cname)
			free( cname );
		goto fail;
	}
	if (gp) {
		ClearCloseOnFork( gp->rfd );
		ClearCloseOnFork( gp->wfd );
	}
	proc->pid = GFork( &proc->pipe, 0, cname, 0, 0 );
	if (proc->pid) {
		close( pip[1] );
		if (gp) {
			RegisterCloseOnFork( gp->rfd );
			RegisterCloseOnFork( gp->wfd );
		}
	}
	switch (proc->pid) {
	case -1:
	  fail1:
		close( pip[0] );
	  fail:
		free( margv[0] );
		free( margv );
		return -1;
	case 0:
		(void)Signal( SIGPIPE, SIG_IGN );
		close( pip[0] );
		fcntl( pip[1], F_SETFD, FD_CLOEXEC );
		if (gp)
			sprintf( coninfo, "CONINFO=%d %d %d %d",
			         proc->pipe.rfd, proc->pipe.wfd, gp->rfd, gp->wfd );
		else
			sprintf( coninfo, "CONINFO=%d %d",
			         proc->pipe.rfd, proc->pipe.wfd );
		env = putEnv( coninfo, env );
		if (debugLevel & DEBUG_VALGRIND) {
			char **nmargv = xCopyStrArr( 3, margv );
			nmargv[0] = locate( "valgrind" );
			nmargv[1] = (char *)"--tool=memcheck";
			nmargv[2] = (char *)"--num-callers=8";
			execute( nmargv, env );
		} else if (debugLevel & DEBUG_STRACE) {
			char **nmargv = xCopyStrArr( 1, margv );
			nmargv[0] = locate( "strace" );
			execute( nmargv, env );
		} else
			execute( margv, env );
		write( pip[1], "", 1 );
		exit( 1 );
	default:
		(void)Signal( SIGPIPE, SIG_IGN );
		if (Reader( pip[0], coninfo, 1 )) {
			Wait4( proc->pid );
			LogError( "Cannot execute %\"s (%s)\n", margv[0], cname );
			GClosen (&proc->pipe);
			goto fail1;
		}
		close( pip[0] );
		Debug( "started %s (%\"s), pid %d\n", cname, margv[0], proc->pid );
		free( margv[0] );
		free( margv );
		GSendInt( debugLevel );
		return 0;
	}
}

static void
iGClosen( GPipe *pajp )
{
	CloseNClearCloseOnFork( pajp->rfd );
	CloseNClearCloseOnFork( pajp->wfd );
	pajp->rfd = pajp->wfd = -1;
}

void
GClosen (GPipe *pajp)
{
	iGClosen( pajp );
	if (pajp->who)
		free( pajp->who );
	pajp->who = 0;
}

int
GClose (GProc *proc, GPipe *gp, int force)
{
	int ret;

	if (!proc->pid) {
		Debug( "whoops, GClose while helper not running\n" );
		return 0;
	}
	iGClosen( &proc->pipe );
	if (gp)
		GClosen (gp);
	if (force)
		TerminateProcess( proc->pid, SIGTERM );
	ret = Wait4( proc->pid );
	proc->pid = 0;
	if (WaitSig( ret ) ? WaitSig( ret ) != SIGTERM :
	    (WaitCode( ret ) < EX_NORMAL || WaitCode( ret ) > EX_MAX))
		LogError( "Abnormal termination of %s, code %d, signal %d\n",
		          proc->pipe.who, WaitCode( ret ), WaitSig( ret ) );
	Debug( "closed %s\n", proc->pipe.who );
	if (proc->pipe.who)
		free( proc->pipe.who );
	proc->pipe.who = 0;
	return ret;
}

static void ATTR_NORETURN
GErr( void )
{
	Longjmp( curtalk->errjmp, 1 );
}

static void
GRead( void *buf, int len )
{
	if (Reader( curtalk->pipe->rfd, buf, len ) != len) {
		LogError( "Cannot read from %s\n", curtalk->pipe->who );
		GErr();
	}
}

static void
GWrite( const void *buf, int len )
{
	if (Writer( curtalk->pipe->wfd, buf, len ) != len) {
		LogError( "Cannot write to %s\n", curtalk->pipe->who );
		GErr();
	}
#ifdef _POSIX_PRIORITY_SCHEDULING
	if ((debugLevel & DEBUG_HLPCON))
		sched_yield();
#endif
}

void
GSendInt( int val )
{
	GDebug( "sending int %d (%#x) to %s\n", val, val, curtalk->pipe->who );
	GWrite( &val, sizeof(val) );
}

int
GRecvInt()
{
	int val;

	GDebug( "receiving int from %s ...\n", curtalk->pipe->who );
	GRead( &val, sizeof(val) );
	GDebug( " -> %d (%#x)\n", val, val );
	return val;
}

int
GRecvCmd( int *cmd )
{
	GDebug( "receiving command from %s ...\n", curtalk->pipe->who );
	if (Reader( curtalk->pipe->rfd, cmd, sizeof(*cmd) ) == sizeof(*cmd)) {
		GDebug( " -> %d\n", *cmd );
		return 1;
	}
	GDebug( " -> no data\n" );
	return 0;
}

void
GSendArr( int len, const char *data )
{
	GDebug( "sending array[%d] %02[*{hhx to %s\n",
	        len, len, data, curtalk->pipe->who );
	GWrite( &len, sizeof(len) );
	GWrite( data, len );
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
	if (!(buf = Malloc( len )))
		GErr();
	GRead( buf, len );
	return buf;
}

char *
GRecvArr( int *rlen )
{
	char *buf;

	GDebug( "receiving array from %s ...\n", curtalk->pipe->who );
	buf = iGRecvArr( rlen );
	GDebug( " -> %02[*{hhx\n", *rlen, buf );
	return buf;
}

static int
iGRecvArrBuf( char *buf )
{
	int len;

	GRead( &len, sizeof(len) );
	GDebug( " -> %d bytes\n", len );
	if (len)
		GRead( buf, len );
	return len;
}

int
GRecvArrBuf( char *buf )
{
	int len;

	GDebug( "receiving already allocated array from %s ...\n",
	        curtalk->pipe->who );
	len = iGRecvArrBuf( buf );
	GDebug( " -> %02[*{hhx\n", len, buf );
	return len;
}

int
GRecvStrBuf( char *buf )
{
	int len;

	GDebug( "receiving already allocated string from %s ...\n",
	        curtalk->pipe->who );
	len = iGRecvArrBuf( buf );
	GDebug( " -> %\".*s\n", len, buf );
	return len;
}

void
GSendStr( const char *buf )
{
	int len;

	GDebug( "sending string %\"s to %s\n", buf, curtalk->pipe->who );
	if (buf) {
		len = strlen( buf ) + 1;
		GWrite( &len, sizeof(len) );
		GWrite( buf, len );
	} else
		GWrite( &buf, sizeof(int));
}

void
GSendNStr( const char *buf, int len )
{
	int tlen = len + 1;
	GDebug( "sending string %\".*s to %s\n", len, buf, curtalk->pipe->who );
	GWrite( &tlen, sizeof(tlen) );
	GWrite( buf, len );
	GWrite( "", 1 );
}

void
GSendStrN( const char *buf, int len )
{
	if (buf)
		GSendNStr( buf, StrNLen( buf, len ) );
	else
		GSendStr( buf );
}

char *
GRecvStr()
{
	int len;
	char *buf;

	GDebug( "receiving string from %s ...\n", curtalk->pipe->who );
	buf = iGRecvArr( &len );
	GDebug( " -> %\".*s\n", len, buf );
	return buf;
}

static void
iGSendStrArr( int num, char **data )
{
	char **cdata;

	GWrite( &num, sizeof(num) );
	for (cdata = data; --num >= 0; cdata++)
		GSendStr( *cdata );
}

/*
void
GSendStrArr (int num, char **data)
{
	GDebug( "sending string array[%d] to %s\n", num, curtalk->pipe->who );
	iGSendStrArr( num, data );
}
*/

char **
GRecvStrArr( int *rnum )
{
	int num;
	char **argv, **cargv;

	GDebug( "receiving string array from %s ...\n", curtalk->pipe->who );
	GRead( &num, sizeof(num) );
	GDebug( " -> %d strings\n", num );
	*rnum = num;
	if (!num)
		return (char **)0;
	if (!(argv = Malloc( num * sizeof(char *))))
		GErr();
	for (cargv = argv; --num >= 0; cargv++)
		*cargv = GRecvStr();
	return argv;
}

void
GSendArgv( char **argv )
{
	int num;

	if (argv) {
		for (num = 0; argv[num]; num++);
		GDebug( "sending argv[%d] to %s ...\n", num, curtalk->pipe->who );
		iGSendStrArr( num + 1, argv );
	} else {
		GDebug( "sending NULL argv to %s\n", curtalk->pipe->who );
		GWrite( &argv, sizeof(int));
	}
}

char **
GRecvArgv()
{
	int num;

	return GRecvStrArr( &num );
}

