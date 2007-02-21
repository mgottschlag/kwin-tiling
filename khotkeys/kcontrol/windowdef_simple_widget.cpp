/****************************************************************************

 KHotKeys
 
 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

#define _WINDOWDEF_SIMPLE_WIDGET_CPP_



#include "windowdef_simple_widget.h"

#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <kdebug.h>

#include <windows.h>

#include "windowdef_list_widget.h"
#include "kcmkhotkeys.h"
#include "windowselector.h"

namespace KHotKeys
{

Windowdef_simple_widget::Windowdef_simple_widget( QWidget* parent_P, const char* name_P )
    : Windowdef_simple_widget_ui( parent_P, name_P )
    {
    window_title_lineedit->setEnabled( false );
    window_class_lineedit->setEnabled( false );
    window_role_lineedit->setEnabled( false );
    connect( autodetect_button, SIGNAL( clicked()), SLOT( autodetect_clicked()));
    clear_data();
    // KHotKeys::Module::changed()
    connect( window_title_combo, SIGNAL( activated( int )),
        module, SLOT( changed()));
    connect( window_title_lineedit, SIGNAL( textChanged( const QString& )),
        module, SLOT( changed()));
    connect( window_class_combo, SIGNAL( activated( int )),
        module, SLOT( changed()));
    connect( window_class_lineedit, SIGNAL( textChanged( const QString& )),
        module, SLOT( changed()));
    connect( window_role_combo, SIGNAL( activated( int )),
        module, SLOT( changed()));
    connect( window_role_lineedit, SIGNAL( textChanged( const QString& )),
        module, SLOT( changed()));
    connect( type_normal_checkbox, SIGNAL( clicked()),
        module, SLOT( changed()));
    connect( type_dialog_checkbox, SIGNAL( clicked()),
        module, SLOT( changed()));
    connect( type_dock_checkbox, SIGNAL( clicked()),
        module, SLOT( changed()));
    connect( type_desktop_checkbox, SIGNAL( clicked()),
        module, SLOT( changed()));
    connect( comment_lineedit, SIGNAL( textChanged( const QString& )),
        module, SLOT( changed()));
    }

void Windowdef_simple_widget::clear_data()
    {
    comment_lineedit->clear();
    window_title_combo->setCurrentIndex( 0 );
    window_title_lineedit->clear();    
    window_title_lineedit->setEnabled( false );
    window_class_combo->setCurrentIndex( 0 );
    window_class_lineedit->clear();
    window_class_lineedit->setEnabled( false );
    window_role_combo->setCurrentIndex( 0 );
    window_role_lineedit->clear();
    window_role_lineedit->setEnabled( false );
    type_normal_checkbox->setChecked( true );
    type_dialog_checkbox->setChecked( true );
    type_dock_checkbox->setChecked( false );
//    type_tool_checkbox->setChecked( false );
//    type_menu_checkbox->setChecked( false );
    type_desktop_checkbox->setChecked( false );
    }
    
void Windowdef_simple_widget::set_data( const Windowdef_simple* data_P )
    {
    if( data_P == NULL )
        {
        clear_data();
        return;
        }
    comment_lineedit->setText( data_P->comment());
    window_title_combo->setCurrentIndex( data_P->title_match_type());
    window_title_lineedit->setText( data_P->title());    
    window_title_lineedit->setEnabled( window_title_combo->currentIndex() != 0 );
    window_class_combo->setCurrentIndex( data_P->wclass_match_type());
    window_class_lineedit->setText( data_P->wclass());
    window_class_lineedit->setEnabled( window_class_combo->currentIndex() != 0 );
    window_role_combo->setCurrentIndex( data_P->role_match_type());
    window_role_lineedit->setText( data_P->role());
    window_role_lineedit->setEnabled( window_role_combo->currentIndex() != 0 );
    type_normal_checkbox->setChecked( data_P->type_match( Windowdef_simple::WINDOW_TYPE_NORMAL ));
    type_dialog_checkbox->setChecked( data_P->type_match( Windowdef_simple::WINDOW_TYPE_DIALOG ));
    type_dock_checkbox->setChecked( data_P->type_match( Windowdef_simple::WINDOW_TYPE_DOCK ));
//    type_tool_checkbox->setChecked( data_P->type_match( Windowdef_simple::WINDOW_TYPE_TOOL ));
//    type_menu_checkbox->setChecked( data_P->type_match( Windowdef_simple::WINDOW_TYPE_MENU ));
    type_desktop_checkbox->setChecked( 
        data_P->type_match( Windowdef_simple::WINDOW_TYPE_DESKTOP ));
    }
    
Windowdef_simple* Windowdef_simple_widget::get_data() const
    {
    return new Windowdef_simple( comment_lineedit->text(), window_title_lineedit->text(),
        static_cast< Windowdef_simple::substr_type_t >( window_title_combo->currentIndex()),
        window_class_lineedit->text(),
        static_cast< Windowdef_simple::substr_type_t >( window_class_combo->currentIndex()),
        window_role_lineedit->text(),
        static_cast< Windowdef_simple::substr_type_t >( window_role_combo->currentIndex()),
            ( type_normal_checkbox->isChecked() ? Windowdef_simple::WINDOW_TYPE_NORMAL : 0 )
            | ( type_dialog_checkbox->isChecked() ? Windowdef_simple::WINDOW_TYPE_DIALOG : 0 )
            | ( type_dock_checkbox->isChecked() ? Windowdef_simple::WINDOW_TYPE_DOCK : 0 )
//            | ( type_tool_checkbox->isChecked() ? Windowdef_simple::WINDOW_TYPE_TOOL : 0 )
//            | ( type_menu_checkbox->isChecked() ? Windowdef_simple::WINDOW_TYPE_MENU : 0 )
            | ( type_desktop_checkbox->isChecked() ? Windowdef_simple::WINDOW_TYPE_DESKTOP : 0 )
        );
    }
    
void Windowdef_simple_widget::window_role_combo_changed( int item_P )
    { // CHECKME nebo je connect() na highlighted() ?
    window_role_lineedit->setEnabled( item_P != 0 );
    }
    
void Windowdef_simple_widget::window_class_combo_changed( int item_P )
    {
    window_class_lineedit->setEnabled( item_P != 0 );
    }
    
void Windowdef_simple_widget::window_title_combo_changed( int item_P )
    {
    window_title_lineedit->setEnabled( item_P != 0 );
    }
    
void Windowdef_simple_widget::set_autodetect( QObject* obj_P, const char* slot_P )
    {
    disconnect( SIGNAL( autodetect_signal()));
    if( obj_P != NULL )
        connect( this, SIGNAL( autodetect_signal()), obj_P, slot_P );
    }
    
void Windowdef_simple_widget::autodetect_clicked()
    {
    emit autodetect_signal();
    autodetect();
    }
    
void Windowdef_simple_widget::autodetect()
    {
    WindowSelector* sel = new WindowSelector( this, SLOT( autodetect_window_selected( WId )));
    sel->select();
    }

void Windowdef_simple_widget::autodetect_window_selected( WId window )
    {
    if( window )
        {
        Window_data data( window );
        window_title_lineedit->setText( data.title );
        window_role_lineedit->setText( data.role );
        window_class_lineedit->setText( data.wclass );
        type_normal_checkbox->setChecked( data.type == NET::Normal );
        type_dialog_checkbox->setChecked( data.type == NET::Dialog );
        type_dock_checkbox->setChecked( data.type == NET::Dock );
//      type_tool_checkbox->setChecked( data_P->type_match( Windowdef_simple::WINDOW_TYPE_TOOL ));
//      type_menu_checkbox->setChecked( data_P->type_match( Windowdef_simple::WINDOW_TYPE_MENU ));
        type_desktop_checkbox->setChecked( data.type == NET::Desktop );
        }
    }

} // namespace KHotKeys

#include "windowdef_simple_widget.moc"
