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


#ifndef UTILS_H
#define UTILS_H

#include <greet.h>

#include <QList>
#include <QString>

QString qString( char *str );
QStringList qStringList( char **strList );

struct DpySpec {
	QString display, from, user, session;
#ifdef HAVE_VTS
	int vt;
#endif
	int flags;
	int count;
};

QList<DpySpec> fetchSessions( int flags );

void decodeSession( const DpySpec &sess, QString &user, QString &loc );

#endif /* UTILS_H */
