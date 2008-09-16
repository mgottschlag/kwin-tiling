/*
   Copyright (c) 2008 Sebastian Trueg <trueg@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef _NEPOMUK_SEARCH_DBUS_OPERATORS_H_
#define _NEPOMUK_SEARCH_DBUS_OPERATORS_H_

#include <QtDBus/QDBusArgument>

#include "result.h"
#include "query.h"
#include "term.h"

#include <nepomuk/nepomuk_export.h>

namespace Nepomuk {
    namespace Search {
        NEPOMUK_EXPORT void registerDBusTypes();
    }
}

NEPOMUK_EXPORT QDBusArgument& operator<<( QDBusArgument& arg, const Soprano::Node& );
NEPOMUK_EXPORT const QDBusArgument& operator>>( const QDBusArgument& arg, Soprano::Node& );

NEPOMUK_EXPORT QDBusArgument& operator<<( QDBusArgument& arg, const Nepomuk::Search::Term& term );
NEPOMUK_EXPORT const QDBusArgument& operator>>( const QDBusArgument& arg, Nepomuk::Search::Term& );

NEPOMUK_EXPORT QDBusArgument& operator<<( QDBusArgument& arg, const Nepomuk::Search::Query& query );
NEPOMUK_EXPORT const QDBusArgument& operator>>( const QDBusArgument& arg, Nepomuk::Search::Query& );

NEPOMUK_EXPORT QDBusArgument& operator<<( QDBusArgument& arg, const Nepomuk::Search::Result& );
NEPOMUK_EXPORT const QDBusArgument& operator>>( const QDBusArgument& arg, Nepomuk::Search::Result& );

#endif
