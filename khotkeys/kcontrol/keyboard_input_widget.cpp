/****************************************************************************

 KHotKeys
 
 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

#define _KEYBOARD_INPUT_WIDGET_CPP_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "keyboard_input_widget.h"

#include <q3groupbox.h>
#include <qpushbutton.h>
#include <qradiobutton.h>

#include <ktextedit.h>

#include <actions.h>
#include <action_data.h>

#include "windowdef_list_widget.h"
#include "kcmkhotkeys.h"

namespace KHotKeys
{

Keyboard_input_widget::Keyboard_input_widget( QWidget* parent_P, const char* name_P )
    : Keyboard_input_widget_ui( parent_P, name_P )
    {
    clear_data();
    // KHotKeys::Module::changed()
    connect( action_window_radio, SIGNAL( clicked()),
        module, SLOT( changed()));
    connect( active_window_radio, SIGNAL( clicked()),
        module, SLOT( changed()));
    connect( specific_window_radio, SIGNAL( clicked()),
        module, SLOT( changed()));
    connect( keyboard_input_multilineedit, SIGNAL( textChanged()),
        module, SLOT( changed()));
    connect( modify_button, SIGNAL( clicked()),
        module, SLOT( changed()));
    }

void Keyboard_input_widget::clear_data()
    {
    keyboard_input_multilineedit->clear();
    action_window_radio->setChecked( true );
    active_window_radio->setChecked( false );
    specific_window_radio->setChecked( false );
    window_groupbox->setEnabled( false );
    windowdef_list_widget->clear_data();
    }

void Keyboard_input_widget::set_data( const Keyboard_input_action* data_P )
    {
    if( data_P == NULL )
        {
        clear_data();
        return;
        }
    keyboard_input_multilineedit->setPlainText( data_P->input());
    const Windowdef_list* dest_window = data_P->dest_window();
    specific_window_radio->setChecked( dest_window != NULL );
    window_groupbox->setEnabled( dest_window != NULL );
    if( dest_window != NULL )
        windowdef_list_widget->set_data( dest_window );
    else
        {
        windowdef_list_widget->clear_data();
        if( data_P->activeWindow())
            active_window_radio->setChecked( true );
        else
            action_window_radio->setChecked( true );
        }
    }

Keyboard_input_action* Keyboard_input_widget::get_data( Action_data* data_P ) const
    {
    const Windowdef_list* windows = NULL;
    if( specific_window_radio->isChecked())
        windows = windowdef_list_widget->get_data();
    return new Keyboard_input_action( data_P, keyboard_input_multilineedit->toPlainText(),
        windows, active_window_radio->isChecked());
    }

void Keyboard_input_widget::modify_pressed()
    { // CHECKME TODO
    
    // CHECKME klavesy nesmi byt i18n() !!
    }
    
} // namespace KHotKeys

#include "keyboard_input_widget.moc"
