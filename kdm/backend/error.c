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

#include "dm.h"
#include "dm_error.h"

#include <stdio.h>
#include <ctype.h>

#ifdef USE_SYSLOG
# include <syslog.h>
#endif

#ifdef USE_SYSLOG
static int lognums[] = { LOG_DEBUG, LOG_INFO, LOG_ERR, LOG_CRIT };
#else
static char *lognams[] = { "debug", "info", "error", "panic" };
#endif

void
GLogger (const char *who, int type, const char *msg)
{
#ifdef USE_SYSLOG
    openlog(prog, LOG_PID, LOG_DAEMON);
    syslog (lognums[type], "%s: %s", who, msg);
#else
    Time_t tim;
    char dbuf[20];

    (void) time (&tim);
    strftime (dbuf, sizeof(dbuf), "%b %e %H:%M:%S", localtime (&tim));
    fprintf (stderr, "%s %s[%d] %s: %s: %s\n", 
	     dbuf, prog, (int)getpid(), lognams[type], who, msg);
    fflush (stderr);
#endif
}

#ifdef USE_SYSLOG
# define OCLBufMisc int fl;
# define OCLBufInit oclb.fl = 0;
# define OCLBufPrint \
	if (!oclbp->fl) { \
	    oclbp->fl = 1; \
	    openlog(prog, LOG_PID, LOG_DAEMON); \
	} \
	syslog (lognums[oclbp->type], "%.*s", oclbp->clen, oclbp->buf);
#else
# define OCLBufMisc
# define OCLBufInit
# define OCLBufPrint \
	Time_t tim; \
	char dbuf[20]; \
	(void) time (&tim); \
	strftime (dbuf, sizeof(dbuf), "%b %e %H:%M:%S", localtime (&tim)); \
	fprintf (stderr, "%s %s[%d] %s: %.*s\n", dbuf, prog, (int)getpid(), \
		 lognams[oclbp->type], oclbp->clen, oclbp->buf); \
	fflush (stderr);
#endif
#define PRINT_QUOTES
#define PRINT_ARRAYS
#include "printf.c"

void
Debug (const char *fmt, ...)
{
    va_list args;

    if (debugLevel & DEBUG_CORE)
    {
	va_start(args, fmt);
	Logger (DM_DEBUG, fmt, args);
	va_end(args);
    }
}

void
GDebug (const char *fmt, ...)
{
    va_list args;

    if (debugLevel & DEBUG_HLPCON)
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
    exit (1);
}

void
LogOutOfMem (const char *fkt)
{
#ifdef USE_SYSLOG
    openlog(prog, LOG_PID, LOG_DAEMON);
    syslog (LOG_ALERT, "Out of memory in %s()", fkt);
#else
    fprintf (stderr, "%s[%d] %s: Out of memory in %s()\n", 
	     prog, (int)getpid(), lognams[DM_ERR], fkt);
    fflush (stderr);
#endif
}

void
Panic (const char *mesg)
{
#ifdef USE_SYSLOG
    openlog(prog, LOG_PID, LOG_DAEMON);
    syslog(LOG_EMERG, mesg);
#else
    int i = creat ("/dev/console", 0666);
    write (i, "xdm panic: ", 11);
    write (i, mesg, strlen (mesg));
#endif
    exit (1);
}


void
InitErrorLog (const char *errorLogFile)
{
    /* We do this independently of using syslog, as we cannot redirect
     * the output of external programs to syslog.
     */
    if (isatty (2)) {
	char buf[100];
	if (!errorLogFile) {
	    sprintf (buf, "/var/log/%s.log", prog);
	    errorLogFile = buf;
	}
	if (!freopen (errorLogFile, "a", stderr))
	    LogError ("Cannot open log file %s\n", errorLogFile);
    }
    dup2 (2, 1);
}

