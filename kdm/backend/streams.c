/*

Copyright 1988, 1998  The Open Group
Copyright 2002,2004 Oswald Buddenhagen <ossi@kde.org>

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
 * streams.c - Support for STREAMS
 */

#include "dm.h"

#if defined(XDMCP) && defined(STREAMSCONN)

#include "dm_error.h"

#include <fcntl.h>
#include <tiuser.h>
#include <netconfig.h>
#include <netdir.h>

static int xdmcpFd = -1, c_request_port;

void
UpdateListenSockets( void )
{
	struct t_bind bind_addr;
	struct netconfig *nconf;
	struct nd_hostserv service;
	struct nd_addrlist *servaddrs;
	char bindbuf[15];
	int it;

	if (c_request_port == request_port)
		return;
	c_request_port = request_port;

	if (xdmcpFd != -1) {
		CloseNClearCloseOnFork( xdmcpFd );
		UnregisterInput( xdmcpFd );
		xdmcpFd = -1;
	}

	if (!request_port)
		return;

	Debug( "creating UDP stream %d\n", request_port );

	nconf = getnetconfigent( "udp" );
	if (!nconf) {
		t_error( "getnetconfigent udp" );
		return;
	}

	xdmcpFd = t_open( nconf->nc_device, O_RDWR, NULL );
	if (xdmcpFd == -1) {
		LogError( "XDMCP stream creation failed\n" );
		t_error( "CreateWellKnownSockets(xdmcpFd): t_open failed" );
		return;
	}

	service.h_host = HOST_SELF;
	sprintf( bindbuf, "%d", request_port );
	service.h_serv = bindbuf;
	netdir_getbyname( nconf, &service, &servaddrs );
	freenetconfigent( nconf );

	bind_addr.qlen = 5;
	bind_addr.addr.buf = servaddrs->n_addrs[0].buf;
	bind_addr.addr.len = servaddrs->n_addrs[0].len;
	bind_addr.addr.maxlen = servaddrs->n_addrs[0].len;
	it = t_bind( xdmcpFd, &bind_addr, &bind_addr );
	netdir_free( (char *)servaddrs, ND_ADDRLIST );
	if (it < 0) {
		LogError( "Error binding UDP port %d\n", request_port );
		t_error( "CreateWellKnownSockets(xdmcpFd): t_bind failed" );
		t_close( xdmcpFd );
		xdmcpFd = -1;
		return;
	}
	RegisterCloseOnFork( xdmcpFd );
	RegisterInput( xdmcpFd );
}

int
AnyListenSockets( void )
{
	return xdmcpFd != -1;
}

int
ProcessRequestSockets( FD_TYPE *reads )
{
	if (xdmcpFd >= 0 && FD_ISSET( xdmcpFd, reads )) {
		ProcessRequestSocket( xdmcpFd );
		return 1;
	}
	return 0;
}

#endif /* STREAMSCONN && XDMCP */
