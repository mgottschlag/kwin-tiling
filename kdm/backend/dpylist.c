/* $TOG: dpylist.c /main/30 1998/02/09 13:55:07 kaleb $ */
/* $Id$ */
/*

Copyright 1988, 1998  The Open Group

All Rights Reserved.

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from The Open Group.

*/
/* $XFree86: xc/programs/xdm/dpylist.c,v 1.3 2000/04/27 16:26:50 eich Exp $ */

/*
 * xdm - display manager daemon
 * Author:  Keith Packard, MIT X Consortium
 *
 * a simple linked list of known displays
 */

#include "dm.h"
#include "dm_error.h"

#include <signal.h>

static struct display	*displays;
static struct disphist	*disphist;

int
AnyDisplaysLeft (void)
{
    return displays != (struct display *) 0;
}

int
AnyActiveDisplays (void)
{
    struct display *d;

Debug("AnyActiveDisplays?\n");
    for (d = displays; d; d = d->next)
	if (d->userSess >= 0)
{Debug(" yes\n");
	    return 1;
}Debug(" no\n");
    return 0;
}

int
AllLocalDisplaysLocked (struct display *dp)
{
    struct display *d;

Debug("AllLocalDisplaysLocked?\n");
    for (d = displays; d; d = d->next)
	if (d != dp &&
	    (d->displayType & d_location) == dLocal &&
	    d->status == running && !d->hstent->lock)
{Debug(" no\n");
	    return 0;
}Debug(" yes\n");
    return 1;
}

void
StartReserveDisplay (int lt)
{
    struct display *d;

Debug("StartReserveDisplay\n");
    for (d = displays; d; d = d->next)
	if (d->status == reserve)
	{
Debug("starting reserve display %s, timeout %d\n", d->name, lt);
	    d->idleTimeout = lt;
	    StartDisplay (d);
	    break;
	}
}

void
ReapReserveDisplays (void)
{
    struct display *d, *rd;

Debug("ReapReserveDisplays\n");
    for (rd = 0, d = displays; d; d = d->next)
	if ((d->displayType & d_location) == dLocal && d->status == running &&
	    !d->hstent->lock)
	{
	    if (rd)
	    {
		rd->idleTimeout = 0;
Debug ("killing reserve display %s\n", rd->name);
		if (rd->pid != -1)
		    kill (rd->pid, SIGALRM);
		rd = 0;
	    }
	    if ((d->displayType & d_lifetime) == dReserve &&
		d->userSess < 0)
		rd = d;
	}
}

void
ForEachDisplay (void (*f)(struct display *))
{
    struct display *d, *next;

    for (d = displays; d; d = next) {
	next = d->next;
	(*f) (d);
    }
}

struct display *
FindDisplayByName (char *name)
{
    struct display *d;

    for (d = displays; d; d = d->next)
	if (!strcmp (name, d->name))
	    return d;
    return 0;
}

struct display *
FindDisplayByPid (int pid)
{
    struct display *d;

    for (d = displays; d; d = d->next)
	if (pid == d->pid)
	    return d;
    return 0;
}

struct display *
FindDisplayByServerPid (int serverPid)
{
    struct display *d;

    for (d = displays; d; d = d->next)
	if (serverPid == d->serverPid)
	    return d;
    return 0;
}

#ifdef XDMCP

struct display *
FindDisplayBySessionID (CARD32 sessionID)
{
    struct display	*d;

    for (d = displays; d; d = d->next)
	if (sessionID == d->sessionID)
	    return d;
    return 0;
}

struct display *
FindDisplayByAddress (XdmcpNetaddr addr, int addrlen, CARD16 displayNumber)
{
    struct display  *d;

    for (d = displays; d; d = d->next)
	if ((d->displayType & d_origin) == dFromXDMCP &&
	    d->displayNumber == displayNumber &&
	    addressEqual ((XdmcpNetaddr)d->from.data, d->from.length, 
			  addr, addrlen))
	    return d;
    return 0;
}

#endif /* XDMCP */

#define IfFree(x)  if (x) free ((char *) x)
    
void
RemoveDisplay (struct display *old)
{
    struct display	*d, **dp;
    int			i;

    for (dp = &displays; (d = *dp); dp = &(*dp)->next) {
	if (d == old) {
Debug ("Removing display %s\n", d->name);
	    *dp = d->next;
	    IfFree (d->class2);
	    IfFree (d->cfg.data);
	    delStr (d->cfg.dep.name);
	    freeStrArr (d->serverArgv);
	    if (d->authorizations)
	    {
		for (i = 0; i < d->authNum; i++)
		    XauDisposeAuth (d->authorizations[i]);
		free ((char *) d->authorizations);
	    }
	    if (d->authFile) {
		(void) unlink (d->authFile);
		free (d->authFile);
	    }
	    IfFree (d->fifoPath);
	    IfFree (d->authNameLens);
#ifdef XDMCP
	    XdmcpDisposeARRAY8 (&d->peer);
	    XdmcpDisposeARRAY8 (&d->from);
	    XdmcpDisposeARRAY8 (&d->clientAddr);
#endif
	    free ((char *) d);
	    break;
	}
    }
}

static struct disphist *
FindHist (char *name)
{
    struct disphist *hstent;

    for (hstent = disphist; hstent; hstent = hstent->next)
	if (!strcmp (hstent->name, name))
	    return hstent;
    return 0;
}

struct display *
NewDisplay (char *name, char *class2)
{
    struct display	*d;
    struct disphist	*hstent;

    if (!(hstent = FindHist (name))) {
	if (!(hstent = calloc (1, sizeof (*hstent)))) {
	    LogOutOfMem ("NewDisplay");
	    return 0;
	}
	if (!StrDup (&hstent->name, name)) {
	    free (hstent);
	    LogOutOfMem ("NewDisplay");
	    return 0;
	}
	hstent->next = disphist; disphist = hstent;
    }

    if (!(d = (struct display *) calloc (1, sizeof (*d)))) {
	LogOutOfMem ("NewDisplay");
	return 0;
    }
    d->next = displays;
    d->hstent = hstent;
    d->name = hstent->name;
    if (!StrDup (&d->class2, class2)) {
	LogOutOfMem ("NewDisplay");
	free ((char *) d);
	return 0;
    }
    /* initialize fields (others are 0) */
    d->status = notRunning;
    d->pid = -1;
    d->serverPid = -1;
    d->fifofd = -1;
    d->pipefd[0] = -1;
    d->pipefd[1] = -1;
    d->userSess = -1;
    displays = d;
Debug ("Created new display %s\n", d->name);
    return d;
}
