/* -------------------------------------------------------------

   history.cpp (part of Klipper - Cut & paste history for KDE)

   (C) Esben Mose Hansen <kde@mosehansen.dk>
   (C) by Andrew Stanley-Jones
   (C) 2000 by Carsten Pfeiffer <pfeiffer@kde.org>

   Generated with the KDE Application Generator

   Licensed under the GNU GPL Version 2

 ------------------------------------------------------------- */

#include <kdebug.h>

#include "history.h"
#include "historystringitem.h"
#include "klipperpopup.h"

History::History( QWidget* parent, const char* name )
    : QObject( parent,  name ),
      m_popup( new KlipperPopup( this, parent, "main_widget" ) )
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
    remove( item );
    forceInsert( item );
}

void History::forceInsert( const HistoryItem* item ) {
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
    if ( newItem ) {
        QPtrListIterator<HistoryItem> it( itemList );
    }
    QString newText( newItem->text() );
    for ( const HistoryItem* item = itemList.current(); item; item=next() ) {
        if ( item->text() == newText ) {
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
    itemList.first();
    for ( ; pos; pos-- ) {
        itemList.next();
    }
    HistoryItem* item = itemList.take();
    itemList.prepend( item );
    emit changed();

}

void History::max_size( unsigned max_size ) {
    m_max_size = max_size;
    trim();

}

KlipperPopup* History::popup() {
    return m_popup;
}

#include "history.moc"
