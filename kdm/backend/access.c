/*

Copyright 1990, 1998  The Open Group
Copyright 2001,2004 Oswald Buddenhagen <ossi@kde.org>
Copyright 2002 Sun Microsystems, Inc.  All rights reserved.

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
 * Author:  Keith Packard, MIT X Consortium
 *
 * Access control for XDMCP - keep a database of allowable display addresses
 * and (potentially) a list of hosts to send ForwardQuery packets to
 */

#ifdef XDMCP

#include "dm.h"
#include "dm_error.h"
#include "dm_socket.h"

#include <X11/Xos.h>
#include <X11/Xdmcp.h>
#include <X11/X.h>

#include <stdio.h>
#include <ctype.h>

#include <netdb.h>
#if defined(IPv6) && defined(AF_INET6)
# include <arpa/inet.h>
#endif

typedef struct {
	short int type;
	union {
		char *aliasPattern;
		char *hostPattern;
		struct _displayAddress {
			CARD16 connectionType;
			ARRAY8 hostAddress;
		} displayAddress;
	} entry;
} HostEntry;

typedef struct {
	short int iface;
	short int mcasts;
	short int nmcasts;
} ListenEntry;

typedef struct {
	char *name;
	short int hosts;
	short int nhosts;
} AliasEntry;

typedef struct {
	short int entries;
	short int nentries;
	short int hosts;
	short int nhosts;
	short int flags;
} AclEntry;

typedef struct {
	HostEntry *hostList;
	ListenEntry *listenList;
	AliasEntry *aliasList;
	AclEntry *acList;
	short int nHosts, nListens, nAliases, nAcls;
	CfgDep dep;
} AccArr;

static AccArr accData[1];


static ARRAY8 localAddress;

