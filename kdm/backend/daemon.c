/* $TOG: daemon.c /main/17 1998/02/09 13:54:47 kaleb $ */
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
/* $XFree86: xc/programs/xdm/daemon.c,v 3.11 2000/08/10 17:40:41 dawes Exp $ */

/*
 * xdm - display manager daemon
 * Author:  Keith Packard, MIT X Consortium
 */

#include "dm.h"
#include "dm_error.h"

#include <X11/Xos.h>

#if defined(X_NOT_POSIX) && !defined(CSRG_BASED) && !defined(SVR4) && !defined(__GLIBC__)
# ifdef USG
#  include <termios.h>
# else
#  include <sys/ioctl.h>
# endif
# ifdef hpux
#  include <sys/ptyio.h>
#  ifndef TIOCNOTTY
#   define TIOCNOTTY _IO('t', 113)           /* void tty association */
#  endif
#endif
#endif

#include <sys/types.h>
#ifdef X_NOT_POSIX
# define Pid_t int
#else
# define Pid_t pid_t
#endif

void
BecomeDaemon (void)
{
    int pfd[2];

    /*
     * fork so that the process goes into the background automatically. Also
     * has a nice side effect of having the child process get inherited by
     * init (pid 1).
     * Create a pipe and block on it, so the parent knows when the child is
     * done with detaching. This eliminates the possibility that the child
     * might get killed when the init script that's running xdm exits.
     */

    if (pipe (pfd))
	pfd[0] = pfd[1] = -1; /* so what ...? */
    switch (fork ()) {
    case 0:
	/* child */
	break;
    case -1:
	/* error */
	LogError("Daemon fork failed: %m\n");
	break;

    default:
	/* parent */
	close(pfd[1]);
	read(pfd[0], &pfd[1] /* dummy */, 1);
	exit (0);
    }

    /* don't use daemon() - it doesn't buy us anything but an additional fork */

#if !defined(X_NOT_POSIX) || defined(SVR4)
    setsid ();
#elif defined(SYSV) || defined(__QNXNTO__)
    setpgrp ();
#else
# if defined(__osf__) || defined(linux) || defined(__GNU__) || defined(__CYGWIN__)
    setpgid (0, 0);
# else /* BSD */
    setpgrp (0, 0);
# endif

    /*
     * Get rid of controlling tty
     */
# if !defined(__UNIXOS2__) && !defined(__CYGWIN__)
#  if !((defined(SYSV) || defined(SVR4)) && defined(i386))
  {
    register int i;
    if ((i = open ("/dev/tty", O_RDWR)) >= 0) {	/* did open succeed? */
#   if defined(USG) && defined(TCCLRCTTY)
	int zero = 0;
	(void) ioctl (i, TCCLRCTTY, &zero);
#   else
#    if (defined(SYSV) || defined(SVR4)) && defined(TIOCTTY)
	int zero = 0;
	(void) ioctl (i, TIOCTTY, &zero);
#    else
	(void) ioctl (i, TIOCNOTTY, (char *) 0);    /* detach, BSD style */
#    endif
#   endif
	(void) close (i);
    }
  }
#  endif /* !((SYSV || SVR4) && i386) */
# endif /* !__UNIXOS2__ && !__CYGWIN__ */
#endif /* !X_NOT_POSIX || SVR4 || SYSV || __QNXNTO_ */

    close(pfd[0]);
    close(pfd[1]); /* tell parent that we're done with detaching */

    chdir ("/");
}
