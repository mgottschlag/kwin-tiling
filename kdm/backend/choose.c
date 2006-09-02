/*

Copyright 1990, 1998  The Open Group
Copyright 2002-2004 Oswald Buddenhagen <ossi@kde.org>

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
 * chooser backend
 */

#include "dm.h"
#include "dm_error.h"
#include "dm_socket.h"

#include <ctype.h>

#ifdef __svr4__
# include <sys/sockio.h>
#endif
#include <arpa/inet.h>

#include <sys/ioctl.h>
#ifdef STREAMSCONN
# ifdef WINTCP /* NCR with Wollongong TCP */
#  include <netinet/ip.h>
# endif
# include <stropts.h>
# include <tiuser.h>
# include <netconfig.h>
# include <netdir.h>
#endif

#if !defined(__GNU__) && !defined(__hpux) /* XXX __hpux might be wrong */
# include <net/if.h>
#endif

#include <netdb.h>

typedef struct _IndirectUsers {
	struct _IndirectUsers *next;
	ARRAY8 client;
	CARD16 connectionType;
} IndirectUsersRec, *IndirectUsersPtr;

static IndirectUsersPtr indirectUsers;

int
RememberIndirectClient( ARRAY8Ptr clientAddress, CARD16 connectionType )
{
	IndirectUsersPtr i;

	for (i = indirectUsers; i; i = i->next)
		if (XdmcpARRAY8Equal( clientAddress, &i->client ) &&
		    connectionType == i->connectionType)
			return 1;
	i = (IndirectUsersPtr)Malloc( sizeof(IndirectUsersRec) );
	if (!i) {
		return 0;
	}
	if (!XdmcpCopyARRAY8( clientAddress, &i->client )) {
		free( (char *)i );
		return 0;
	}
	i->connectionType = connectionType;
	i->next = indirectUsers;
	indirectUsers = i;
	return 1;
}

void
ForgetIndirectClient( ARRAY8Ptr clientAddress, CARD16 connectionType )
{
	IndirectUsersPtr *i, ni;

	for (i = &indirectUsers; *i; i = &(*i)->next)
		if (XdmcpARRAY8Equal( clientAddress, &(*i)->client ) &&
		    connectionType == (*i)->connectionType)
		{
			ni = (*i)->next;
			XdmcpDisposeARRAY8( &(*i)->client );
			free( (char *)(*i) );
			(*i) = ni;
			break;
		}
}

int
IsIndirectClient( ARRAY8Ptr clientAddress, CARD16 connectionType )
{
	IndirectUsersPtr i;

	for (i = indirectUsers; i; i = i->next)
		if (XdmcpARRAY8Equal( clientAddress, &i->client ) &&
		    connectionType == i->connectionType)
			return 1;
	return 0;
}

typedef struct _Choices {
	struct _Choices *next;
	ARRAY8 client;
	CARD16 connectionType;
	ARRAY8 choice;
	Time_t time;
} ChoiceRec, *ChoicePtr;

static ChoicePtr choices;

ARRAY8Ptr
IndirectChoice( ARRAY8Ptr clientAddress, CARD16 connectionType )
{
	ChoicePtr c, next, prev;

	prev = 0;
	for (c = choices; c; c = next) {
		next = c->next;
		Debug( "choice checking timeout: %ld >? %d\n",
		       (long)(now - c->time), choiceTimeout );
		if (now - c->time > (Time_t)choiceTimeout) {
			Debug( "timeout choice %ld > %d\n",
			       (long)(now - c->time), choiceTimeout );
			if (prev)
				prev->next = next;
			else
				choices = next;
			XdmcpDisposeARRAY8( &c->client );
			XdmcpDisposeARRAY8( &c->choice );
			free( (char *)c );
		} else {
			if (XdmcpARRAY8Equal( clientAddress, &c->client ) &&
			    connectionType == c->connectionType)
				return &c->choice;
			prev = c;
		}
	}
	return 0;
}

