/* $TOG: error.c /main/17 1998/02/09 13:55:13 kaleb $ */
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
/* $XFree86: xc/programs/xdm/error.c,v 1.2 1998/10/10 15:25:34 dawes Exp $ */

/*
 * xdm - display manager daemon
 * Author:  Keith Packard, MIT X Consortium
 *
 * error.c
 *
 * Log display manager errors to a file as
 * we generally do not have a terminal to talk to
 * or use syslog if it exists
 */

# include <stdio.h>
# include <stdarg.h>

# include "dm.h"
# include "dm_error.h"

#ifdef HAVE_SYSLOG_H
#include <syslog.h>
#endif

/*VARARGS1*/
void 
LogInfo(const char *fmt, ...)
{
#  ifndef USE_SYSLOG
    char fmt1[256];
#  endif
    va_list args;

    va_start(args, fmt);
#  ifdef USE_SYSLOG
    openlog("kdm", LOG_PID, LOG_DAEMON);
    vsyslog (LOG_INFO, fmt, args);
#  else
    sprintf (fmt1, "xdm info (pid %d): %s", (int)getpid(), fmt);
    vfprintf (stderr, fmt1, args);
    fflush (stderr);
#  endif
    va_end(args);
}

/*VARARGS1*/
void
LogError (const char *fmt, ...)
{
#  ifndef USE_SYSLOG
    char fmt1[256];
#  endif
    va_list args;

    va_start(args, fmt);
#  ifdef USE_SYSLOG
    openlog("kdm", LOG_PID, LOG_DAEMON);
    vsyslog (LOG_ERR, fmt, args);
#  else
    sprintf (fmt1, "xdm error (pid %d): %s", (int)getpid(), fmt);
    vfprintf (stderr, fmt1, args);
    fflush (stderr);
#  endif
    va_end(args);
}

/*VARARGS1*/
void
LogPanic (const char *fmt, ...)
{
#  ifndef USE_SYSLOG
    char fmt1[256];
#  endif
    va_list args;

    va_start(args, fmt);
#  ifdef USE_SYSLOG
    openlog("kdm", LOG_PID, LOG_DAEMON);
    vsyslog (LOG_EMERG, fmt, args);
#  else
    sprintf (fmt1, "xdm panic (pid %d): %s", (int)getpid(), fmt);
    vfprintf (stderr, fmt1, args);
    fflush (stderr);
#  endif
    va_end(args);
    exit (1);
}

/*VARARGS1*/
void
LogOutOfMem (const char *fmt, ...)
{
    char fmt1[256];
    va_list args;

    va_start(args, fmt);
#  ifdef USE_SYSLOG
    openlog("kdm", LOG_PID, LOG_DAEMON);
    sprintf (fmt1, "out of memory in routine %s", fmt);
    vsyslog (LOG_ALERT, fmt1, args);
#  else
    sprintf (fmt1, "kdm: out of memory in routine %s", fmt);
    vfprintf (stderr, fmt1, args);
    fflush (stderr);
#  endif
    va_end(args);
}

void
Panic (const char *mesg)
{
#ifdef USE_SYSLOG
    openlog("kdm", LOG_PID, LOG_DAEMON);
    syslog(LOG_EMERG, mesg);
#else
    int	i;

    i = creat ("/dev/console", 0666);
    write (i, "kdm panic: ", 7);
    write (i, mesg, strlen (mesg));
#endif
    exit (1);
}


/*VARARGS1*/
void
Debug (const char *fmt, ...)
{
#  ifndef USE_SYSLOG
    char fmt1[256];
#  endif
    va_list args;

    if (debugLevel > 0)
    {
	va_start(args, fmt);
#  ifdef USE_SYSLOG
	openlog("kdm", LOG_PID, LOG_DAEMON);
	vsyslog (LOG_DEBUG, fmt, args);
#  else
	sprintf(fmt1, "%d: %s", getpid(), fmt);
	vprintf (fmt1, args);
	fflush (stdout);
#  endif
	va_end(args);
    }
}

void
InitErrorLog ()
{
#ifdef USE_SYSLOG
	/* aw shit! PAM re-opens the log with "bad" values ...
	   so we have to re-open it on every message *grmph*
	*/
#else
	int	i;
	if (errorLogFile[0]) {
		i = creat (errorLogFile, 0666);
		if (i != -1) {
			if (i != 2) {
				dup2 (i, 2);
				close (i);
			}
		} else
			LogError ("Cannot open errorLogFile %s\n", errorLogFile);
	}
#endif
}
