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
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include <qmime.h>
#include <qdragobject.h>
#include <qstring.h>
#include <qpixmap.h>

#include <kdebug.h>

#include "historyitem.h"
#include "historystringitem.h"
#include "historyimageitem.h"

HistoryItem::HistoryItem() {

}

HistoryItem::~HistoryItem() {

}

HistoryItem* HistoryItem::create( const QMimeSource& aSource )
{
#if 0
    int i=0;
    while ( const char* f = aSource.format( i++ ) ) {
        kdDebug() << "format(" << i <<"): " << f << endl;
    }
#endif
    if ( QTextDrag::canDecode( &aSource ) ) {
        QString text;
        QTextDrag::decode( &aSource, text );
        return text.isNull() ? 0 : new HistoryStringItem( text );
    } else if ( QImageDrag::canDecode( &aSource ) ) {
        QPixmap image;
        QImageDrag::decode( &aSource, image );
        return image.isNull() ? 0 : new HistoryImageItem( image );
    }

    return 0; // Failed.

}

HistoryItem* HistoryItem::create( QDataStream& aSource ) {
    if ( aSource.atEnd() ) {
        return 0;
    }
    QString type;
    aSource >> type;
    if ( type == "string" ) {
        QString text;
        aSource >> text;
        return new HistoryStringItem( text );
    }
    if ( type == "image" ) {
        QPixmap image;
        aSource >> image;
        return new HistoryImageItem( image );
    }
    kdWarning() << "Failed to restore history item: Unknown type \"" << type << "\"" << endl;
    return 0;
}