ARRAY8Ptr
getLocalAddress( void )
{
	static int haveLocalAddress;

	if (!haveLocalAddress) {
#if defined(IPv6) && defined(AF_INET6)
		struct addrinfo *ai;

		if (getaddrinfo( localHostname(), NULL, NULL, &ai )) {
			XdmcpAllocARRAY8( &localAddress, 4 );
			localAddress.data[0] = 127;
			localAddress.data[1] = 0;
			localAddress.data[2] = 0;
			localAddress.data[3] = 1;
		} else {
			if (ai->ai_family == AF_INET) {
				XdmcpAllocARRAY8( &localAddress, sizeof(struct in_addr));
				memcpy( localAddress.data,
				        &((struct sockaddr_in *)ai->ai_addr)->sin_addr,
				        sizeof(struct in_addr));
			} else /* if (ai->ai_family == AF_INET6) */ {
				XdmcpAllocARRAY8( &localAddress, sizeof(struct in6_addr));
				memcpy( localAddress.data,
				        &((struct sockaddr_in6 *)ai->ai_addr)->sin6_addr,
				        sizeof(struct in6_addr));
			}
			freeaddrinfo( ai );
#else
		struct hostent *hostent;

		if ((hostent = gethostbyname( localHostname() ))) {
			XdmcpAllocARRAY8( &localAddress, hostent->h_length );
			memmove( localAddress.data, hostent->h_addr, hostent->h_length );
#endif
			haveLocalAddress = 1;
		}
	}
	return &localAddress;
}


void
ScanAccessDatabase( int force )
{
	struct _displayAddress *da;
	char *cptr;
	int nChars, i;

	Debug( "ScanAccessDatabase\n" );
	if (Setjmp( cnftalk.errjmp ))
		return; /* may memleak */
	if (startConfig( GC_gXaccess, &accData->dep, force ) <= 0)
		return;
	if (accData->hostList)
		free( accData->hostList );
	accData->nHosts = GRecvInt();
	accData->nListens = GRecvInt();
	accData->nAliases = GRecvInt();
	accData->nAcls = GRecvInt();
	nChars = GRecvInt();
	if (!(accData->hostList = (HostEntry *)
	      Malloc( accData->nHosts * sizeof(HostEntry)+
	              accData->nListens * sizeof(ListenEntry)+
	              accData->nAliases * sizeof(AliasEntry)+
	              accData->nAcls * sizeof(AclEntry)+
	              nChars )))
	{
		CloseGetter();
		return;
	}
	accData->listenList = (ListenEntry *)(accData->hostList + accData->nHosts);
	accData->aliasList = (AliasEntry *)(accData->listenList + accData->nListens);
	accData->acList = (AclEntry *)(accData->aliasList + accData->nAliases);
	cptr = (char *)(accData->acList + accData->nAcls);
	for (i = 0; i < accData->nHosts; i++) {
		switch ((accData->hostList[i].type = GRecvInt())) {
		case HOST_ALIAS:
			accData->hostList[i].entry.aliasPattern = cptr;
			cptr += GRecvStrBuf( cptr );
			break;
		case HOST_PATTERN:
			accData->hostList[i].entry.hostPattern = cptr;
			cptr += GRecvStrBuf( cptr );
			break;
		case HOST_ADDRESS:
			da = &accData->hostList[i].entry.displayAddress;
			da->hostAddress.data = (unsigned char *)cptr;
			cptr += (da->hostAddress.length = GRecvArrBuf( cptr ));
			switch (GRecvInt())
			{
#ifdef AF_INET
			case AF_INET:
				da->connectionType = FamilyInternet;
				break;
#endif
#if defined(IPv6) && defined(AF_INET6)
			case AF_INET6:
				da->connectionType = FamilyInternet6;
				break;
#endif
#ifdef AF_DECnet
			case AF_DECnet:
				da->connectionType = FamilyDECnet;
				break;
#endif
/*#ifdef AF_UNIX
			case AF_UNIX:
#endif*/
			default:
				da->connectionType = FamilyLocal;
				break;
			}
			break;
		case HOST_BROADCAST:
			break;
		default:
			LogError( "Received unknown host type %d from config reader\n", accData->hostList[i].type );
			return;
		}
	}
	for (i = 0; i < accData->nListens; i++) {
		accData->listenList[i].iface = GRecvInt();
		accData->listenList[i].mcasts = GRecvInt();
		accData->listenList[i].nmcasts = GRecvInt();
	}
	for (i = 0; i < accData->nAliases; i++) {
		accData->aliasList[i].name = cptr;
		cptr += GRecvStrBuf( cptr );
		accData->aliasList[i].hosts = GRecvInt();
		accData->aliasList[i].nhosts = GRecvInt();
	}
	for (i = 0; i < accData->nAcls; i++) {
		accData->acList[i].entries = GRecvInt();
		accData->acList[i].nentries = GRecvInt();
		accData->acList[i].hosts = GRecvInt();
		accData->acList[i].nhosts = GRecvInt();
		accData->acList[i].flags = GRecvInt();
	}
}


/* Returns non-0 if string is matched by pattern.  Does case folding.
 */
static int
patternMatch( const char *string, const char *pattern )
{
	int p, s;

	if (!string)
		string = "";

	for (;;) {
		s = *string++;
		switch (p = *pattern++) {
		case '*':
			if (!*pattern)
				return 1;
			for (string--; *string; string++)
				if (patternMatch( string, pattern ))
					return 1;
			return 0;
		case '?':
			if (s == '\0')
				return 0;
			break;
		case '\0':
			return s == '\0';
		case '\\':
			p = *pattern++;
			/* fall through */
		default:
			if (tolower( p ) != tolower( s ))
				return 0;
		}
	}
}


/*
 * calls the given function for each valid indirect entry.  Returns TRUE if
 * the local host exists on any of the lists, else FALSE
 */

#define MAX_DEPTH 32

static void
scanHostlist( int fh, int nh,
              ARRAY8Ptr clientAddress, CARD16 connectionType,
              ChooserFunc function, char *closure,
              int depth, int broadcast, int *haveLocalhost )
{
	HostEntry *h;
	AliasEntry *a;
	int na;

	for (h = accData->hostList + fh; nh; nh--, h++) {
		switch (h->type) {
		case HOST_ALIAS:
			if (depth == MAX_DEPTH) {
				LogError( "Alias recursion in XDMCP access control list\n" );
				break;
			}
			for (a = accData->aliasList, na = accData->nAliases; na; na--, a++)
				if (patternMatch( a->name, h->entry.aliasPattern )) /* XXX originally swapped, no wildcards in alias name matching */
					scanHostlist( a->hosts, a->nhosts,
					              clientAddress, connectionType,
					              function, closure, depth + 1, broadcast,
					              haveLocalhost );
			break;
		case HOST_ADDRESS:
			if (XdmcpARRAY8Equal( getLocalAddress(), &h->entry.displayAddress.hostAddress ))
				*haveLocalhost = 1;
			else if (function)
				(*function)( connectionType, &h->entry.displayAddress.hostAddress, closure );
			break;
		case HOST_BROADCAST:
			if (broadcast && function)
				(*function)( FamilyBroadcast, 0, closure );
			break;
		default:
			break;
		}
	}
}

static int
scanEntrylist( int fh, int nh,
               ARRAY8Ptr clientAddress, CARD16 connectionType,
               char **clientName, int depth )
{
	HostEntry *h;
	AliasEntry *a;
	int na;

	for (h = accData->hostList + fh; nh; nh--, h++) {
		switch (h->type) {
		case HOST_ALIAS:
			if (depth == MAX_DEPTH) {
				LogError( "Alias recursion in XDMCP access control list\n" );
				break;
			}
			for (a = accData->aliasList, na = accData->nAliases; na; na--, a++)
				if (patternMatch( a->name, h->entry.aliasPattern ))
					if (scanEntrylist( a->hosts, a->nhosts,
					                   clientAddress, connectionType,
					                   clientName, depth + 1 ))
						return 1;
			break;
		case HOST_PATTERN:
			if (!*clientName)
				*clientName = NetworkAddressToHostname( connectionType,
				                                        clientAddress );
			if (patternMatch( *clientName, h->entry.hostPattern ))
				return 1;
			break;
		case HOST_ADDRESS:
			if (h->entry.displayAddress.connectionType == connectionType &&
			    XdmcpARRAY8Equal( &h->entry.displayAddress.hostAddress,
			                      clientAddress ))
				return 1;
			break;
		default:
			break;
		}
	}
	return 0;
}

static AclEntry *
matchAclEntry( ARRAY8Ptr clientAddress, CARD16 connectionType, int direct )
{
	AclEntry *e, *re;
	char *clientName = 0;
	int ne;

	for (e = accData->acList, ne = accData->nAcls, re = 0; ne; ne--, e++)
		if (!e->nhosts == direct)
			if (scanEntrylist( e->entries, e->nentries,
			                   clientAddress, connectionType,
			                   &clientName, 0 ))
			{
				re = e;
				break;
			}
	if (clientName)
		free( clientName );
	return re;
}

int
ForEachMatchingIndirectHost( ARRAY8Ptr clientAddress,
                             CARD16 connectionType,
                             ChooserFunc function, char *closure )
{
	AclEntry *e;
	int haveLocalhost = 0;

	e = matchAclEntry( clientAddress, connectionType, 0 );
	if (e && !(e->flags & a_notAllowed)) {
		if (e->flags & a_useChooser) {
			ARRAY8Ptr choice;

			choice = IndirectChoice( clientAddress, connectionType );
			if (!choice || XdmcpARRAY8Equal( getLocalAddress(), choice ))
				haveLocalhost = 1;
			else
				(*function)( connectionType, choice, closure );
		} else
			scanHostlist( e->hosts, e->nhosts, clientAddress, connectionType,
			              function, closure, 0, FALSE, &haveLocalhost );
	}
	return haveLocalhost;
}

int
UseChooser( ARRAY8Ptr clientAddress, CARD16 connectionType )
{
	AclEntry *e;

	e = matchAclEntry( clientAddress, connectionType, 0 );
	return e && !(e->flags & a_notAllowed) && (e->flags & a_useChooser) &&
		!IndirectChoice( clientAddress, connectionType );
}

void
ForEachChooserHost( ARRAY8Ptr clientAddress, CARD16 connectionType,
                    ChooserFunc function, char *closure )
{
	AclEntry *e;
	int haveLocalhost = 0;

	e = matchAclEntry( clientAddress, connectionType, 0 );
	if (e && !(e->flags & a_notAllowed) && (e->flags & a_useChooser))
		scanHostlist( e->hosts, e->nhosts, clientAddress, connectionType,
		              function, closure, 0, TRUE, &haveLocalhost );
	if (haveLocalhost)
		(*function)( connectionType, getLocalAddress(), closure );
}

/*
 * returns TRUE if the given client is acceptable to the local host.  The
 * given display client is acceptable if it occurs without a host list.
 */
int
AcceptableDisplayAddress( ARRAY8Ptr clientAddress, CARD16 connectionType,
                          xdmOpCode type )
{
	AclEntry *e;

	if (type == INDIRECT_QUERY)
		return 1;

	e = matchAclEntry( clientAddress, connectionType, 1 );
	return e && !(e->flags & a_notAllowed) &&
		(type != BROADCAST_QUERY || !(e->flags & a_notBroadcast));
}

void
ForEachListenAddr( ListenFunc listenfunction, ListenFunc mcastfunction,
                   void **closure )
{
	int i, j, ifc, mc, nmc;

	for (i = 0; i < accData->nListens; i++) {
		ifc = accData->listenList[i].iface;
		(*listenfunction)( ifc < 0 ? 0 :
		                   &accData->hostList[ifc].entry.displayAddress.hostAddress,
		                   closure );
		mc = accData->listenList[i].mcasts;
		nmc = accData->listenList[i].nmcasts;
		for (j = 0; j < nmc; j++, mc++)
			(*mcastfunction)( &accData->hostList[mc].entry.displayAddress.hostAddress,
			                  closure );
	}
}
#endif /* XDMCP */
