/****************************************************************************

 KHotKeys

 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.

****************************************************************************/

#define _WINDOWDEF_LIST_WIDGET_CPP_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "windowdef_list_widget.h"

#include <assert.h>
#include <QPushButton>
#include <q3header.h>
#include <QLineEdit>
#include <QMenu>

#include <kdebug.h>
#include <klocale.h>

#include <khlistview.h>

#include <windows.h>

#include "windowdef_simple_widget.h"
#include "kcmkhotkeys.h"

namespace KHotKeys
{

// Windowdef_list_widget

Windowdef_list_widget::Windowdef_list_widget( QWidget* parent_P, const char* name_P )
    : Windowdef_list_widget_ui( parent_P, name_P ), autodetect_object( NULL ),
        autodetect_slot( NULL ), selected_item( NULL )
    {
    QMenu* popup = new QMenu; // CHECKME looks like setting parent doesn't work
    QAction *action = popup->addAction( i18n( "Simple Window..." ) );
    connect( action, SIGNAL( triggered()), SLOT( new_selected()));

    connect( windows_listview, SIGNAL( doubleClicked ( Q3ListViewItem *, const QPoint &, int ) ),
             this, SLOT( modify_pressed() ) );
    new_button->setMenu( popup );
    windows_listview->header()->hide();
    windows_listview->addColumn( "" );
    windows_listview->setSorting( -1 );
    windows_listview->setForceSelect( true );
    copy_button->setEnabled( false );
    modify_button->setEnabled( false );
    delete_button->setEnabled( false );
    clear_data();
    // KHotKeys::Module::changed()
    connect( new_button, SIGNAL( clicked()),
        module, SLOT( changed()));
    connect( copy_button, SIGNAL( clicked()),
        module, SLOT( changed()));
    connect( modify_button, SIGNAL( clicked()),
        module, SLOT( changed()));
    connect( delete_button, SIGNAL( clicked()),
        module, SLOT( changed()));
    connect( comment_lineedit, SIGNAL( textChanged( const QString& )),
        module, SLOT( changed()));
    }

Windowdef_list_widget::~Windowdef_list_widget()
    {
    delete new_button->menu();
    }

void Windowdef_list_widget::clear_data()
    {
    comment_lineedit->clear();
    windows_listview->clear();
    }

void Windowdef_list_widget::set_data( const Windowdef_list* data_P )
    {
    if( data_P == NULL )
        {
        clear_data();
        return;
        }
    comment_lineedit->setText( data_P->comment());
    Windowdef_list_item* after = NULL;
    windows_listview->clear();
    for( Windowdef_list::Iterator it( *data_P );
         *it;
         ++it )
        after = create_listview_item( *it, windows_listview, NULL, after, true );
    }

Windowdef_list* Windowdef_list_widget::get_data() const
    {
// CHECKME TODO hmm, tady to bude chtit asi i children :(
    Windowdef_list* list = new Windowdef_list( comment_lineedit->text());
    for( Q3ListViewItem* pos = windows_listview->firstChild();
         pos != NULL;
         pos = pos->nextSibling())
        list->append( static_cast< Windowdef_list_item* >( pos )->window()->copy());
    return list;
    }

void Windowdef_list_widget::new_selected()
    {
    Windowdef_dialog* dlg = NULL;
    dlg = new Windowdef_simple_dialog(
        new Windowdef_simple( "", "", Windowdef_simple::NOT_IMPORTANT, "",
        Windowdef_simple::NOT_IMPORTANT, "", Windowdef_simple::NOT_IMPORTANT,
        Windowdef_simple::WINDOW_TYPE_NORMAL | Windowdef_simple::WINDOW_TYPE_DIALOG ),
        NULL, NULL ); // CHECKME tady pak autodetect
    if( dlg != NULL )
        {
        Windowdef* window = dlg->edit_windowdef();
        if( window != NULL )
            windows_listview->setSelected( create_listview_item( window, windows_listview,
                NULL, selected_item, false ), true );
                // CHECKME tady pak jeste spravne vnoreni, kdyz bude skupina
        delete dlg;
        }
    }

void Windowdef_list_widget::copy_pressed()
    {
    windows_listview->setSelected( create_listview_item( selected_item->window(),
        selected_item->parent() ? NULL : windows_listview, selected_item->parent(),
        selected_item, true ), true );
    }

void Windowdef_list_widget::delete_pressed()
    {
    delete selected_item; // CHECKME snad vyvola signaly pro enable()
    }

void Windowdef_list_widget::modify_pressed()
    {
    edit_listview_item( selected_item );
    }

void Windowdef_list_widget::current_changed( Q3ListViewItem* item_P )
    {
//    if( item_P == selected_item )
//        return;
    selected_item = static_cast< Windowdef_list_item* >( item_P );
//    windows_listview->setSelected( item_P, true );
    copy_button->setEnabled( item_P != NULL );
    modify_button->setEnabled( item_P != NULL );
    delete_button->setEnabled( item_P != NULL );
    }

Windowdef_list_item* Windowdef_list_widget::create_listview_item( Windowdef* window_P,
    Q3ListView* parent1_P, Q3ListViewItem* parent2_P, Q3ListViewItem* after_P, bool copy_P )
    {
    Windowdef* new_win = copy_P ? window_P->copy() : window_P;
// CHECKME uz by nemelo byt treba
/*    if( after_P == NULL )
        {
        if( parent1_P == NULL )
            return new Windowdef_list_item( parent2_P, new_win );
        else
            return new Windowdef_list_item( parent1_P, new_win );
        }
    else*/
        {
        if( parent1_P == NULL )
            return new Windowdef_list_item( parent2_P, after_P, new_win );
        else
            return new Windowdef_list_item( parent1_P, after_P, new_win );
        }
    }

void Windowdef_list_widget::edit_listview_item( Windowdef_list_item* item_P )
    {
    Windowdef_dialog* dlg = NULL;
    if( Windowdef_simple* window = dynamic_cast< Windowdef_simple* >( item_P->window()))
        dlg = new Windowdef_simple_dialog( window, autodetect_object, autodetect_slot );
    else // CHECKME TODO pridat dalsi
        assert( false );
    Windowdef* new_window = dlg->edit_windowdef();
    if( new_window != NULL )
        {
        item_P->set_window( new_window );
        item_P->widthChanged( 0 ); // SELI tohle i u dalsich listview?
        windows_listview->repaintItem( item_P );
        }
    delete dlg;
    }

// Windowdef_list_item

QString Windowdef_list_item::text( int column_P ) const
    {
    return column_P == 0 ? window()->description() : QString();
    }

Windowdef_list_item::~Windowdef_list_item()
    {               // CHECKME if the listview will ever be used hiearchically,
    delete _window; // this will be wrong, the windows tree will have to be kept
    }               // and deleted separately

// Windowdef_simple_dialog

Windowdef_simple_dialog::Windowdef_simple_dialog( Windowdef_simple* window_P, QObject* obj_P,
    const char* slot_P )
    : KDialog( 0 ), window( NULL )
    {
    setModal( true );
    setCaption( i18n( "Window Details" ) );
    setButtons( Ok | Cancel );
    widget = new Windowdef_simple_widget( this );
    widget->set_autodetect( obj_P, slot_P );
    widget->set_data( window_P );
    setMainWidget( widget );
    }

Windowdef* Windowdef_simple_dialog::edit_windowdef()
    {
    exec();
    return window;
    }

void Windowdef_simple_dialog::accept()
    {
    KDialog::accept();
    window = widget->get_data(); // CHECKME NULL ?
    }


} // namespace KHotKeys

#include "windowdef_list_widget.moc"
