/*

Copyright 1988, 1998  The Open Group
Copyright 2002 Sun Microsystems, Inc.  All rights reserved.
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
 * socket.c - Support for BSD sockets
 */

#include "dm.h"

#if defined(XDMCP) && !defined(STREAMSCONN)

#include "dm_error.h"
#include "dm_socket.h"

#include <netdb.h>
#include <arpa/inet.h>

static int c_request_port;

static int
CreateListeningSocket( struct sockaddr *sock_addr, int salen )
{
	int fd;
	const char *addrstring = "unknown";
#if defined(IPv6) && defined(AF_INET6)
	char addrbuf[INET6_ADDRSTRLEN];
#endif

	if (!request_port)
		return -1;

	if (debugLevel & DEBUG_CORE) {
#if defined(IPv6) && defined(AF_INET6)
		void *ipaddr;
		if (sock_addr->sa_family == AF_INET6)
			ipaddr = & ((struct sockaddr_in6 *)sock_addr)->sin6_addr;
		else
			ipaddr = & ((struct sockaddr_in *)sock_addr)->sin_addr;
		addrstring =
			inet_ntop( sock_addr->sa_family, ipaddr, addrbuf, sizeof(addrbuf) );

#else
		addrstring = inet_ntoa( ((struct sockaddr_in *)sock_addr)->sin_addr );
#endif

		Debug( "creating socket to listen on port %d of address %s\n",
		       request_port, addrstring );
	}

	if ((fd = socket( sock_addr->sa_family, SOCK_DGRAM, 0 )) == -1) {
		LogError( "XDMCP socket creation failed, errno %d\n", errno );
		return -1;
	}

	if (bind( fd, sock_addr, salen ) == -1) {
		LogError( "error %d binding socket address %d\n", errno, request_port );
		close( fd );
		return -1;
	}

	RegisterCloseOnFork( fd );
	RegisterInput( fd );
	return fd;
}

struct socklist {
	struct socklist *next;
	struct socklist *mcastgroups;
	struct sockaddr *addr;
	int salen;
	int addrlen;
	int fd;
	int ref; /* referenced bit - see UpdateListenSockets */
};

static struct socklist *listensocks;

static void
DestroyListeningSocket( struct socklist *s )
{
	struct socklist *g, *n;

	if (s->fd >= 0) {
		CloseNClearCloseOnFork( s->fd );
		UnregisterInput( s->fd );
		s->fd = -1;
	}
	if (s->addr) {
		free( s->addr );
		s->addr = NULL;
	}
	for (g = s->mcastgroups; g; g = n) {
		n = g->next;
		if (g->addr)
			free( g->addr );
		free( g );
	}
	s->mcastgroups = NULL;
}

static struct socklist*
FindInList( struct socklist *list, ARRAY8Ptr addr )
{
	struct socklist *s;

	for (s = list; s; s = s->next) {
		if (s->addrlen == addr->length) {
			char *addrdata;

			switch (s->addr->sa_family) {
			case AF_INET:
				addrdata = (char *)
				  &(((struct sockaddr_in *)s->addr)->sin_addr.s_addr);
				break;
#if defined(IPv6) && defined(AF_INET6)
			case AF_INET6:
				addrdata = (char *)
				  &(((struct sockaddr_in6 *)s->addr)->sin6_addr.s6_addr);
				break;
#endif
			default:
				/* Unrecognized address family */
				continue;
			}
			if (!memcmp( addrdata, addr->data, addr->length ))
				return s;
		}
	}
	return NULL;
}

