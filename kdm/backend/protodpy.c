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
 * manage a collection of proto-displays.  These are displays for
 * which sessionID's have been generated, but no session has been
 * started.
 */

#include "dm.h"
#include "dm_error.h"

static struct protoDisplay *protoDisplays;

struct protoDisplay *
findProtoDisplay(XdmcpNetaddr address,
                 int addrlen,
                 CARD16 displayNumber)
{
    struct protoDisplay *pdpy;

    debug("findProtoDisplay\n");
    for (pdpy = protoDisplays; pdpy; pdpy = pdpy->next)
        if (pdpy->displayNumber == displayNumber &&
                addressEqual(address, addrlen, pdpy->address, pdpy->addrlen))
            return pdpy;
    return 0;
}

static void
timeoutProtoDisplays(void)
{
    struct protoDisplay *pdpy, *next;

    for (pdpy = protoDisplays; pdpy; pdpy = next) {
        next = pdpy->next;
        if (pdpy->date < (unsigned long)(now - PROTO_TIMEOUT))
            disposeProtoDisplay(pdpy);
    }
}

struct protoDisplay *
newProtoDisplay(XdmcpNetaddr address, int addrlen, CARD16 displayNumber,
                CARD16 connectionType, ARRAY8Ptr connectionAddress,
                CARD32 sessionID)
{
    struct protoDisplay *pdpy;

    debug("newProtoDisplay\n");
    timeoutProtoDisplays();
    pdpy = Malloc(sizeof(*pdpy));
    if (!pdpy)
        return 0;
    pdpy->address = Malloc(addrlen);
    if (!pdpy->address) {
        free(pdpy);
        return 0;
    }
    pdpy->addrlen = addrlen;
    memmove(pdpy->address, address, addrlen);
    pdpy->displayNumber = displayNumber;
    pdpy->connectionType = connectionType;
    pdpy->date = now;
    if (!XdmcpCopyARRAY8(connectionAddress, &pdpy->connectionAddress)) {
        free(pdpy->address);
        free(pdpy);
        return 0;
    }
    pdpy->sessionID = sessionID;
    pdpy->fileAuthorization = 0;
    pdpy->xdmcpAuthorization = 0;
    pdpy->next = protoDisplays;
    protoDisplays = pdpy;
    return pdpy;
}

void
disposeProtoDisplay(struct protoDisplay *pdpy)
{
    struct protoDisplay *p, *prev;

    prev = False;
    for (p = protoDisplays; p; p = p->next) {
        if (p == pdpy)
            break;
        prev = p;
    }
    if (!p)
        return;
    if (prev)
        prev->next = pdpy->next;
    else
        protoDisplays = pdpy->next;
    bzero(&pdpy->key, sizeof(pdpy->key));
    if (pdpy->fileAuthorization)
        XauDisposeAuth(pdpy->fileAuthorization);
    if (pdpy->xdmcpAuthorization)
        XauDisposeAuth(pdpy->xdmcpAuthorization);
    XdmcpDisposeARRAY8(&pdpy->connectionAddress);
    free(pdpy->address);
    free(pdpy);
}
