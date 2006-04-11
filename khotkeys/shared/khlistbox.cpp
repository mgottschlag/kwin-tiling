/****************************************************************************

 KHotKeys
 
 Copyright (C) 1999-2002 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

#define _KHLISTBOX_CPP_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "khlistbox.h"

#include <kdebug.h>

namespace KHotKeys
{

KHListBox::KHListBox( QWidget* parent_P, const char* name_P )
    : Q3ListBox( parent_P, name_P ), saved_current_item( NULL ),
        in_clear( false ), force_select( false )
    {
    insert_select_timer.setSingleShot( true );
    
    connect( this, SIGNAL( selectionChanged( Q3ListBoxItem* )),
        SLOT( slot_selection_changed( Q3ListBoxItem* )));
    connect( this, SIGNAL( currentChanged( Q3ListBoxItem* )),
        SLOT( slot_current_changed( Q3ListBoxItem* )));
    // CHECKME grrr
    connect( this, SIGNAL( selectionChanged()),
        SLOT( slot_selection_changed()));
    connect( &insert_select_timer, SIGNAL( timeout()),
        SLOT( slot_insert_select()));
    }

void KHListBox::slot_selection_changed()
    {
    if( saved_current_item == NULL )
        slot_selection_changed( NULL );
    else if( !saved_current_item->isSelected()) // no way
        setSelected( saved_current_item, true );
    }
    
void KHListBox::slot_selection_changed( Q3ListBoxItem* item_P )
    {
    if( item_P == saved_current_item )
        return;
    saved_current_item = item_P;
    setCurrentItem( saved_current_item );
    emit current_changed( saved_current_item );
    }
    
void KHListBox::slot_current_changed( Q3ListBoxItem* item_P )
    {
    insert_select_timer.stop();
    if( item_P == saved_current_item )
        return;
    saved_current_item = item_P;
    setSelected( saved_current_item, true );
    emit current_changed( saved_current_item );
    }

// neni virtual :((
void KHListBox::clear()
    {
    in_clear = true;
    Q3ListBox::clear();
    in_clear = false;
    slot_selection_changed( NULL );
    }


// neni virtual :(( a vubec nefunguje
void KHListBox::insertItem( Q3ListBoxItem* item_P )
    {
    bool set = false;
    if( !in_clear )
        set = count() == 0;
    Q3ListBox::insertItem( item_P );
    if( set && force_select )
        {
        bool block = signalsBlocked();
        blockSignals( true );
        setCurrentItem( item_P );
        blockSignals( block );
        insert_select_timer.start( 0 );
        }
    }

// items are often inserted using the QListBoxItem constructor,
// which means that a derived class are not yet fully created
void KHListBox::slot_insert_select()
    {
    slot_current_changed( item( currentItem()));
    }
    

} // namespace KHotKeys

#include "khlistbox.moc"
