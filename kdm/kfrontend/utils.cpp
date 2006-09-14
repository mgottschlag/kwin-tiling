/*

Copyright (C) 2005-2006 Oswald Buddenhagen <ossi@kde.org>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#include "utils.h"
#include "kdm_greet.h"

#include <klocale.h>

#include <stdlib.h>

static void
disposeSession( dpySpec *sess )
{
	free( sess->display );
	free( sess->from );
	if (sess->user)
		free( sess->user );
	if (sess->session)
		free( sess->session );
}

dpySpec *
fetchSessions( int flags )
{
	dpySpec *sess, *sessions = 0, tsess;

	GSet( 1 );
	GSendInt( G_List );
	GSendInt( flags );
  next:
	while ((tsess.display = GRecvStr())) {
		tsess.from = GRecvStr();
#ifdef HAVE_VTS
		tsess.vt = GRecvInt();
#endif
		tsess.user = GRecvStr();
		tsess.session = GRecvStr();
		tsess.flags = GRecvInt();
		if ((tsess.flags & isTTY) && *tsess.from)
			for (sess = sessions; sess; sess = sess->next)
				if (sess->user && !strcmp( sess->user, tsess.user ) &&
				    !strcmp( sess->from, tsess.from ))
				{
					sess->count++;
					disposeSession( &tsess );
					goto next;
				}
		if (!(sess = (dpySpec *)malloc( sizeof(*sess) )))
			LogPanic( "Out of memory\n" );
		tsess.count = 1;
		tsess.next = sessions;
		*sess = tsess;
		sessions = sess;
	}
	GSet( 0 );
	return sessions;
}

void
disposeSessions( dpySpec *sess )
{
	while (sess) {
		dpySpec *nsess = sess->next;
		disposeSession( sess );
		free( sess );
		sess = nsess;
	}
}

void
decodeSess( dpySpec *sess, QString &user, QString &loc )
{
	if (sess->flags & isTTY) {
		user =
			i18np( "%1: TTY login", "%1: %n TTY logins", sess->count,
			       sess->user );
		loc =
#ifdef HAVE_VTS
			sess->vt ?
				QString("vt%1").arg( sess->vt ) :
#endif
				QString::fromLatin1( *sess->from ? sess->from : sess->display );
	} else {
		user =
			!sess->user ?
				i18n("Unused") :
				*sess->user ?
					i18nc("user: session type", "%1: %2",
					      sess->user, sess->session ) :
					i18nc("... host", "X login on %1", sess->session );
		loc =
#ifdef HAVE_VTS
			sess->vt ?
				QString("%1, vt%2").arg( sess->display ).arg( sess->vt ) :
#endif
				QString::fromLatin1( sess->display );
	}
}
