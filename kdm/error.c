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
#include <stdarg.h>

#ifdef HAVE_SYSLOG_H
# include <syslog.h>
#endif

enum { DM_DEBUG, DM_INFO, DM_ERR, DM_EMERG };
#ifdef USE_SYSLOG
static int lognums[] = { LOG_DEBUG, LOG_INFO, LOG_ERR, LOG_EMERG };
#else
static char *lognams[] = { "debug", "info", "error", "panic" };
static FILE *errf;
#endif

static void
logger (int type, const char *fmt, va_list args)
{
#ifdef USE_SYSLOG
    int fl = 1;
#endif
    static char *pbuf;
    char *p, *cp, buf[1100];	/* bad bad */

#ifdef HAS_SNPRINTF
    vsnprintf (buf, sizeof(buf), fmt, args);
#else
    if (vsprintf (buf, fmt, args) >= sizeof(buf))
	Panic ("buffer overflow in logger()\n");
#endif

    for (cp = buf; (p = strchr (cp, '\n')); cp = p + 1) {
#ifdef USE_SYSLOG
	if (fl) {
	    fl = 0;
	    openlog(prog, LOG_PID, LOG_DAEMON);
	}
#endif
	if (pbuf) {
#ifdef USE_SYSLOG
	    syslog (lognums[type], "%s%.*s", pbuf, (int)(p - cp), cp);
#else
	    fprintf (errf ? errf : stderr, "%s[%d] %s: %s%.*s\n", 
		 prog, (int)getpid(), lognams[type], pbuf, (int)(p - cp), cp);
#endif
	    free (pbuf);
	    pbuf = NULL;
	} else
#ifdef USE_SYSLOG
	    syslog (lognums[type], "%.*s", (int)(p - cp), cp);
#else
	    fprintf (errf ? errf : stderr, "%s[%d] %s: %.*s\n", 
		     prog, (int)getpid(), lognams[type], (int)(p - cp), cp);
#endif
    }
    if (*cp)
	StrApp (&pbuf, cp);
}

/*VARARGS1*/
void
Debug (const char *fmt, ...)
{
    va_list args;

    if (debugLevel > 0)
    {
	va_start(args, fmt);
	logger (DM_DEBUG, fmt, args);
	va_end(args);
    }
}

/*VARARGS1*/
void 
LogInfo(const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    logger (DM_INFO, fmt, args);
    va_end(args);
}

/*VARARGS1*/
void
LogError (const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    logger (DM_ERR, fmt, args);
    va_end(args);
}

/*VARARGS1*/
void
LogPanic (const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    logger (DM_EMERG, fmt, args);
    va_end(args);
    exit (1);
}

void
LogOutOfMem (const char *fkt)
{
#ifdef USE_SYSLOG
    openlog(prog, LOG_PID, LOG_DAEMON);
    syslog (LOG_ALERT, "out of memory in %s()", fkt);
#else
    fprintf (errf ? errf : stderr, "%s[%d] %s: out of memory in %s()\n", 
	     prog, (int)getpid(), lognams[DM_ERR], fkt);
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
InitErrorLog ()
{
#ifdef USE_SYSLOG
    /* aw shit! PAM re-opens the log with "bad" values ...
     * so we have to re-open it on every message *grmph*
     */
#else
    if (errorLogFile[0]) {
	if (!(errf = fopen (errorLogFile, "a")))
	    LogError ("Cannot open errorLogFile %s\n", errorLogFile);
    }
#endif
}
