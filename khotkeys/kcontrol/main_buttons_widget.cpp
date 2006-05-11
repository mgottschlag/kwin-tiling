/****************************************************************************

 KHotKeys
 
 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

#define MAIN_BUTTONS_WIDGET_CPP

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "main_buttons_widget.h"

#include <QPushButton>

#include "kcmkhotkeys.h"

namespace KHotKeys
{

Main_buttons_widget::Main_buttons_widget( QWidget* parent_P, const char* name_P )
    : Main_buttons_widget_ui( parent_P, name_P )
    {
    connect( new_action_button, SIGNAL( clicked()), SIGNAL( new_action_pressed()));
    connect( new_action_group_button, SIGNAL( clicked()), SIGNAL( new_action_group_pressed()));
    connect( delete_action_button, SIGNAL( clicked()), SIGNAL( delete_action_pressed()));
    connect( global_settings_button, SIGNAL( clicked()), SIGNAL( global_settings_pressed()));
    enable_delete( false );
    // KHotKeys::Module::changed()
    connect( new_action_button, SIGNAL( clicked()),
        module, SLOT( changed()));
    connect( new_action_group_button, SIGNAL( clicked()),
        module, SLOT( changed()));
    connect( delete_action_button, SIGNAL( clicked()),
        module, SLOT( changed()));
    setMaximumHeight( sizeHint().height()); // it gets too high and I have no idea why
    }

void Main_buttons_widget::enable_delete( bool enable_P )
    {
    delete_action_button->setEnabled( enable_P );
    }
        
} // namespace KHotKeys

#include "main_buttons_widget.moc"
