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

#include <config.h>

#ifdef XDMCP

#include "dm.h"
#include "dm_error.h"

static struct protoDisplay *protoDisplays;

#ifdef DEBUG
static
PrintProtoDisplay( pdpy )
	struct protoDisplay *pdpy;
{
	char addrbuf[128];

	PrintSockAddr( addrbuf, pdpy->address, pdpy->addrlen );
	Debug( "ProtoDisplay %p\n\taddress: %s\n\tdate %d (%d from now)\n"
	       "\tdisplay Number %d\n\tsessionID %d\n",
	       pdpy, addrbuf, pdpy->date, now - pdpy->date,
	       pdpy->displayNumber, pdpy->sessionID );
}
#endif

struct protoDisplay *
FindProtoDisplay(
                 XdmcpNetaddr address,
                 int addrlen,
                 CARD16 displayNumber )
{
	struct protoDisplay *pdpy;

	Debug( "FindProtoDisplay\n" );
	for (pdpy = protoDisplays; pdpy; pdpy=pdpy->next) {
		if (pdpy->displayNumber == displayNumber &&
		    addressEqual( address, addrlen, pdpy->address, pdpy->addrlen ))
		{
			return pdpy;
		}
	}
	return (struct protoDisplay *)0;
}

static void
TimeoutProtoDisplays (void)
{
	struct protoDisplay *pdpy, *next;

	for (pdpy = protoDisplays; pdpy; pdpy = next) {
		next = pdpy->next;
		if (pdpy->date < (unsigned long)(now - PROTO_TIMEOUT))
			DisposeProtoDisplay( pdpy );
	}
}

struct protoDisplay *
NewProtoDisplay( XdmcpNetaddr address, int addrlen, CARD16 displayNumber,
                 CARD16 connectionType, ARRAY8Ptr connectionAddress,
                 CARD32 sessionID )
{
	struct protoDisplay *pdpy;

	Debug( "NewProtoDisplay\n" );
	TimeoutProtoDisplays ();
	pdpy = (struct protoDisplay *)Malloc( sizeof *pdpy );
	if (!pdpy)
		return NULL;
	pdpy->address = (XdmcpNetaddr)Malloc( addrlen );
	if (!pdpy->address) {
		free( (char *)pdpy );
		return NULL;
	}
	pdpy->addrlen = addrlen;
	memmove( pdpy->address, address, addrlen );
	pdpy->displayNumber = displayNumber;
	pdpy->connectionType = connectionType;
	pdpy->date = now;
	if (!XdmcpCopyARRAY8( connectionAddress, &pdpy->connectionAddress )) {
		free( (char *)pdpy->address );
		free( (char *)pdpy );
		return NULL;
	}
	pdpy->sessionID = sessionID;
	pdpy->fileAuthorization = (Xauth *)NULL;
	pdpy->xdmcpAuthorization = (Xauth *)NULL;
	pdpy->next = protoDisplays;
	protoDisplays = pdpy;
	return pdpy;
}

void
DisposeProtoDisplay( pdpy )
	struct protoDisplay *pdpy;
{
	struct protoDisplay *p, *prev;

	prev = 0;
	for (p = protoDisplays; p; p=p->next) {
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
	bzero( &pdpy->key, sizeof(pdpy->key) );
	if (pdpy->fileAuthorization)
		XauDisposeAuth( pdpy->fileAuthorization );
	if (pdpy->xdmcpAuthorization)
		XauDisposeAuth( pdpy->xdmcpAuthorization );
	XdmcpDisposeARRAY8( &pdpy->connectionAddress );
	free( (char *)pdpy->address );
	free( (char *)pdpy );
}

#endif /* XDMCP */
