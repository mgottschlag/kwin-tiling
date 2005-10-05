// -*- Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 8; -*-
/* This file is part of the KDE project
   Copyright (C) 2004  Esben Mose Hansen <kde@mosehansen.dk>
   Copyright (C) by Andrew Stanley-Jones
   Copyright (C) 2000 by Carsten Pfeiffer <pfeiffer@kde.org>

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
#include <kdebug.h>

#include "history.h"
#include "historystringitem.h"
#include "klipperpopup.h"

History::History( QWidget* parent, const char* name )
    : QObject( parent,  name ),
      m_popup( new KlipperPopup( this, parent ) ),
      m_topIsUserSelected( false )
{
    connect( this, SIGNAL( changed() ), m_popup,  SLOT( slotHistoryChanged() ) );
    itemList.setAutoDelete( true );

}


History::~History() {
}

History::iterator History::youngest() {
    return iterator( itemList );
}

void History::insert( const HistoryItem* item ) {
    if ( !item )
        return;

    m_topIsUserSelected = false;

    // Optimisation: Compare with top item.
    if ( !itemList.isEmpty() && *itemList.first() == *item ) {
        delete item;
        return;
    }

    remove( item );
    forceInsert( item );

    emit topChanged();

}

void History::forceInsert( const HistoryItem* item ) {
    if ( !item )
        return;
    itemList.prepend( item );
    emit changed();
    trim();
}

void History::trim() {
    int i = itemList.count() - max_size();
    if ( i <= 0 )
        return;

    while ( i-- ) {
        itemList.removeLast();
    }
    emit changed();
}

void History::remove( const HistoryItem* newItem ) {
    if ( !newItem )
        return;

    for ( const HistoryItem* item = itemList.first(); item; item=next() ) {
        if ( *item == *newItem ) {
            itemList.remove();
            emit changed();
            return;
        }
    }

}


void History::slotClear() {
    itemList.clear();
    emit changed();
}

void History::slotMoveToTop(int pos ) {
    if ( pos < 0 || static_cast<unsigned>( pos ) >= itemList.count() ) {
        kdDebug() << "Argument pos out of range: " << pos << endl;
        return;
    }

    m_topIsUserSelected = true;

    itemList.first();
    for ( ; pos; pos-- ) {
        itemList.next();
    }
    HistoryItem* item = itemList.take();
    itemList.prepend( item );
    emit changed();
    emit topChanged();
}

void History::max_size( unsigned max_size ) {
    m_max_size = max_size;
    trim();

}

KlipperPopup* History::popup() {
    return m_popup;
}

#include "history.moc"
