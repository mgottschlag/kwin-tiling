/****************************************************************************

 KHotKeys
 
 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

#define _ACTIONS_LISTVIEW_WIDGET_CPP_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "actions_listview_widget.h"

#include <qheader.h>

#include <klocale.h>
#include <kdebug.h>

#include <khlistview.h>

#include <actions.h>
#include <action_data.h>
#include <triggers.h>

#include "kcmkhotkeys.h"

namespace KHotKeys
{

Actions_listview_widget::Actions_listview_widget( QWidget* parent_P, const char* name_P )
    : Actions_listview_widget_ui( parent_P, name_P ), recent_item( NULL ),
        saved_current_item( NULL )
    {
//    actions_listview->setSorting( 0 );
    actions_listview->header()->hide();
    actions_listview->addColumn( "" );
    actions_listview->setRootIsDecorated( true ); // CHECKME
    connect( actions_listview, SIGNAL( current_changed( QListViewItem* )),
        SLOT( current_changed( QListViewItem* )));
    // KHotKeys::Module::changed()
    }
    
void Actions_listview_widget::action_name_changed( const QString& )
    {
    current_action()->widthChanged( 0 );
    actions_listview->repaintItem( current_action());
    }

void Actions_listview_widget::set_action_data( Action_data_base* data_P, bool recent_action_P )
    {
    if( recent_action_P )
        {
        assert( recent_item != NULL );
        recent_item->set_data( data_P );
        }
    else
        saved_current_item->set_data( data_P );
    }

void Actions_listview_widget::current_changed( QListViewItem* item_P )
    {
    kdDebug( 1217 ) << "current_changed:" << item_P << endl;
    set_current_action( static_cast< Action_listview_item* >( item_P ));
    }

void Actions_listview_widget::set_current_action( Action_listview_item* item_P )
    {
    if( item_P == saved_current_item )
        return;
    recent_item = saved_current_item;
    saved_current_item = item_P;
    if( actions_listview->currentItem() != item_P )
        {
        if( item_P == NULL )
            actions_listview->clearSelection();
        actions_listview->setCurrentItem( item_P );
        }
    emit current_action_changed();
    }

// in_group_P = don't put next to the current, but in it (it must be a group)    
void Actions_listview_widget::new_action( Action_data_base* data_P, bool in_group_P )
    {
    Action_listview_item* tmp;
    if( in_group_P )
        {
        assert( current_action() != NULL
            && dynamic_cast< Action_data_group* >( current_action()->data()) != NULL );
        tmp = new Action_listview_item( current_action(), data_P );
        current_action()->setOpen( true );
        }
    else
        {
        if( current_action() == NULL )
            tmp = new Action_listview_item( actions_listview, data_P );
        else if( current_action()->parent() == NULL )
            tmp = new Action_listview_item( actions_listview, current_action(), data_P );
        else
            tmp = new Action_listview_item( current_action()->parent(),
                current_action(), data_P );
        }
    recent_item = saved_current_item;
    saved_current_item = tmp;
    actions_listview->setSelected( tmp, true );
    }

void Actions_listview_widget::delete_action()
    {
//    while( QListViewItem* child = current_action()->firstChild())
//        delete child;
//    QListViewItem* nw = current_action()->itemAbove();
//    if( nw == NULL )
//        nw = current_action()->itemBelow();
    delete saved_current_item;
    recent_item = NULL;
//    if( nw != NULL )
//        {
//        saved_current_item = static_cast< Action_listview_item* >( nw );
//        actions_listview->setSelected( nw, true );
//        }
//    else
//        saved_current_item = NULL;
    }

void Actions_listview_widget::build_up()
    {
    build_up_recursively( module->actions_root(), NULL );
    }
    
void Actions_listview_widget::build_up_recursively( Action_data_group* parent_P,
    Action_listview_item* item_parent_P )
    {
    Action_listview_item* prev = NULL;
    for( Action_data_group::Iterator it = parent_P->first_child();
         it;
         ++it )
        {
        if( prev == NULL )
            {
            if( item_parent_P == NULL )
                prev = new Action_listview_item( actions_listview, ( *it )); 
            else
                prev = new Action_listview_item( item_parent_P, ( *it )); 
            }
        else
            {
            if( item_parent_P == NULL )
                prev = new Action_listview_item( actions_listview, prev, ( *it )); 
            else
                prev = new Action_listview_item( item_parent_P, prev, ( *it )); 
            }
        Action_data_group* grp = dynamic_cast< Action_data_group* >( *it );
        if( grp != NULL )
            build_up_recursively( grp, prev );
        }
    }
    
// Action_listview_item

QString Action_listview_item::text( int column_P ) const
    {
    return column_P == 0 ? data()->name() : QString::null;
    }

// CHECKME poradne tohle zkontrolovat po tom prekopani

} // namespace KHotKeys

#include "actions_listview_widget.moc"