int
RegisterIndirectChoice( ARRAY8Ptr clientAddress, CARD16 connectionType,
                        ARRAY8Ptr choice )
{
	ChoicePtr c;
	int insert;
#if 0
	int found = 0;
#endif

	Debug( "got indirect choice back\n" );
	for (c = choices; c; c = c->next) {
		if (XdmcpARRAY8Equal( clientAddress, &c->client ) &&
		    connectionType == c->connectionType) {
#if 0
			found = 1;
#endif
			break;
		}
	}
#if 0
	if (!found)
		return 0;
#endif

	insert = 0;
	if (!c) {
		insert = 1;
		c = (ChoicePtr)Malloc( sizeof(ChoiceRec) );
		if (!c)
			return 0;
		c->connectionType = connectionType;
		if (!XdmcpCopyARRAY8( clientAddress, &c->client )) {
			free( (char *)c );
			return 0;
		}
	} else
		XdmcpDisposeARRAY8( &c->choice );
	if (!XdmcpCopyARRAY8( choice, &c->choice )) {
		XdmcpDisposeARRAY8( &c->client );
		free( (char *)c );
		return 0;
	}
	if (insert) {
		c->next = choices;
		choices = c;
	}
	c->time = now;
	return 1;
}

#if 0
static void
RemoveIndirectChoice( ARRAY8Ptr clientAddress, CARD16 connectionType )
{
	ChoicePtr c, prev;

	prev = 0;
	for (c = choices; c; c = c->next) {
		if (XdmcpARRAY8Equal( clientAddress, &c->client ) &&
		    connectionType == c->connectionType)
		{
			if (prev)
				prev->next = c->next;
			else
				choices = c->next;
			XdmcpDisposeARRAY8( &c->client );
			XdmcpDisposeARRAY8( &c->choice );
			free( (char *)c );
			return;
		}
		prev = c;
	}
}
#endif


/* ####################### */


typedef struct _hostAddr {
	struct _hostAddr *next;
	struct sockaddr *addr;
	int addrlen;
	xdmOpCode type;
} HostAddr;

static HostAddr *hostAddrdb;

typedef struct _hostName {
	struct _hostName *next;
	unsigned willing:1, alive:1;
	ARRAY8 hostname, status;
	CARD16 connectionType;
	ARRAY8 hostaddr;
} HostName;

static HostName *hostNamedb;

static XdmcpBuffer directBuffer, broadcastBuffer;
static XdmcpBuffer buffer;

static int socketFD;
#if defined(IPv6) && defined(AF_INET6)
static int socket6FD;
#endif