static struct socklist *
CreateSocklistEntry( ARRAY8Ptr addr )
{
	struct socklist *s;

	if (!(s = Calloc( 1, sizeof(struct socklist) )))
		return NULL;

	if (addr->length == 4) { /* IPv4 */
		struct sockaddr_in *sin4;
		sin4 = Calloc( 1, sizeof(struct sockaddr_in) );
#ifdef HAVE_STRUCT_SOCKADDR_IN_SIN_LEN
		sin4->sin_len = sizeof(struct sockaddr_in);
#endif
		s->addr = (struct sockaddr *)sin4;
		s->salen = sizeof(struct sockaddr_in);
		s->addrlen = sizeof(struct in_addr);
		sin4->sin_family = AF_INET;
		sin4->sin_port = htons( (short)request_port );
		memcpy( &sin4->sin_addr, addr->data, addr->length );
	}
#if defined(IPv6) && defined(AF_INET6)
	else if (addr->length == 16) { /* IPv6 */
		struct sockaddr_in6 *sin6;
		sin6 = Calloc( 1, sizeof(struct sockaddr_in6) );
#ifdef SIN6_LEN
		sin6->sin6_len = sizeof(struct sockaddr_in6);
#endif
		s->addr = (struct sockaddr *)sin6;
		s->salen = sizeof(struct sockaddr_in6);
		s->addrlen = sizeof(struct in6_addr);
		sin6->sin6_family = AF_INET6;
		sin6->sin6_port = htons( (short)request_port );
		memcpy( &sin6->sin6_addr, addr->data, addr->length );
	}
#endif
	else {
		/* Unknown address type */
		free( s );
		s = NULL;
	}
	return s;
}

static void
UpdateListener( ARRAY8Ptr addr, void **closure )
{
	struct socklist *s;

	*closure = NULL;

	if (addr == NULL) {
		ARRAY8 tmpaddr;
		struct in_addr in;
#if defined(IPv6) && defined(AF_INET6)
		struct in6_addr in6 = in6addr_any;
		tmpaddr.length = sizeof(in6);
		tmpaddr.data = (CARD8Ptr) &in6;
		UpdateListener( &tmpaddr, closure );
		if (*closure)
			return;
#endif
		in.s_addr = htonl( INADDR_ANY );
		tmpaddr.length = sizeof(in);
		tmpaddr.data = (CARD8Ptr) &in;
		UpdateListener( &tmpaddr, closure );
		return;
	}

	if (c_request_port == request_port &&
	    (s = FindInList( listensocks, addr )))
	{
		*closure = (void *)s;
		s->ref = 1;
		return;
	}

	if (!(s = CreateSocklistEntry( addr )))
		return;

	if ((s->fd = CreateListeningSocket( s->addr, s->salen )) < 0) {
		free( s->addr );
		free( s );
		return;
	}
	s->ref = 1;
	s->next = listensocks;
	listensocks = s;
	*closure = (void *)s;
}

#define JOIN_MCAST_GROUP 0
#define LEAVE_MCAST_GROUP 1

