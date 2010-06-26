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

#include "dm.h"
#include "dm_error.h"
#include "dm_socket.h"

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
getLocalAddress(void)
{
    static int haveLocalAddress;

    if (!haveLocalAddress) {
#if defined(IPv6) && defined(AF_INET6)
        struct addrinfo *ai;

        if (getaddrinfo(localHostname(), 0, 0, &ai)) {
            XdmcpAllocARRAY8(&localAddress, 4);
            localAddress.data[0] = 127;
            localAddress.data[1] = 0;
            localAddress.data[2] = 0;
            localAddress.data[3] = 1;
        } else {
            if (ai->ai_family == AF_INET) {
                XdmcpAllocARRAY8(&localAddress, sizeof(struct in_addr));
                memcpy(localAddress.data,
                       &((struct sockaddr_in *)ai->ai_addr)->sin_addr,
                       sizeof(struct in_addr));
            } else /* if (ai->ai_family == AF_INET6) */ {
                XdmcpAllocARRAY8(&localAddress, sizeof(struct in6_addr));
                memcpy(localAddress.data,
                       &((struct sockaddr_in6 *)ai->ai_addr)->sin6_addr,
                       sizeof(struct in6_addr));
            }
            freeaddrinfo(ai);
#else
        struct hostent *hostent;

        if ((hostent = gethostbyname(localHostname()))) {
            XdmcpAllocARRAY8(&localAddress, hostent->h_length);
            memmove(localAddress.data, hostent->h_addr, hostent->h_length);
#endif
            haveLocalAddress = True;
        }
    }
    return &localAddress;
}


void
scanAccessDatabase(int force)
{
    struct _displayAddress *da;
    char *cptr;
    int nChars, i;

    debug("scanAccessDatabase\n");
    if (Setjmp(cnftalk.errjmp))
        return; /* may memleak */
    if (startConfig(GC_gXaccess, &accData->dep, force) <= 0)
        return;
    free(accData->hostList);
    accData->nHosts = gRecvInt();
    accData->nListens = gRecvInt();
    accData->nAliases = gRecvInt();
    accData->nAcls = gRecvInt();
    nChars = gRecvInt();
    if (!(accData->hostList = (HostEntry *)
              Malloc(accData->nHosts * sizeof(HostEntry) +
                     accData->nListens * sizeof(ListenEntry) +
                     accData->nAliases * sizeof(AliasEntry) +
                     accData->nAcls * sizeof(AclEntry) +
                     nChars))) {
        closeGetter();
        return;
    }
    accData->listenList = (ListenEntry *)(accData->hostList + accData->nHosts);
    accData->aliasList = (AliasEntry *)(accData->listenList + accData->nListens);
    accData->acList = (AclEntry *)(accData->aliasList + accData->nAliases);
    cptr = (char *)(accData->acList + accData->nAcls);
    for (i = 0; i < accData->nHosts; i++) {
        switch ((accData->hostList[i].type = gRecvInt())) {
        case HOST_ALIAS:
            accData->hostList[i].entry.aliasPattern = cptr;
            cptr += gRecvStrBuf(cptr);
            break;
        case HOST_PATTERN:
            accData->hostList[i].entry.hostPattern = cptr;
            cptr += gRecvStrBuf(cptr);
            break;
        case HOST_ADDRESS:
            da = &accData->hostList[i].entry.displayAddress;
            da->hostAddress.data = (unsigned char *)cptr;
            cptr += (da->hostAddress.length = gRecvArrBuf(cptr));
            switch (gRecvInt()) {
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
            logError("Received unknown host type %d from config reader\n", accData->hostList[i].type);
            return;
        }
    }
    for (i = 0; i < accData->nListens; i++) {
        accData->listenList[i].iface = gRecvInt();
        accData->listenList[i].mcasts = gRecvInt();
        accData->listenList[i].nmcasts = gRecvInt();
    }
    for (i = 0; i < accData->nAliases; i++) {
        accData->aliasList[i].name = cptr;
        cptr += gRecvStrBuf(cptr);
        accData->aliasList[i].hosts = gRecvInt();
        accData->aliasList[i].nhosts = gRecvInt();
    }
    for (i = 0; i < accData->nAcls; i++) {
        accData->acList[i].entries = gRecvInt();
        accData->acList[i].nentries = gRecvInt();
        accData->acList[i].hosts = gRecvInt();
        accData->acList[i].nhosts = gRecvInt();
        accData->acList[i].flags = gRecvInt();
    }
}


/* Returns True if string is matched by pattern.  Does case folding.
 */
static int
patternMatch(const char *string, const char *pattern)
{
    int p, s;

    if (!string)
        string = "";

    for (;;) {
        s = *string++;
        switch (p = *pattern++) {
        case '*':
            if (!*pattern)
                return True;
            for (string--; *string; string++)
                if (patternMatch(string, pattern))
                    return True;
            return False;
        case '?':
            if (s == '\0')
                return False;
            break;
        case '\0':
            return s == '\0';
        case '\\':
            p = *pattern++;
            /* fall through */
        default:
            if (tolower(p) != tolower(s))
                return False;
        }
    }
}


#define MAX_DEPTH 32

static void
scanHostlist(int fh, int nh,
             ARRAY8Ptr clientAddress, CARD16 connectionType,
             ChooserFunc function, char *closure,
             int broadcast, int *haveLocalhost)
{
    HostEntry *h;
    AliasEntry *a;
    int na;

    for (h = accData->hostList + fh; nh; nh--, h++) {
        switch (h->type) {
        case HOST_ALIAS:
            for (a = accData->aliasList, na = accData->nAliases; na; na--, a++)
                if (patternMatch(a->name, h->entry.aliasPattern)) /* XXX originally swapped, no wildcards in alias name matching */
                    scanHostlist(a->hosts, a->nhosts,
                                 clientAddress, connectionType,
                                 function, closure, broadcast,
                                 haveLocalhost);
            break;
        case HOST_ADDRESS:
            if (haveLocalhost &&
                    XdmcpARRAY8Equal(getLocalAddress(), &h->entry.displayAddress.hostAddress))
                *haveLocalhost = True;
            else if (function)
                (*function)(connectionType, &h->entry.displayAddress.hostAddress, closure);
            break;
        case HOST_BROADCAST:
            if (broadcast && function)
                (*function)(FamilyBroadcast, 0, closure);
            break;
        default:
            break;
        }
    }
}

static int
scanEntrylist(int fh, int nh,
              ARRAY8Ptr clientAddress, CARD16 connectionType,
              char **clientName)
{
    HostEntry *h;
    AliasEntry *a;
    int na;

    for (h = accData->hostList + fh; nh; nh--, h++) {
        switch (h->type) {
        case HOST_ALIAS:
            for (a = accData->aliasList, na = accData->nAliases; na; na--, a++)
                if (patternMatch(a->name, h->entry.aliasPattern))
                    if (scanEntrylist(a->hosts, a->nhosts,
                                      clientAddress, connectionType,
                                      clientName))
                        return True;
            break;
        case HOST_PATTERN:
            if (!*clientName)
                *clientName = networkAddressToHostname(connectionType,
                                                       clientAddress);
            if (patternMatch(*clientName, h->entry.hostPattern))
                return True;
            break;
        case HOST_ADDRESS:
            if (h->entry.displayAddress.connectionType == connectionType &&
                XdmcpARRAY8Equal(&h->entry.displayAddress.hostAddress,
                                 clientAddress))
                return True;
            break;
        default:
            break;
        }
    }
    return False;
}

static AclEntry *
matchAclEntry(ARRAY8Ptr clientAddress, CARD16 connectionType, int direct)
{
    AclEntry *e, *re;
    char *clientName = 0;
    int ne;

    for (e = accData->acList, ne = accData->nAcls, re = 0; ne; ne--, e++)
        if (!e->nhosts == direct)
            if (scanEntrylist(e->entries, e->nentries,
                              clientAddress, connectionType,
                              &clientName))
            {
                re = e;
                break;
            }
    free(clientName);
    return re;
}

/*
 * calls the given function for each valid indirect entry.  Returns True if
 * the local host exists on any of the lists, else False
 */
int
forEachMatchingIndirectHost(ARRAY8Ptr clientAddress, ARRAY8Ptr clientPort,
                            CARD16 connectionType,
                            ChooserFunc function, char *closure)
{
    AclEntry *e;
    int haveLocalhost = False;

    e = matchAclEntry(clientAddress, connectionType, False);
    if (e && !(e->flags & a_notAllowed)) {
        if (e->flags & a_useChooser) {
            ARRAY8Ptr choice;

            choice = indirectChoice(clientAddress, clientPort, connectionType);
            if (!choice || XdmcpARRAY8Equal(getLocalAddress(), choice))
                /* If nothing was chosen yet, we want to pop up the chooser.
                 * If we were chosen, we want to pop up the greeter. */
                haveLocalhost = True;
            else
                /* If something else was chosen, we forward the query to
                 * the chosen host. */
                (*function)(connectionType, choice, closure);
        } else
            /* Just forward the query to each listed host. */
            scanHostlist(e->hosts, e->nhosts, clientAddress, connectionType,
                         function, closure, False, &haveLocalhost);
    }
    return haveLocalhost;
}

int
useChooser(ARRAY8Ptr clientAddress, CARD16 connectionType)
{
    AclEntry *e;

    e = matchAclEntry(clientAddress, connectionType, False);
    return e && !(e->flags & a_notAllowed) && (e->flags & a_useChooser);
}

void
forEachChooserHost(ARRAY8Ptr clientAddress, CARD16 connectionType,
                   ChooserFunc function, char *closure)
{
    AclEntry *e;

    e = matchAclEntry(clientAddress, connectionType, False);
    if (e && !(e->flags & a_notAllowed) && (e->flags & a_useChooser))
        scanHostlist(e->hosts, e->nhosts, clientAddress, connectionType,
                     function, closure, True, 0);
}

/*
 * returns True if the given client is acceptable to the local host.  The
 * given display client is acceptable if it occurs without a host list.
 */
int
acceptableDisplayAddress(ARRAY8Ptr clientAddress, CARD16 connectionType,
                         xdmOpCode type)
{
    AclEntry *e;

    if (type == INDIRECT_QUERY)
        return True;

    e = matchAclEntry(clientAddress, connectionType, True);
    return e && !(e->flags & a_notAllowed) &&
        (type != BROADCAST_QUERY || !(e->flags & a_notBroadcast));
}

void
forEachListenAddr(ListenFunc listenfunction, ListenFunc mcastfunction,
                  void **closure)
{
    int i, j, ifc, mc, nmc;

    for (i = 0; i < accData->nListens; i++) {
        ifc = accData->listenList[i].iface;
        (*listenfunction)(ifc < 0 ? 0 :
                          &accData->hostList[ifc].entry.displayAddress.hostAddress,
                          closure);
        mc = accData->listenList[i].mcasts;
        nmc = accData->listenList[i].nmcasts;
        for (j = 0; j < nmc; j++, mc++)
            (*mcastfunction)(&accData->hostList[mc].entry.displayAddress.hostAddress,
                             closure);
    }
}