static void
doPingHosts()
{
	HostAddr *hosts;

	for (hosts = hostAddrdb; hosts; hosts = hosts->next)
#if defined(IPv6) && defined(AF_INET6)
		XdmcpFlush( hosts->addr->sa_family == AF_INET6 ? socket6FD : socketFD,
#else
		XdmcpFlush( socketFD,
#endif
		            hosts->type == QUERY ? &directBuffer : &broadcastBuffer,
		            (XdmcpNetaddr)hosts->addr, hosts->addrlen );
}

static int
addHostname( ARRAY8Ptr hostname, ARRAY8Ptr status,
             struct sockaddr *addr, int will )
{
	HostName **names, *name;
	ARRAY8 hostAddr;
	CARD16 connectionType;

	switch (addr->sa_family) {
		case AF_INET:
			hostAddr.data =
				(CARD8 *)& ((struct sockaddr_in *)addr)->sin_addr;
			hostAddr.length = 4;
			connectionType = FamilyInternet;
			break;
#if defined(IPv6) && defined(AF_INET6)
		case AF_INET6:
			hostAddr.data =
				(CARD8 *)&((struct sockaddr_in6 *)addr)->sin6_addr;
			hostAddr.length = 16;
			connectionType = FamilyInternet6;
			break;
#endif
		default:
			hostAddr.data = (CARD8 *)"";
			hostAddr.length = 0;
			connectionType = FamilyLocal;
			break;
	}
	for (names = &hostNamedb; *names; names = &(*names)->next) {
		name = *names;
		if (connectionType == name->connectionType &&
		    XdmcpARRAY8Equal( &hostAddr, &name->hostaddr ))
		{
			if (XdmcpARRAY8Equal( status, &name->status ))
				return 0;
			XdmcpDisposeARRAY8( &name->status );
			XdmcpDisposeARRAY8( hostname );

			GSendInt( G_Ch_ChangeHost );
			goto gotold;
		}
	}
	if (!(name = (HostName *)Malloc( sizeof(*name) )))
		return 0;
	if (hostname->length) {
		switch (addr->sa_family) {
		case AF_INET:
#if defined(IPv6) && defined(AF_INET6)
		case AF_INET6:
#endif
			{
				struct hostent *hostent;
				char *host;

				hostent = gethostbyaddr( (char *)hostAddr.data,
				                         hostAddr.length, addr->sa_family );
				if (hostent) {
					XdmcpDisposeARRAY8( hostname );
					host = hostent->h_name;
					XdmcpAllocARRAY8( hostname, strlen( host ) );
					memmove( hostname->data, host, hostname->length );
				}
			}
		}
	}
	if (!XdmcpAllocARRAY8( &name->hostaddr, hostAddr.length )) {
		free( (char *)name );
		return 0;
	}
	memmove( name->hostaddr.data, hostAddr.data, hostAddr.length );
	name->connectionType = connectionType;
	name->hostname = *hostname;

	*names = name;
	name->next = 0;

	GSendInt( G_Ch_AddHost );
  gotold:
	name->alive = 1;
	name->willing = will;
	name->status = *status;

	GSendInt( (int)name ); /* just an id */
	GSendNStr( (char *)name->hostname.data, name->hostname.length );
	GSendNStr( (char *)name->status.data, name->status.length );
	GSendInt( will );

	return 1;
}

static void
disposeHostname( HostName *host )
{
	XdmcpDisposeARRAY8( &host->hostname );
	XdmcpDisposeARRAY8( &host->hostaddr );
	XdmcpDisposeARRAY8( &host->status );
	free( (char *)host );
}

static void
emptyHostnames( void )
{
	HostName *host, *nhost;

	for (host = hostNamedb; host; host = nhost) {
		nhost = host->next;
		disposeHostname( host );
	}
	hostNamedb = 0;
}

static void
receivePacket( int sfd )
{
	XdmcpHeader header;
	ARRAY8 authenticationName;
	ARRAY8 hostname;
	ARRAY8 status;
	int saveHostname = 0;
#if defined(IPv6) && defined(AF_INET6)
	struct sockaddr_storage addr;
#else
	struct sockaddr addr;
#endif
	int addrlen;

	addrlen = sizeof(addr);
	if (!XdmcpFill( sfd, &buffer, (XdmcpNetaddr)&addr, &addrlen ))
		return;
	if (!XdmcpReadHeader( &buffer, &header ))
		return;
	if (header.version != XDM_PROTOCOL_VERSION)
		return;
	hostname.data = 0;
	status.data = 0;
	authenticationName.data = 0;
	switch (header.opcode) {
		case WILLING:
			if (XdmcpReadARRAY8( &buffer, &authenticationName ) &&
			    XdmcpReadARRAY8( &buffer, &hostname ) &&
			    XdmcpReadARRAY8( &buffer, &status )) {
				if (header.length == 6 + authenticationName.length +
				    hostname.length + status.length) {
					if (addHostname( &hostname, &status,
					                 (struct sockaddr *)&addr, 1 ))
						saveHostname = 1;
				}
			}
			XdmcpDisposeARRAY8( &authenticationName );
			break;
		case UNWILLING:
			if (XdmcpReadARRAY8( &buffer, &hostname ) &&
			    XdmcpReadARRAY8( &buffer, &status )) {
				if (header.length == 4 + hostname.length + status.length) {
					if (addHostname( &hostname, &status,
					                 (struct sockaddr *)&addr, 0 ))
						saveHostname = 1;
				}
			}
			break;
		default:
			break;
	}
	if (!saveHostname) {
		XdmcpDisposeARRAY8( &hostname );
		XdmcpDisposeARRAY8( &status );
	}
}

static void
addHostaddr( HostAddr **hosts, struct sockaddr *addr, int len, xdmOpCode type )
{
	HostAddr *host;

	Debug( "adding host %[*hhu, type %d\n", len, addr, type );
	for (host = *hosts; host; host = host->next)
		if (host->type == type && host->addr->sa_family == addr->sa_family)
			switch (addr->sa_family) {
			case AF_INET:
				{
				struct sockaddr_in *na = (struct sockaddr_in *)addr;
				struct sockaddr_in *oa = (struct sockaddr_in *)host->addr;
				if (na->sin_port == oa->sin_port &&
				    na->sin_addr.s_addr == oa->sin_addr.s_addr)
					return;
				break;
				}
#if defined(IPv6) && defined(AF_INET6)
			case AF_INET6:
				{
				struct sockaddr_in6 *na = (struct sockaddr_in6 *)addr;
				struct sockaddr_in6 *oa = (struct sockaddr_in6 *)host->addr;
				if (na->sin6_port == oa->sin6_port &&
				    !memcmp( &na->sin6_addr, &oa->sin6_addr, 16 ))
					return;
				break;
				}
#endif
			default: /* ... */
				break;
			}
	Debug( " not dupe\n" );
	if (!(host = (HostAddr *)Malloc( sizeof(*host) )))
		return;
	if (!(host->addr = (struct sockaddr *)Malloc( len ))) {
		free( (char *)host );
		return;
	}
	memcpy( (char *)host->addr, (char *)addr, len );
	host->addrlen = len;
	host->type = type;
	host->next = *hosts;
	*hosts = host;
}

static void
registerHostaddr( struct sockaddr *addr, int len, xdmOpCode type )
{
	addHostaddr( &hostAddrdb, addr, len, type );
}

static void
emptyPingHosts( void )
{
	HostAddr *host, *nhost;

	for (host = hostAddrdb; host; host = nhost) {
		nhost = host->next;
		free( host->addr );
		free( host );
	}
	hostAddrdb = 0;
}

/* Handle variable length ifreq in BNR2 and later */
#ifdef VARIABLE_IFREQ
# define ifr_size(p) \
	(sizeof(struct ifreq) + \
	 (p->ifr_addr.sa_len > sizeof (p->ifr_addr) ? \
	  p->ifr_addr.sa_len - sizeof (p->ifr_addr) : 0))
#else
# define ifr_size(p) (sizeof(struct ifreq))
#endif

#define IFC_REQ(ifc) ifc.ifc_req

#ifndef SYSV_SIOCGIFCONF
# define ifioctl ioctl
#endif

static void
registerBroadcastForPing( void )
{
	struct sockaddr_in in_addr;

#ifdef __GNU__
	in_addr.sin_addr.s_addr = htonl( 0xFFFFFFFF );
	in_addr.sin_port = htons( XDM_UDP_PORT );
	registerHostaddr( (struct sockaddr *)&in_addr, sizeof(in_addr),
	                  BROADCAST_QUERY );
#else /* __GNU__ */
	struct ifconf ifc;
	register struct ifreq *ifr;
	struct sockaddr broad_addr;
	char buf[2048], *cp, *cplim;
# ifdef WINTCP /* NCR with Wollongong TCP */
		int ipfd;
		struct ifconf *ifcp;
		struct strioctl ioc;
		int n;

		ifcp = (struct ifconf *)buf;
		ifcp->ifc_buf = buf + 4;
		ifcp->ifc_len = sizeof(buf) - 4;

		if ((ipfd = open( "/dev/ip", O_RDONLY )) < 0) {
			t_error( "RegisterBroadcastForPing() t_open(/dev/ip) failed" );
			return;
		}

		ioc.ic_cmd = IPIOC_GETIFCONF;
		ioc.ic_timout = 60;
		ioc.ic_len = sizeof(buf);
		ioc.ic_dp = (char *)ifcp;

		if (ioctl( ipfd, (int)I_STR, (char *)&ioc ) < 0) {
			perror( "RegisterBroadcastForPing() ioctl(I_STR(IPIOC_GETIFCONF)) failed" );
			close( ipfd );
			return;
		}

		for (ifr = ifcp->ifc_req, n = ifcp->ifc_len / sizeof(struct ifreq);
		     --n >= 0; ifr++)
# else /* WINTCP */
		ifc.ifc_len = sizeof(buf);
		ifc.ifc_buf = buf;
		if (ifioctl(socketFD, (int)SIOCGIFCONF, (char *)&ifc) < 0)
			return;

		cplim = (char *)IFC_REQ( ifc ) + ifc.ifc_len;

		for (cp = (char *)IFC_REQ( ifc ); cp < cplim; cp += ifr_size(ifr))
# endif /* WINTCP */
		{
# ifndef WINTCP
			ifr = (struct ifreq *)cp;
# endif
			if (ifr->ifr_addr.sa_family != AF_INET)
				continue;

			broad_addr = ifr->ifr_addr;
			((struct sockaddr_in *)&broad_addr)->sin_addr.s_addr =
				htonl( INADDR_BROADCAST );
# ifdef SIOCGIFBRDADDR
			{
				struct ifreq broad_req;

				broad_req = *ifr;
#  ifdef WINTCP /* NCR with Wollongong TCP */
				ioc.ic_cmd = IPIOC_GETIFFLAGS;
				ioc.ic_timout = 0;
				ioc.ic_len = sizeof(broad_req);
				ioc.ic_dp = (char *)&broad_req;

				if (ioctl( ipfd, I_STR, (char *)&ioc )
#  else /* WINTCP */
				if (ifioctl( socketFD, SIOCGIFFLAGS, (char *)&broad_req )
#  endif /* WINTCP */
				      != -1 &&
				    (broad_req.ifr_flags & IFF_BROADCAST) &&
				    (broad_req.ifr_flags & IFF_UP))
				{
					broad_req = *ifr;
#  ifdef WINTCP /* NCR with Wollongong TCP */
					ioc.ic_cmd = IPIOC_GETIFBRDADDR;
					ioc.ic_timout = 0;
					ioc.ic_len = sizeof(broad_req);
					ioc.ic_dp = (char *)&broad_req;

					if (ioctl( ipfd, I_STR, (char *)&ioc )
#  else /* WINTCP */
					if (ifioctl( socketFD, SIOCGIFBRDADDR, (char *)&broad_req )
#  endif /* WINTCP */
					      != -1)
						broad_addr = broad_req.ifr_addr;
					else
						continue;
				} else
					continue;
			}
# endif
			in_addr = *((struct sockaddr_in *)&broad_addr);
			in_addr.sin_port = htons( XDM_UDP_PORT );
# ifdef HAVE_STRUCT_SOCKADDR_IN_SIN_LEN
			in_addr.sin_len = sizeof(in_addr);
# endif
			registerHostaddr( (struct sockaddr *)&in_addr, sizeof(in_addr),
			                  BROADCAST_QUERY );
		}
#endif
}

static int
makeSockAddrs( const char *name, HostAddr **hosts )
{
#if defined(IPv6) && defined(AF_INET6)
	struct addrinfo *ai, *nai, hints;
	bzero( &hints, sizeof(hints) );
	hints.ai_socktype = SOCK_DGRAM;
	if (getaddrinfo( name, stringify( XDM_UDP_PORT ), &hints, &ai ))
		return 0;
	for (nai = ai; nai; nai = nai->ai_next)
		if ((nai->ai_family == AF_INET) || (nai->ai_family == AF_INET6))
			addHostaddr( hosts, nai->ai_addr, nai->ai_addrlen,
			             (nai->ai_family == AF_INET ?
			              IN_MULTICAST( ((struct sockaddr_in *)nai->ai_addr)->sin_addr.s_addr ) :
			              IN6_IS_ADDR_MULTICAST( &((struct sockaddr_in6 *)nai->ai_addr)->sin6_addr )) ?
			             BROADCAST_QUERY : QUERY );
#else
	struct sockaddr_in in_addr;
	/* Per RFC 1123, check first for IP address in dotted-decimal form */
	if ((in_addr.sin_addr.s_addr = inet_addr( name )) == (unsigned)-1) {
		struct hostent *hostent;
		if (!(hostent = gethostbyname( name )) ||
		    hostent->h_addrtype != AF_INET)
			return 0;
		memcpy( &in_addr.sin_addr, hostent->h_addr, 4 );
	}
	in_addr.sin_family = AF_INET;
	in_addr.sin_port = htons( XDM_UDP_PORT );
# ifdef HAVE_STRUCT_SOCKADDR_IN_SIN_LEN
	in_addr.sin_len = sizeof(in_addr);
# endif
	addHostaddr( hosts, (struct sockaddr *)&in_addr, sizeof(in_addr),
# ifdef IN_MULTICAST
	             IN_MULTICAST( in_addr.sin_addr.s_addr ) ?
	             BROADCAST_QUERY :
# endif
	             QUERY );
#endif
	return 1;
}

/*
 * Register the address for this host.
 * Called for interactively specified hosts.
 * The special names "BROADCAST" and "*" look up all the broadcast
 *  addresses on the local host.
 */

static int
registerForPing( const char *name )
{
	Debug( "manual host registration: %s\n", name );
	if (!strcmp( name, "BROADCAST" ) || !strcmp( name, "*" ))
		registerBroadcastForPing();
	else if (!makeSockAddrs( name, &hostAddrdb ))
		return 0;
	return 1;
}

/*ARGSUSED*/
static void
AddChooserHost(
               CARD16 connectionType,
               ARRAY8Ptr addr,
               char *closure ATTR_UNUSED )
{
	if (connectionType == FamilyBroadcast) {
		registerBroadcastForPing();
		return;
	}
	Debug( "internal host registration: %[*hhu, family %hx\n",
	       addr->length, addr->data, connectionType );
	if (connectionType == FamilyInternet) {
		struct sockaddr_in in_addr;
		in_addr.sin_family = AF_INET;
		memmove( &in_addr.sin_addr, addr->data, 4 );
		in_addr.sin_port = htons( XDM_UDP_PORT );
#ifdef HAVE_STRUCT_SOCKADDR_IN_SIN_LEN
		in_addr.sin_len = sizeof(in_addr);
#endif
		registerHostaddr( (struct sockaddr *)&in_addr, sizeof(in_addr),
#ifdef IN_MULTICAST
		                  IN_MULTICAST( in_addr.sin_addr.s_addr ) ?
		                  BROADCAST_QUERY :
#endif
		                  QUERY );
	}
#if defined(IPv6) && defined(AF_INET6)
	else if (connectionType == FamilyInternet6) {
		struct sockaddr_in6 in6_addr;
		in6_addr.sin6_family = AF_INET6;
		memmove( &in6_addr.sin6_addr, addr->data, 16 );
		in6_addr.sin6_port = htons( XDM_UDP_PORT );
#ifdef HAVE_STRUCT_SOCKADDR_IN6_SIN6_LEN
		in6_addr.sin6_len = sizeof(in6_addr);
#endif
		registerHostaddr( (struct sockaddr *)&in6_addr, sizeof(in6_addr),
		                  IN6_IS_ADDR_MULTICAST( &in6_addr.sin6_addr ) ?
		                  BROADCAST_QUERY : QUERY );
	}
#endif
}

static ARRAYofARRAY8 AuthenticationNames;

#if 0
static void RegisterAuthenticationName( char *name, int namelen )
{
	ARRAY8Ptr authName;
	if (!XdmcpReallocARRAYofARRAY8( &AuthenticationNames,
	                                AuthenticationNames.length + 1 ))
		return;
	authName = &AuthenticationNames.data[AuthenticationNames.length - 1];
	if (!XdmcpAllocARRAY8( authName, namelen ))
		return;
	memmove( authName->data, name, namelen );
}
#endif

static int
initXDMCP()
{
	XdmcpHeader header;
#if 0
	int i;
#endif
#ifndef STREAMSCONN
#ifdef SO_BROADCAST
	int soopts;
#endif
#endif

	header.version = XDM_PROTOCOL_VERSION;
	header.length = 1;
#if 0
	for (i = 0; i < (int)AuthenticationNames.length; i++)
		header.length += 2 + AuthenticationNames.data[i].length;
#endif

	header.opcode = (CARD16)BROADCAST_QUERY;
	XdmcpWriteHeader( &broadcastBuffer, &header );
	XdmcpWriteARRAYofARRAY8( &broadcastBuffer, &AuthenticationNames );

	header.opcode = (CARD16)QUERY;
	XdmcpWriteHeader( &directBuffer, &header );
	XdmcpWriteARRAYofARRAY8( &directBuffer, &AuthenticationNames );

#if defined(STREAMSCONN)
	if ((socketFD = t_open( "/dev/udp", O_RDWR, 0 )) < 0)
		return 0;

	if (t_bind( socketFD, NULL, NULL ) < 0) {
		t_close( socketFD );
		return 0;
	}

	/*
	 * This part of the code looks contrived. It will actually fit in nicely
	 * when the CLTS part of Xtrans is implemented.
	 */
	{
		struct netconfig *nconf;

		if ((nconf = getnetconfigent( "udp" )) == NULL) {
			t_unbind( socketFD );
			t_close( socketFD );
			return 0;
		}

		if (netdir_options( nconf, ND_SET_BROADCAST, socketFD, NULL )) {
			freenetconfigent( nconf );
			t_unbind( socketFD );
			t_close( socketFD );
			return 0;
		}

		freenetconfigent( nconf );
	}
#else
	if ((socketFD = socket( AF_INET, SOCK_DGRAM, 0 )) < 0)
		return 0;
#if defined(IPv6) && defined(AF_INET6)
	socket6FD = socket( AF_INET6, SOCK_DGRAM, 0 );
#endif
# ifdef SO_BROADCAST
	soopts = 1;
	if (setsockopt( socketFD, SOL_SOCKET, SO_BROADCAST, (char *)&soopts,
	                sizeof(soopts) ) < 0)
		perror( "setsockopt" );
# endif
#endif

	return 1;
}

static void ATTR_NORETURN
chooseHost( int hid )
{
	HostName *h;
#if defined(IPv6) && defined(AF_INET6)
	char addr[64];
#endif

	for (h = hostNamedb; h; h = h->next)
		if ((int)h == hid) {
			/* XXX error handling */
			GSet( &mstrtalk );
			if ((td->displayType & d_location) == dLocal) {
				GSendInt( D_RemoteHost );
#if defined(IPv6) && defined(AF_INET6)
				switch (h->connectionType) {
				case FamilyInternet6:
					inet_ntop( AF_INET6, h->hostaddr.data, addr, sizeof(addr) );
					break;
				default: /* FamilyInternet */
					inet_ntop( AF_INET, h->hostaddr.data, addr, sizeof(addr) );
					break;
				}
				GSendStr( addr );
#else
				GSendStr( inet_ntoa( *(struct in_addr *)h->hostaddr.data ) );
#endif
				CloseGreeter( FALSE );
				SessionExit( EX_REMOTE );
			} else {
				GSendInt( D_ChooseHost );
				GSendArr( td->clientAddr.length, (char *)td->clientAddr.data );
				GSendInt( td->connectionType );
				GSendArr( h->hostaddr.length, (char *)h->hostaddr.data );
				CloseGreeter( FALSE );
				goto bout;
			}
			break;
		}
/*	LogError( "Internal error: chose unexisting host\n" ); */
  bout:
	SessionExit( EX_NORMAL );
}

static void
directChooseHost( const char *name )
{
	HostAddr *hosts = 0;

	if (!makeSockAddrs( name, &hosts ))
		return;
	GSendInt( G_Ch_Exit );
	/* XXX error handling */
	GSet( &mstrtalk );
	if ((td->displayType & d_location) == dLocal) {
		GSendInt( D_RemoteHost );
		GSendStr( name );
		CloseGreeter( FALSE );
		SessionExit( EX_REMOTE );
	} else {
		GSendInt( D_ChooseHost );
		GSendArr( td->clientAddr.length, (char *)td->clientAddr.data );
		GSendInt( td->connectionType );
		GSendArr( hosts->addrlen, (char *)hosts->addr );
		CloseGreeter( FALSE );
		SessionExit( EX_NORMAL );
	}
}

#define PING_TRIES 3

int
DoChoose()
{
	HostName **hp, *h;
	char *host, **hostp;
	struct timeval *to, tnow, nextPing;
	int pingTry, n, cmd;
	FD_TYPE rfds;
	static int xdmcpInited;

	OpenGreeter();
	GSendInt( G_Choose );
	switch (cmd = CtrlGreeterWait( TRUE )) {
	case G_Ready:
		break;
	default: /* error */
		return cmd;
	}

	if (!xdmcpInited) {
		if (!initXDMCP())
			SessionExit( EX_UNMANAGE_DPY );
		xdmcpInited = 1;
	}
	if ((td->displayType & d_location) == dLocal) {
		/* XXX the config reader should do the lookup already */
		for (hostp = td->chooserHosts; *hostp; hostp++)
			if (!registerForPing( *hostp ))
				LogError( "Unkown host %\"s specified for local chooser preload of display %s\n", *hostp, td->name );
	} else
		ForEachChooserHost( &td->clientAddr, td->connectionType,
		                    AddChooserHost, 0 );

	GSendInt( 0 ); /* entering async mode signal */

  reping:
	for (h = hostNamedb; h; h = h->next)
		h->alive = 0;
	pingTry = 0;
	goto pingen;

	for (;;) {
		to = 0;
		if (pingTry <= PING_TRIES) {
			gettimeofday( &tnow, 0 );
			if (nextPing.tv_sec < tnow.tv_sec ||
			    (nextPing.tv_sec == tnow.tv_sec &&
			     nextPing.tv_usec < tnow.tv_usec)) {
				if (pingTry < PING_TRIES) {
				  pingen:
					pingTry++;
					doPingHosts();
					gettimeofday( &tnow, 0 );
					nextPing = tnow;
					nextPing.tv_sec++;
				} else {
					for (hp = &hostNamedb; *hp; )
						if (!(*hp)->alive) {
							h = (*hp)->next;
							disposeHostname( *hp );
							GSendInt( G_Ch_RemoveHost );
							GSendInt( (int)*hp ); /* just an id */
							*hp = h;
						} else
							hp = &(*hp)->next;
					goto noto;
				}
			}
			to = &tnow;
			tnow.tv_sec = nextPing.tv_sec - tnow.tv_sec;
			tnow.tv_usec = nextPing.tv_usec - tnow.tv_usec;
			if (tnow.tv_usec < 0) {
				tnow.tv_usec += 1000000;
				tnow.tv_sec--;
			}
		}
	  noto:
		FD_ZERO( &rfds );
		FD_SET( grtproc.pipe.rfd, &rfds );
		FD_SET( socketFD, &rfds );
#if defined(IPv6) && defined(AF_INET6)
		if (socket6FD >= 0)
			FD_SET( socket6FD, &rfds );
#endif
		n = grtproc.pipe.rfd;
		if (socketFD > n)
			n = socketFD;
#if defined(IPv6) && defined(AF_INET6)
		if (socket6FD > n)
			n = socket6FD;
#endif
		if (select( n + 1, &rfds, 0, 0, to ) > 0) {
			if (FD_ISSET( grtproc.pipe.rfd, &rfds ))
				switch (cmd = CtrlGreeterWait( FALSE )) {
				case -1:
					break;
				case G_Ch_Refresh:
					goto reping;
				case G_Ch_RegisterHost:
					host = GRecvStr();
					if (!registerForPing( host )) {
						GSendInt( G_Ch_BadHost );
						GSendStr( host );
					}
					free( host );
					goto reping;
				case G_Ch_DirectChoice:
					host = GRecvStr();
					directChooseHost( host );
					GSendInt( G_Ch_BadHost );
					GSendStr( host );
					free( host );
					break;
				case G_Ready:
					chooseHost( GRecvInt() );
					/* NOTREACHED */
				default:
					emptyHostnames();
					emptyPingHosts();
					return cmd;
				}
			if (FD_ISSET( socketFD, &rfds ))
				receivePacket( socketFD );
#if defined(IPv6) && defined(AF_INET6)
			if (socket6FD >= 0 && FD_ISSET( socket6FD, &rfds ))
				receivePacket( socket6FD );
#endif
		}
	}

}
