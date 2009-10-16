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
      m_topIsUserSelected( false ),
      m_nextCycle(-1)
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

    // OptimizationCompare with top item. If identical, the top isn't changing
    if ( !itemList.isEmpty() && *itemList.first() == *item ) {
        const HistoryItem* top = itemList.first();
        itemList.first() = item;
        delete top;
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
    m_nextCycle = (itemList.size()>2)?1:-1;
    emit changed();
    trim();
}

void History::trim() {
    int i = itemList.count() - maxSize();
    if ( i <= 0 )
        return;

    while ( i-- ) {
        itemList.removeLast();
    }
    if (itemList.size()<=1) {
        m_nextCycle = -1;
    }
    emit changed();
}

void History::remove( const HistoryItem* newItem ) {
    if ( !newItem )
        return;

    // TODO: This is rather broken.. it only checks by pointer!
    if (itemList.contains(newItem)) {
        itemList.removeAll(newItem);
        emit changed();
    }
}


void History::slotClear() {
    itemList.clear();
    emit changed();
}

void History::slotMoveToTop(QAction *action) {
    bool ok = false;
    int pos = action->data().toInt(&ok);
    if (!ok) // not an action from popupproxy
        return;

    if ( pos < 0 || pos >= itemList.count() ) {
        kDebug() << "Argument pos out of range: " << pos;
        return;
    }

    m_topIsUserSelected = true;

    itemList.move(pos, 0);
    m_nextCycle = (itemList.size()>2)?1:-1;
    emit changed();
    emit topChanged();
}

void History::setMaxSize( unsigned max_size ) {
    m_max_size = max_size;
    trim();

}

KlipperPopup* History::popup() {
    return m_popup;
}

void History::cycleNext() {
    if (m_nextCycle != -1 && m_nextCycle < itemList.size()) {
        itemList.swap(0, m_nextCycle++);
        emit changed();
    }
}

void History::cyclePrev()
{
    if (m_nextCycle > 1 && m_nextCycle - 1 < itemList.size()) {
        itemList.swap(0, --m_nextCycle);
        emit changed();
    }

}


const HistoryItem* History::nextInCycle() const
{
    return (m_nextCycle != -1 && m_nextCycle < itemList.size()) ? itemList.at(m_nextCycle) : 0L;

}

const HistoryItem* History::prevInCycle() const
{
    return (m_nextCycle > 1 && m_nextCycle -1 < itemList.size()) ? itemList.at(m_nextCycle-1) : 0L;

}

#include "history.moc"
