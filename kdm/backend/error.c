/*

Copyright 1988, 1998  The Open Group
Copyright 2000-2004 Oswald Buddenhagen <ossi@kde.org>

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
 * Log display manager errors to a file as
 * we generally do not have a terminal to talk to
 * or use syslog if it exists
 */

#include "dm.h"
#include "dm_error.h"
#include <config-kdm.h>
#include <unistd.h>
#include <stdio.h>

#define PRINT_QUOTES
#define PRINT_ARRAYS
#define LOG_DEBUG_MASK DEBUG_CORE
#define LOG_PANIC_EXIT 1
#define NEED_ASPRINTF
#define STATIC
#include "printf.c"

void
GDebug( const char *fmt, ... )
{
	va_list args;

	if (debugLevel & DEBUG_HLPCON) {
		va_start( args, fmt );
		Logger( DM_DEBUG, fmt, args );
		va_end( args );
	}
}

void
Panic( const char *mesg )
{
	int fd = open( "/dev/console", O_WRONLY );
	write( fd, "xdm panic: ", 11 );
	write( fd, mesg, strlen( mesg ) );
	write( fd, "\n", 1 );
#ifdef USE_SYSLOG
	ReInitErrorLog();
	syslog( LOG_ALERT, "%s", mesg );
#endif
	exit( 1 );
}

#ifdef USE_SYSLOG
void
ReInitErrorLog()
{
	if (!(debugLevel & DEBUG_NOSYSLOG))
		InitLog();
}
#endif

void
InitErrorLog( const char *errorLogFile )
{
	int fd;
	char buf[128];

#ifdef USE_SYSLOG
	ReInitErrorLog();
#endif
	/* We do this independently of using syslog, as we cannot redirect
	 * the output of external programs to syslog.
	 */
	if (!errorLogFile || strcmp( errorLogFile, "-" )) {
		if (!errorLogFile) {
			sprintf( buf, "/var/log/%s.log", prog );
			errorLogFile = buf;
		}
		if ((fd = open( errorLogFile, O_CREAT | O_APPEND | O_WRONLY, 0666 )) < 0)
			LogError( "Cannot open log file %s\n", errorLogFile );
		else {
#ifdef USE_SYSLOG
# ifdef USE_PAM
#  define PAMLOG " PAM logs messages related to authentication to authpriv.*."
# else
#  define PAMLOG
# endif
# define WARNMSG \
  "********************************************************************************\n" \
  "Note that your system uses syslog. All of kdm's internally generated messages\n" \
  "(i.e., not from libraries and external programs/scripts it uses) go to the\n" \
  "daemon.* syslog facility; check your syslog configuration to find out to which\n" \
  "file(s) it is logged." PAMLOG "\n" \
  "********************************************************************************\n\n"
			if (!lseek( fd, 0, SEEK_END ))
				write( fd, WARNMSG, sizeof(WARNMSG) - 1 );
#endif
			dup2( fd, 1 );
			close( fd );
			dup2( 1, 2 );
		}
	}
}

