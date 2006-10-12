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
 * a simple linked list of known displays
 */

#include "dm.h"
#include "dm_error.h"

struct display *displays;
static struct disphist *disphist;

int
AnyDisplaysLeft( void )
{
	return displays != (struct display *)0;
}

int
AnyActiveDisplays( void )
{
	struct display *d;

	for (d = displays; d; d = d->next)
		if (d->status == remoteLogin || d->userSess >= 0)
			return 1;
	return 0;
}

int
AnyRunningDisplays( void )
{
	struct display *d;

	for (d = displays; d; d = d->next)
		switch (d->status) {
		case notRunning:
		case textMode:
		case reserve:
			break;
		default:
			return 1;
		}
	return 0;
}

int
AnyReserveDisplays( void )
{
	struct display *d;

	for (d = displays; d; d = d->next)
		if ((d->displayType & d_lifetime) == dReserve)
			return 1;
	return 0;
}

int
idleReserveDisplays( void )
{
	struct display *d;
	int cnt = 0;

	for (d = displays; d; d = d->next)
		if (d->status == reserve)
			cnt++;
	return cnt;
}

int
StartReserveDisplay( int lt )
{
	struct display *d, *rd;

	for (rd = 0, d = displays; d; d = d->next)
		if (d->status == reserve)
			rd = d;
	if (rd) {
		rd->idleTimeout = lt;
		rd->status = notRunning;
		return 1;
	}
	return 0;
}

void
ForEachDisplay( void (*f)( struct display * ) )
{
	struct display *d, *next;

	for (d = displays; d; d = next) {
		next = d->next;
		(*f)( d );
	}
}

#ifdef HAVE_VTS
static void
_forEachDisplayRev( struct display *d, void (*f)( struct display * ) )
{
	if (d) {
		if (d->next)
			_forEachDisplayRev( d->next, f );
		(*f)( d );
	}
}

void
ForEachDisplayRev( void (*f)( struct display * ) )
{
	_forEachDisplayRev( displays, f );
}
#endif

struct display *
FindDisplayByName( const char *name )
{
	struct display *d;

	for (d = displays; d; d = d->next)
		if (!strcmp( name, d->name ))
			return d;
	return 0;
}

struct display *
FindDisplayByPid( int pid )
{
	struct display *d;

	for (d = displays; d; d = d->next)
		if (pid == d->pid)
			return d;
	return 0;
}

struct display *
FindDisplayByServerPid( int serverPid )
{
	struct display *d;

	for (d = displays; d; d = d->next)
		if (serverPid == d->serverPid)
			return d;
	return 0;
}

#ifdef XDMCP

struct display *
FindDisplayBySessionID( CARD32 sessionID )
{
	struct display *d;

	for (d = displays; d; d = d->next)
		if (sessionID == d->sessionID)
			return d;
	return 0;
}

struct display *
FindDisplayByAddress( XdmcpNetaddr addr, int addrlen, CARD16 displayNumber )
{
	struct display *d;

	for (d = displays; d; d = d->next)
		if ((d->displayType & d_origin) == dFromXDMCP &&
		    d->displayNumber == displayNumber &&
		    addressEqual( (XdmcpNetaddr)d->from.data, d->from.length,
		                  addr, addrlen ))
			return d;
	return 0;
}

#endif /* XDMCP */

#define IfFree(x)  if (x) free( (char *)x )

void
RemoveDisplay( struct display *old )
{
	struct display *d, **dp;
	int i;

	for (dp = &displays; (d = *dp); dp = &(*dp)->next) {
		if (d == old) {
			Debug( "Removing display %s\n", d->name );
			*dp = d->next;
			IfFree( d->class2 );
			IfFree( d->cfg.data );
			delStr( d->cfg.dep.name );
#ifdef XDMCP
			IfFree( d->remoteHost );
#endif
			if (d->authorizations) {
				for (i = 0; i < d->authNum; i++)
					XauDisposeAuth( d->authorizations[i] );
				free( (char *)d->authorizations );
			}
			if (d->authFile) {
				(void)unlink( d->authFile );
				free( d->authFile );
			}
			IfFree( d->authNameLens );
#ifdef XDMCP
			XdmcpDisposeARRAY8( &d->peer );
			XdmcpDisposeARRAY8( &d->from );
			XdmcpDisposeARRAY8( &d->clientAddr );
#endif
			free( (char *)d );
			break;
		}
	}
}

static struct disphist *
FindHist( const char *name )
{
	struct disphist *hstent;

	for (hstent = disphist; hstent; hstent = hstent->next)
		if (!strcmp( hstent->name, name ))
			return hstent;
	return 0;
}

struct display *
NewDisplay( const char *name )
{
	struct display *d;
	struct disphist *hstent;

	if (!(hstent = FindHist( name ))) {
		if (!(hstent = Calloc( 1, sizeof(*hstent) )))
			return 0;
		if (!StrDup( &hstent->name, name )) {
			free( hstent );
			return 0;
		}
		hstent->next = disphist; disphist = hstent;
	}

	if (!(d = (struct display *)Calloc( 1, sizeof(*d) )))
		return 0;
	d->next = displays;
	d->hstent = hstent;
	d->name = hstent->name;
	/* initialize fields (others are 0) */
	d->pid = -1;
	d->serverPid = -1;
	d->ctrl.fd = -1;
	d->pipe.rfd = -1;
	d->pipe.wfd = -1;
	d->gpipe.rfd = -1;
	d->gpipe.wfd = -1;
	d->userSess = -1;
#ifdef XDMCP
	d->xdmcpFd = -1;
#endif
	displays = d;
	Debug( "created new display %s\n", d->name );
	return d;
}
