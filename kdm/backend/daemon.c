/*

Copyright 1988, 1998  The Open Group
Copyright 2000,2001,2003 Oswald Buddenhagen <ossi@kde.org>

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
 */

#include "dm.h"
#include "dm_error.h"

void
BecomeDaemon( void )
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

	if (pipe( pfd ))
		pfd[0] = pfd[1] = -1; /* so what ...? */
	switch (fork ()) {
	case 0:
		/* child */
		break;
	case -1:
		/* error */
		LogError( "Daemon fork failed: %m\n" );
		break;

	default:
		/* parent */
		close( pfd[1] );
		read( pfd[0], &pfd[1] /* dummy */, 1 );
		exit( 0 );
	}

	/* don't use daemon() - it doesn't buy us anything but an additional fork */

	setsid();

	close( pfd[0] );
	close( pfd[1] ); /* tell parent that we're done with detaching */

	chdir( "/" );
}