static void
ChangeMcastMembership( struct socklist *s, struct socklist *g, int op )
{
	int sockopt;

	switch (s->addr->sa_family)
	{
		case AF_INET:
		{
			struct ip_mreq mreq;
			memcpy( &mreq.imr_multiaddr,
			        &((struct sockaddr_in *)g->addr)->sin_addr,
			        sizeof(struct in_addr) );
			memcpy( &mreq.imr_interface,
			        &((struct sockaddr_in *)s->addr)->sin_addr,
			        sizeof(struct in_addr) );
			if (op == JOIN_MCAST_GROUP)
				sockopt = IP_ADD_MEMBERSHIP;
			else
				sockopt = IP_DROP_MEMBERSHIP;
			if (setsockopt( s->fd, IPPROTO_IP, sockopt,
			                &mreq, sizeof(mreq) ) < 0) {
				LogError( "XDMCP socket multicast %s to %s failed, errno %d\n",
				          (op == JOIN_MCAST_GROUP) ? "join" : "drop",
				          inet_ntoa( ((struct sockaddr_in *)g->addr)->sin_addr ),
				          errno );
			} else if (debugLevel & DEBUG_CORE) {
				Debug( "XDMCP socket multicast %s to %s succeeded\n",
				       (op == JOIN_MCAST_GROUP) ? "join" : "drop",
				       inet_ntoa( ((struct sockaddr_in *)g->addr)->sin_addr ) );
			}
			return;
		}
#if defined(IPv6) && defined(AF_INET6)
# ifndef IPV6_JOIN_GROUP
#  define IPV6_JOIN_GROUP IPV6_ADD_MEMBERSHIP
# endif
# ifndef IPV6_LEAVE_GROUP
#  define IPV6_LEAVE_GROUP IPV6_DROP_MEMBERSHIP
# endif
		case AF_INET6:
		{
			struct ipv6_mreq mreq6;
			memcpy( &mreq6.ipv6mr_multiaddr,
			        &((struct sockaddr_in6 *)g->addr)->sin6_addr,
			        sizeof(struct in6_addr) );
			mreq6.ipv6mr_interface = 0;  /* TODO: fix this */
			if (op == JOIN_MCAST_GROUP)
				sockopt = IPV6_JOIN_GROUP;
			else
				sockopt = IPV6_LEAVE_GROUP;
			if (setsockopt( s->fd, IPPROTO_IPV6, sockopt,
			                &mreq6, sizeof(mreq6) ) < 0)
			{
				int saveerr = errno;
				char addrbuf[INET6_ADDRSTRLEN];

				inet_ntop( s->addr->sa_family,
				           &((struct sockaddr_in6 *)g->addr)->sin6_addr,
				           addrbuf, sizeof(addrbuf) );

				LogError( "XDMCP socket multicast %s to %s failed, errno %d\n",
				          (op == JOIN_MCAST_GROUP) ? "join" : "drop", addrbuf,
				          saveerr );
			} else if (debugLevel & DEBUG_CORE) {
				char addrbuf[INET6_ADDRSTRLEN];

				inet_ntop( s->addr->sa_family,
				           &((struct sockaddr_in6 *)g->addr)->sin6_addr,
				           addrbuf, sizeof(addrbuf) );

				Debug( "XDMCP socket multicast %s to %s succeeded\n",
				       (op == JOIN_MCAST_GROUP) ? "join" : "drop", addrbuf );
			}
			return;
		}
#endif
	}
}

static void
UpdateMcastGroup( ARRAY8Ptr addr, void **closure )
{
	struct socklist *s = (struct socklist *)*closure;
	struct socklist *g;

	if (!s)
		return;

	/* Already in the group, mark & continue */
	if ((g = FindInList( s->mcastgroups, addr ))) {
		g->ref = 1;
		return;
	}

	/* Need to join the group */
	if (!(g = CreateSocklistEntry( addr )))
		return;

	ChangeMcastMembership( s, g, JOIN_MCAST_GROUP );
}

/* Open or close listening sockets to match the current settings read in
   from the access database. */
void
UpdateListenSockets( void )
{
	struct socklist *s, *g, **ls, **lg;
	void *tmpPtr = NULL;

	/* Clear Ref bits - any not marked by UpdateCallback will be closed */
	for (s = listensocks; s; s = s->next) {
		s->ref = 0;
		for (g = s->mcastgroups; g; g = g->next)
			g->ref = 0;
	}
	ForEachListenAddr( UpdateListener, UpdateMcastGroup, &tmpPtr );
	c_request_port = request_port;
	for (ls = &listensocks; (s = *ls); )
		if (!s->ref) {
			DestroyListeningSocket( s );
			*ls = s->next;
			free( s );
		} else {
			ls = &s->next;
			for (lg = &s->mcastgroups; (g = *lg); )
				if (!g->ref) {
					ChangeMcastMembership( s, g, LEAVE_MCAST_GROUP );
					*lg = g->next;
					free( g );
				} else
					lg = &g->next;
		}
}

int
AnyListenSockets( void )
{
	return listensocks != NULL;
}

int
ProcessListenSockets( FD_TYPE *reads )
{
	struct socklist *s;
	int ret = 0;

	for (s = listensocks; s; s = s->next)
		if (FD_ISSET( s->fd, reads )) {
			ProcessRequestSocket( s->fd );
			ret = 1;
		}
	return ret;
}

#endif /* !STREAMSCONN && XDMCP */
