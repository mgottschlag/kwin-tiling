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

QString qString( char *str )
{
	if (!str)
		return QString();
	QString qs = QString::fromUtf8( str );
	free( str );
	return qs;
}

QStringList qStringList( char **strList )
{
	QStringList qsl;
	for (int i = 0; strList[i]; i++) {
		qsl.append( QString::fromUtf8( strList[i] ) );
		free( strList[i] );
	}
	free( strList );
	return qsl;
}

QList<DpySpec>
fetchSessions( int flags )
{
	QList<DpySpec> sessions;
	DpySpec tsess;

	gSet( 1 );
	gSendInt( G_List );
	gSendInt( flags );
  next:
	while (!(tsess.display = qString( gRecvStr() )).isEmpty()) {
		tsess.from = qString( gRecvStr() );
#ifdef HAVE_VTS
		tsess.vt = gRecvInt();
#endif
		tsess.user = qString( gRecvStr() );
		tsess.session = qString( gRecvStr() );
		tsess.flags = gRecvInt();
		if ((tsess.flags & isTTY) && !tsess.from.isEmpty())
			for (int i = 0; i < sessions.size(); i++)
				if (!sessions[i].user.isEmpty() &&
				    sessions[i].user == tsess.user &&
				    sessions[i].from == tsess.from)
				{
					sessions[i].count++;
					goto next;
				}
		tsess.count = 1;
		sessions.append( tsess );
	}
	gSet( 0 );
	return sessions;
}

void
decodeSession( const DpySpec &sess, QString &user, QString &loc )
{
	if (sess.flags & isTTY) {
		user =
			i18ncp( "user: ...", "%2: TTY login", "%2: %1 TTY logins",
			        sess.count, sess.user );
		loc =
#ifdef HAVE_VTS
			sess.vt ?
				QString("vt%1").arg( sess.vt ) :
#endif
				!sess.from.isEmpty() ?
					sess.from : sess.display;
	} else {
		user =
			sess.session.isEmpty() ?
				i18nc("... session", "Unused") :
				!sess.user.isEmpty() ?
					i18nc( "user: session type", "%1: %2",
					       sess.user, sess.session ) :
					i18nc( "... host", "X login on %1", sess.session );
		loc =
#ifdef HAVE_VTS
			sess.vt ?
				QString("%1, vt%2").arg( sess.display ).arg( sess.vt ) :
#endif
				sess.display;
	}
}
