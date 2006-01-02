/****************************************************************************

 KHotKeys

 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.

****************************************************************************/

#define _TRIGGERS_TAB_CPP_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "triggers_tab.h"

#include <assert.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qmenu.h>
#include <qlayout.h>
#include <qlabel.h>
#include <q3header.h>
//Added by qt3to4:
#include <QVBoxLayout>

#include <kdebug.h>
#include <klocale.h>
#include <kapplication.h>
#include <kshortcut.h>
#include <kconfig.h>
#include <kshortcutlist.h>
#include <kkeybutton.h>
#include <kkeydialog.h>
#include <kvbox.h>

#include "kcmkhotkeys.h"
#include "windowdef_list_widget.h"
#include "window_trigger_widget.h"
#include "gesturerecordpage.h"

namespace KHotKeys
{

// Triggers_tab

Triggers_tab::Triggers_tab( QWidget* parent_P, const char* name_P )
    : Triggers_tab_ui( parent_P, name_P ), selected_item( NULL )
    {
    QMenu* popup = new QMenu; // CHECKME looks like setting parent doesn't work
    popup->insertItem( i18n( "Shortcut Trigger..." ), TYPE_SHORTCUT_TRIGGER );
    popup->insertItem( i18n( "Gesture Trigger..." ), TYPE_GESTURE_TRIGGER );
    popup->insertItem( i18n( "Window Trigger..." ), TYPE_WINDOW_TRIGGER );
    connect( popup, SIGNAL( activated( int )), SLOT( new_selected( int )));
    connect( triggers_listview, SIGNAL( doubleClicked ( Q3ListViewItem *, const QPoint &, int ) ),
             this, SLOT( modify_pressed() ) );

    new_button->setPopup( popup );
    copy_button->setEnabled( false );
    modify_button->setEnabled( false );
    delete_button->setEnabled( false );
    triggers_listview->header()->hide();
    triggers_listview->addColumn( "" );
    triggers_listview->setSorting( -1 );
    triggers_listview->setForceSelect( true );
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

Triggers_tab::~Triggers_tab()
    {
    delete new_button->popup(); // CHECKME
    }

void Triggers_tab::clear_data()
    {
    comment_lineedit->clear();
    triggers_listview->clear();
    }

void Triggers_tab::set_data( const Trigger_list* data_P )
    {
    if( data_P == NULL )
        {
        clear_data();
        return;
        }
    comment_lineedit->setText( data_P->comment());
    Trigger_list_item* after = NULL;
    triggers_listview->clear();
    for( Trigger_list::Iterator it( *data_P );
         *it;
         ++it )
        after = create_listview_item( *it, triggers_listview, after, true );
    }

Trigger_list* Triggers_tab::get_data( Action_data* data_P ) const
    {
    Trigger_list* list = new Trigger_list( comment_lineedit->text());
    for( Q3ListViewItem* pos = triggers_listview->firstChild();
         pos != NULL;
         pos = pos->nextSibling())
        list->append( static_cast< Trigger_list_item* >( pos )->trigger()->copy( data_P ));
    return list;
    }

void Triggers_tab::new_selected( int type_P )
    {
    Trigger_dialog* dlg = NULL;
    switch( type_P )
        {
        case TYPE_SHORTCUT_TRIGGER: // Shortcut_trigger
            dlg = new Shortcut_trigger_dialog(
                new Shortcut_trigger( NULL, KShortcut())); // CHECKME NULL ?
          break;
        case TYPE_GESTURE_TRIGGER: // Gesture trigger
            dlg = new Gesture_trigger_dialog(
                new Gesture_trigger( NULL, QString() )); // CHECKME NULL ?
          break;
        case TYPE_WINDOW_TRIGGER: // Window trigger
            dlg = new Window_trigger_dialog( new Window_trigger( NULL, new Windowdef_list( "" ),
                0 )); // CHECKME NULL ?
          break;
        }
    if( dlg != NULL )
        {
        Trigger* trg = dlg->edit_trigger();
        if( trg != NULL )
            triggers_listview->setSelected( create_listview_item( trg, triggers_listview,
                selected_item, false ), true );
        delete dlg;
        }
    }

void Triggers_tab::copy_pressed()
    {
    triggers_listview->setSelected( create_listview_item( selected_item->trigger(),
        triggers_listview, selected_item, true ), true );
    }

void Triggers_tab::delete_pressed()
    {
    delete selected_item; // CHECKME snad vyvola signaly pro enable()
    }

void Triggers_tab::modify_pressed()
    {
    edit_listview_item( selected_item );
    }

void Triggers_tab::current_changed( Q3ListViewItem* item_P )
    {
//    if( item_P == selected_item )
//        return;
    selected_item = static_cast< Trigger_list_item* >( item_P );
//    triggers_listview->setSelected( item_P, true );
    copy_button->setEnabled( item_P != NULL );
    modify_button->setEnabled( item_P != NULL );
    delete_button->setEnabled( item_P != NULL );
    }

Trigger_list_item* Triggers_tab::create_listview_item( Trigger* trigger_P,
    Q3ListView* parent_P, Q3ListViewItem* after_P, bool copy_P )
    {
    Trigger* new_trg = copy_P ? trigger_P->copy( NULL ) : trigger_P; // CHECKME NULL ?
// CHECKME uz by nemelo byt treba    if( after_P == NULL )
//        return new Trigger_list_item( parent_P, new_trg );
//    else
        return new Trigger_list_item( parent_P, after_P, new_trg );
    }

void Triggers_tab::edit_listview_item( Trigger_list_item* item_P )
    {
    Trigger_dialog* dlg = NULL;
    if( Shortcut_trigger* trg = dynamic_cast< Shortcut_trigger* >( item_P->trigger()))
        dlg = new Shortcut_trigger_dialog( trg );
    else if( Gesture_trigger* trg = dynamic_cast< Gesture_trigger* >( item_P->trigger()))
        dlg = new Gesture_trigger_dialog( trg );
    else if( Window_trigger* trg = dynamic_cast< Window_trigger* >( item_P->trigger()))
        dlg = new Window_trigger_dialog( trg );
// CHECKME TODO dalsi
    else
        assert( false );
    Trigger* new_trigger = dlg->edit_trigger();
    if( new_trigger != NULL )
        item_P->set_trigger( new_trigger );
    delete dlg;
    }

// Trigger_list_item

QString Trigger_list_item::text( int column_P ) const
    {
    return column_P == 0 ? trigger()->description() : QString();
    }

// Shortcut_trigger_widget

Shortcut_trigger_widget::Shortcut_trigger_widget( QWidget* parent_P, const char* )
    : QWidget( parent_P )
    {
    QVBoxLayout* lay = new QVBoxLayout( this, 11, 6 );
    QLabel* lbl = new QLabel( i18n( "Select keyboard shortcut:" ), this );
    lay->addWidget( lbl );
    lay->addSpacing( 10 );
    bt = new KKeyButton( this );
    lay->addWidget( bt, 0 , Qt::AlignHCenter );
    lay->addStretch();
    clear_data();
    connect( bt, SIGNAL( capturedShortcut( const KShortcut& )),
        this, SLOT( capturedShortcut( const KShortcut& )));
    }

void Shortcut_trigger_widget::clear_data()
    {
    bt->setShortcut( KShortcut(), false );
    }

void Shortcut_trigger_widget::capturedShortcut( const KShortcut& s_P )
    {
    if( KKeyChooser::checkGlobalShortcutsConflict( s_P, true, topLevelWidget())
        || KKeyChooser::checkStandardShortcutsConflict( s_P, true, topLevelWidget()))
        return;
    // KHotKeys::Module::changed()
    module->changed();
    bt->setShortcut( s_P, false );
    }

void Shortcut_trigger_widget::set_data( const Shortcut_trigger* data_P )
    {
    if( data_P == NULL )
        {
        clear_data();
        return;
        }
    bt->setShortcut( data_P->shortcut(), false );
    }

Shortcut_trigger* Shortcut_trigger_widget::get_data( Action_data* data_P ) const
    {
    return !bt->shortcut().isNull()
        ? new Shortcut_trigger( data_P, bt->shortcut()) : NULL;
    }

// Shortcut_trigger_dialog

Shortcut_trigger_dialog::Shortcut_trigger_dialog( Shortcut_trigger* trigger_P )
    : KDialogBase( NULL, NULL, true, "", Ok | Cancel ), // CHECKME caption
        trigger( NULL )
    {
    widget = new Shortcut_trigger_widget( this );
    widget->set_data( trigger_P );
    setMainWidget( widget );
    }

Trigger* Shortcut_trigger_dialog::edit_trigger()
    {
    exec();
    return trigger;
    }

void Shortcut_trigger_dialog::accept()
    {
    KDialogBase::accept();
    trigger = widget->get_data( NULL ); // CHECKME NULL ?
    }

// Window_trigger_dialog

Window_trigger_dialog::Window_trigger_dialog( Window_trigger* trigger_P )
    : KDialogBase( NULL, NULL, true, "", Ok | Cancel ), // CHECKME caption
        trigger( NULL )
    {
    widget = new Window_trigger_widget( this );
    widget->set_data( trigger_P );
    setMainWidget( widget );
    }

Trigger* Window_trigger_dialog::edit_trigger()
    {
    exec();
    return trigger;
    }

void Window_trigger_dialog::accept()
    {
    KDialogBase::accept();
    trigger = widget->get_data( NULL ); // CHECKME NULL ?
    }

// Gesture_trigger_dialog

Gesture_trigger_dialog::Gesture_trigger_dialog( Gesture_trigger* trigger_P )
    : KDialogBase( NULL, NULL, true, "", Ok | Cancel ), // CHECKME caption
        _trigger( trigger_P ), _page( NULL )
    {
    _page = new GestureRecordPage( _trigger->gesturecode(),
                                  this, "GestureRecordPage");

    connect(_page, SIGNAL(gestureRecorded(bool)),
            this, SLOT(enableButtonOK(bool)));

    setMainWidget( _page );
    }

Trigger* Gesture_trigger_dialog::edit_trigger()
    {
    if( exec())
        return new Gesture_trigger( NULL, _page->getGesture()); // CHECKME NULL?
    else
        return NULL;
    }


} // namespace KHotKeys

#include "triggers_tab.moc"
