/*
 * $TOG: access.c /main/17 1998/02/09 13:54:13 kaleb $
 * $Id$
 *
Copyright 1990, 1998  The Open Group

All Rights Reserved.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group.
 *
 * Author:  Keith Packard, MIT X Consortium
 */

/* $XFree86: xc/programs/xdm/access.c,v 3.5 1998/10/10 15:25:30 dawes Exp $ */

/*
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

#ifndef MINIX
# include <netdb.h>
#else /* MINIX */
# include <net/gen/netdb.h>
#endif /* !MINIX */

typedef struct {
    short int	type;
    union _hostOrAlias {
	char	*aliasPattern;
	char	*hostPattern;
	struct _display {
	    CARD16	connectionType;
	    ARRAY8	hostAddress;
	} displayAddress;
    } entry;
} HostEntry;

typedef struct {
    char	*name;
    short int	hosts;
    short int	nhosts;
} AliasEntry;

typedef struct {
    short int	entries;
    short int	nentries;
    short int	hosts;
    short int	nhosts;
    short int	flags;
} AclEntry;

typedef struct {
    HostEntry		*hostList;
    AliasEntry		*aliasList;
    AclEntry		*acList;
    short int		nHosts, nAliases, nAcls;
    CfgDep		dep;
} AccArr;

static AccArr		accData[1];


static ARRAY8		localAddress;

ARRAY8Ptr
getLocalAddress (void)
{
    static int	haveLocalAddress;
    
    if (!haveLocalAddress)
    {
	struct hostent	*hostent;

	hostent = gethostbyname (localHostname());
	XdmcpAllocARRAY8 (&localAddress, hostent->h_length);
	memmove( localAddress.data, hostent->h_addr, hostent->h_length);
    }
    return &localAddress;
}


