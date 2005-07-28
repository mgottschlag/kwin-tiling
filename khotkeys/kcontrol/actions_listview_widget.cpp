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

#include <q3header.h>

#include <klocale.h>
#include <kdebug.h>
#include <q3dragobject.h>

#include <khlistview.h>
#include <actions.h>
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
    connect( actions_listview, SIGNAL( current_changed( Q3ListViewItem* )),
        SLOT( current_changed( Q3ListViewItem* )));
    connect( actions_listview, SIGNAL( moved( Q3ListViewItem*, Q3ListViewItem*, Q3ListViewItem* )),
        SLOT( item_moved( Q3ListViewItem*, Q3ListViewItem*, Q3ListViewItem* )));
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

void Actions_listview_widget::current_changed( Q3ListViewItem* item_P )
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

void Actions_listview_widget::new_action( Action_data_base* data_P )
    {
    Q3ListViewItem* parent = NULL;
    if( current_action() != NULL )
        {
        if( dynamic_cast< Action_data_group* >( current_action()->data()) != NULL )
            parent = current_action();
        else
            parent = current_action()->parent();
        }
    if( parent )
        parent->setOpen( true );
    Action_listview_item* tmp = create_item( parent, NULL, data_P );
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

void Actions_listview_widget::item_moved( Q3ListViewItem* item_P, Q3ListViewItem*, Q3ListViewItem* )
    {
    Action_listview_item* item = static_cast< Action_listview_item* >( item_P );
    Action_listview_item* parent = static_cast< Action_listview_item* >( item->parent());
    if( parent == NULL )
        item->data()->reparent( module->actions_root());
    else if( Action_data_group* group = dynamic_cast< Action_data_group* >( parent->data()))
        item->data()->reparent( group );
    else
        item->data()->reparent( module->actions_root());
    module->changed();
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
        prev = create_item( item_parent_P, prev, ( *it )); 
        Action_data_group* grp = dynamic_cast< Action_data_group* >( *it );
        if( grp != NULL )
            build_up_recursively( grp, prev );
        }
    }
    
Action_listview_item* Actions_listview_widget::create_item( Q3ListViewItem* parent_P,
    Q3ListViewItem* after_P, Action_data_base* data_P )
    {
    if( parent_P != NULL )
        return new Action_listview_item( parent_P, after_P, data_P );
    else
        return new Action_listview_item( actions_listview, after_P, data_P );
    }

// Action_listview_item

QString Action_listview_item::text( int column_P ) const
    {
    return column_P == 0 ? data()->name() : QString::null;
    }

// CHECKME poradne tohle zkontrolovat po tom prekopani

} // namespace KHotKeys

#include "actions_listview_widget.moc"
