// -*- Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 8; -*-
/* This file is part of the KDE project
   Copyright (C) 2004  Esben Mose Hansen <kde@mosehansen.dk>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include "historyurlitem.h"

#include <k3multipledrag.h>
#include <k3urldrag.h>
#include <Qt3Support/Q3CString>

HistoryURLItem::HistoryURLItem( const KUrl::List &_urls, KUrl::MetaDataMap _metaData, bool _cut )
    : urls( _urls ), metaData( _metaData ), cut( _cut )
{
}

/* virtual */
void HistoryURLItem::write( QDataStream& stream ) const
{
    stream << QString( "url" ) << urls << metaData << (int)cut;
}

QString HistoryURLItem::text() const {
    return urls.toStringList().join( " " );
}

QMimeData* HistoryURLItem::mimeData() const {
    QMimeData *data = new QMimeData();
    urls.populateMimeData(data, metaData);
    data->setData("application/x-kde-cutselection", QByteArray(cut ? "1" : "0"));
    return data;
}

bool HistoryURLItem::operator==( const HistoryItem& rhs) const
{
    if ( const HistoryURLItem* casted_rhs = dynamic_cast<const HistoryURLItem*>( &rhs ) ) {
        return casted_rhs->urls == urls
            && casted_rhs->metaData.count() == metaData.count()
            && qEqual( casted_rhs->metaData.begin(), casted_rhs->metaData.end(), metaData.begin())
            && casted_rhs->cut == cut;
    }
    return false;
}