void
ScanAccessDatabase (int force)
{
    struct _display *da;
    char *cptr;
    int nChars, i;

Debug("ScanAccessDatabase\n");
    if (!startConfig (GC_gXaccess, &accData->dep, force))
        return;
    if (accData->hostList)
	free (accData->hostList);
    accData->nHosts = GRecvInt ();
    accData->nAliases = GRecvInt ();
    accData->nAcls = GRecvInt ();
    nChars = GRecvInt ();
    if (!(accData->hostList = (HostEntry *)
	    malloc (accData->nHosts * sizeof(HostEntry) +
		    accData->nAliases * sizeof(AliasEntry) +
		    accData->nAcls * sizeof(AclEntry) +
		    nChars)))
    {
	LogOutOfMem ("ScanAccessDatabase");
	CloseGetter ();
	return;
    }
    accData->aliasList = (AliasEntry *)(accData->hostList + accData->nHosts);
    accData->acList = (AclEntry *)(accData->aliasList + accData->nAliases);
    cptr = (char *)(accData->acList + accData->nAcls);
    for (i = 0; i < accData->nHosts; i++) {
Debug ("Host entry %d at %p:\n", i, &accData->hostList[i]);
	switch ((accData->hostList[i].type = GRecvInt ())) {
	case HOST_ALIAS:
	    accData->hostList[i].entry.aliasPattern = cptr;
	    cptr += GRecvStrBuf (cptr);
Debug ("Alias pattern %s\n", accData->hostList[i].entry.aliasPattern);
	    break;
	case HOST_PATTERN:
	    accData->hostList[i].entry.hostPattern = cptr;
	    cptr += GRecvStrBuf (cptr);
Debug ("Host pattern %s\n", accData->hostList[i].entry.hostPattern);
	    break;
	case HOST_ADDRESS:
	    da = &accData->hostList[i].entry.displayAddress;
	    da->hostAddress.data = (unsigned char *)cptr;
	    cptr += (da->hostAddress.length = GRecvArrBuf (cptr));
	    switch (GRecvInt ())
	    {
#ifdef AF_INET
	    case AF_INET:
		da->connectionType = FamilyInternet;
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
Debug ("Address %02[:*hhx, type %d\n",
	    da->hostAddress.length,
	    da->hostAddress.data,
	    da->connectionType);
	    break;
	default:
Debug ("Other host type %d\n", accData->hostList[i].type);
	    break;
	}
    }
    for (i = 0; i < accData->nAliases; i++) {
	accData->aliasList[i].name = cptr;
	cptr += GRecvStrBuf (cptr);
	accData->aliasList[i].hosts = GRecvInt ();
	accData->aliasList[i].nhosts = GRecvInt ();
Debug ("Alias %s, %d hosts at %d\n", accData->aliasList[i].name,
	accData->aliasList[i].nhosts, accData->aliasList[i].hosts);
    }
    for (i = 0; i < accData->nAcls; i++) {
	accData->acList[i].entries = GRecvInt ();
	accData->acList[i].nentries = GRecvInt ();
	accData->acList[i].hosts = GRecvInt ();
	accData->acList[i].nhosts = GRecvInt ();
	accData->acList[i].flags = GRecvInt ();
Debug ("Entry at %p: %d entries at %d, %d hosts at %d, flags %d\n", 
	&accData->acList[i],
	accData->acList[i].nentries,
	accData->acList[i].entries,
	accData->acList[i].nhosts,
	accData->acList[i].hosts,
	accData->acList[i].flags);
    }
}


/* Returns non-0 if string is matched by pattern.  Does case folding.
 */
static int
patternMatch (const char *string, const char *pattern)
{
    int	    p, s;

    if (!string)
	string = "";

    for (;;)
    {
	s = *string++;
	switch (p = *pattern++) {
	case '*':
	    if (!*pattern)
		return 1;
	    for (string--; *string; string++)
		if (patternMatch (string, pattern))
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
	    if (tolower(p) != tolower(s))
		return 0;
	}
    }
}


/*
 * calls the given function for each valid indirect entry.  Returns TRUE if
 * the local host exists on any of the lists, else FALSE
 */

#define MAX_DEPTH   32

static void
scanHostlist (
    int		fh,
    int		nh,
    ARRAY8Ptr	clientAddress,
    CARD16	connectionType,
    ChooserFunc	function,
    char	*closure,
    int		depth,
    int		broadcast,
    int		*haveLocalhost)
{
    HostEntry	*h;
    AliasEntry	*a;
    int		na;

    for (h = accData->hostList + fh; nh; nh--, h++)
    {
	switch (h->type) {
	case HOST_ALIAS:
Debug ("scanHostlist: alias pattern %s\n", h->entry.aliasPattern);
	    if (depth == MAX_DEPTH) {
		LogError ("Alias recursion in XDMCP access control list\n");
		break;
	    }
	    for (a = accData->aliasList, na = accData->nAliases; na; na--, a++)
{Debug ("alias name %s\n", a->name);
		if (patternMatch (a->name, h->entry.aliasPattern))	/* XXX originally swapped, no wildcards in alias name matching */
{Debug (" matches\n");
		    scanHostlist (a->hosts, a->nhosts,
				  clientAddress, connectionType,
				  function, closure, depth + 1, broadcast,
				  haveLocalhost);
}}	    break;
	case HOST_ADDRESS:
Debug ("scanHostlist: host address %02[*hhx\n", h->entry.displayAddress.hostAddress.length, h->entry.displayAddress.hostAddress.data);
	    if (XdmcpARRAY8Equal (getLocalAddress(), &h->entry.displayAddress.hostAddress))
		*haveLocalhost = 1;
	    else if (function)
		(*function) (connectionType, &h->entry.displayAddress.hostAddress, closure);
	    break;
	case HOST_BROADCAST:
Debug ("scanHostlist: broadcast\n");
	    if (broadcast && function)
	    {
		ARRAY8	temp;
		temp.data = (BYTE *) "BROADCAST";
		temp.length = 9;
		(*function) (connectionType, &temp, closure);
	    }
	    break;
	default:
Debug ("scanHostlist: whatever\n");
	    break;
	}
    }
}

static int
scanEntrylist(
    int		fh,
    int		nh,
    ARRAY8Ptr	clientAddress,
    CARD16	connectionType,
    char	**clientName,
    int		depth)
{
    HostEntry	*h;
    AliasEntry	*a;
    int		na;

    for (h = accData->hostList + fh; nh; nh--, h++)
    {
Debug ("hostentry %p\n", h);
	switch (h->type) {
	case HOST_ALIAS:
Debug ("scanEntrylist: alias pattern %s\n", h->entry.aliasPattern);
	    if (depth == MAX_DEPTH) {
		LogError ("Alias recursion in XDMCP access control list\n");
		break;
	    }
	    for (a = accData->aliasList, na = accData->nAliases; na; na--, a++)
{Debug ("alias name %s\n", a->name);
		if (patternMatch (a->name, h->entry.aliasPattern))
{Debug (" matches\n");
		    if (scanEntrylist (a->hosts, a->nhosts, 
				       clientAddress, connectionType,
				       clientName, depth + 1))
			return 1;
}}	    break;
	case HOST_PATTERN:
Debug ("scanEntrylist: host pattern %s\n", h->entry.hostPattern);
	    if (!*clientName)
		*clientName = NetworkAddressToHostname (connectionType,
							clientAddress);
	    if (patternMatch (*clientName, h->entry.hostPattern))
		return 1;
	    break;
	case HOST_ADDRESS:
Debug ("scanEntrylist: host address %02[*hhx\n", h->entry.displayAddress.hostAddress.length, h->entry.displayAddress.hostAddress.data);
	    if (h->entry.displayAddress.connectionType == connectionType &&
	    	XdmcpARRAY8Equal (&h->entry.displayAddress.hostAddress,
				  clientAddress))
		return 1;
	    break;
	default:
Debug ("scanEntrylist: whatever\n");
	    break;
	}
    }
    return 0;
}

static AclEntry *
matchAclEntry (
    ARRAY8Ptr	clientAddress,
    CARD16	connectionType,
    int		direct)
{
    AclEntry	*e, *re;
    char	*clientName = 0;
    int		ne;

    for (e = accData->acList, ne = accData->nAcls, re = 0; ne; ne--, e++)
{Debug ("potential aclentry %p (nhosts is %d)\n", e, e->nhosts);
	if (!e->nhosts == direct)
{Debug ("aclentry %p\n", e);
	    if (scanEntrylist (e->entries, e->nentries, 
			       clientAddress, connectionType, 
			       &clientName, 0))
	    {
		re = e;
		break;
	    }
}}    if (clientName)
	free (clientName);
    return re;
}

int ForEachMatchingIndirectHost (
    ARRAY8Ptr	clientAddress,
    CARD16	connectionType,
    ChooserFunc	function,
    char	*closure)
{
    AclEntry	*e;
    int		haveLocalhost = 0;

    e = matchAclEntry (clientAddress, connectionType, 0);
    if (e && !(e->flags & a_notAllowed))
    {
	if (e->flags & a_useChooser)
	{
	    ARRAY8Ptr	choice;

	    choice = IndirectChoice (clientAddress, connectionType);
	    if (!choice || XdmcpARRAY8Equal (getLocalAddress(), choice))
		haveLocalhost = 1;
	    else
		(*function) (connectionType, choice, closure);
	}
	else
	    scanHostlist (e->hosts, e->nhosts, clientAddress, connectionType,
			  function, closure, 0, FALSE, &haveLocalhost);
    }
    return haveLocalhost;
}

int UseChooser (
    ARRAY8Ptr	clientAddress,
    CARD16	connectionType)
{
    AclEntry	*e;

    e = matchAclEntry (clientAddress, connectionType, 0);
    return e && !(e->flags & a_notAllowed) && (e->flags & a_useChooser) && 
	!IndirectChoice (clientAddress, connectionType);
}

void ForEachChooserHost (
    ARRAY8Ptr	clientAddress,
    CARD16	connectionType,
    ChooserFunc	function,
    char	*closure)
{
    AclEntry	*e;
    int		haveLocalhost = 0;

    e = matchAclEntry (clientAddress, connectionType, 0);
    if (e && !(e->flags & a_notAllowed) && (e->flags & a_useChooser))
	scanHostlist (e->hosts, e->nhosts, clientAddress, connectionType,
		      function, closure, 0, TRUE, &haveLocalhost);
    if (haveLocalhost)
	(*function) (connectionType, getLocalAddress(), closure);
}

/*
 * returns TRUE if the given client is acceptable to the local host.  The
 * given display client is acceptable if it occurs without a host list.
 */

int
AcceptableDisplayAddress (
    ARRAY8Ptr	clientAddress,
    CARD16	connectionType,
    xdmOpCode	type)
{
    AclEntry	*e;

    if (type == INDIRECT_QUERY)
	return 1;

    e = matchAclEntry (clientAddress, connectionType, 1);
Debug ("matched %p\n", e);
    return e && !(e->flags & a_notAllowed) && 
	(type != BROADCAST_QUERY || !(e->flags & a_notBroadcast));
}

#endif /* XDMCP */
