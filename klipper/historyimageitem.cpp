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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <qstring.h>

#include <kdebug.h>

#include "historyimageitem.h"

HistoryImageItem::HistoryImageItem( const QPixmap& data )
    : HistoryItem(),  m_data( data )
{
}

const QString& HistoryImageItem::text() const {
    if ( m_text.isNull() ) {
        m_text = QString( "%1x%2x%3 %4" )
                 .arg( m_data.width() )
                 .arg( m_data.height() )
               .arg( m_data.depth() );
    }
    return m_text;

}

/* virtual */
void HistoryImageItem::write( QDataStream& stream ) const {
    stream << QString( "image" ) << m_data;
}
