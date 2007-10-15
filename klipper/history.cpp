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

History::History( QObject* parent )
    : QObject( parent ),
      m_popup( new KlipperPopup( this ) ),
      m_topIsUserSelected( false )
{
    connect( this, SIGNAL( changed() ), m_popup, SLOT( slotHistoryChanged() ) );

}


History::~History() {
    qDeleteAll(itemList);
}

History::iterator History::youngest() {
    return iterator( itemList );
}

void History::insert( const HistoryItem* item ) {
    if ( !item )
        return;

    m_topIsUserSelected = false;

    // Optimization: Compare with top item.
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

    if (itemList.contains(newItem)) {
        itemList.removeAll(newItem);
        emit changed();
    }
}


void History::slotClear() {
    itemList.clear();
    emit changed();
}

void History::slotMoveToTop(int pos ) {
    if ( pos < 0 || static_cast<unsigned>( pos ) >= itemList.count() ) {
        kDebug() << "Argument pos out of range: " << pos;
        return;
    }

    m_topIsUserSelected = true;

    itemList.move(pos, 0);
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
