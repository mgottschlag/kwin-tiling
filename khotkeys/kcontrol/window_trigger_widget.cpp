/****************************************************************************

 KHotKeys
 
 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

#define _WINDOW_TRIGGER_WIDGET_CPP_



#include "window_trigger_widget.h"

#include <QCheckBox>

#include <triggers.h>
#include <actions.h> 
#include <action_data.h>

#include "windowdef_list_widget.h"
#include "kcmkhotkeys.h"

namespace KHotKeys
{

Window_trigger_widget::Window_trigger_widget( QWidget* parent_P, const char* name_P )
    : Window_trigger_widget_ui( parent_P, name_P )
    {
    clear_data();
    // KHotKeys::Module::changed()
    connect( window_appears_checkbox, SIGNAL( clicked()),
        module, SLOT( changed()));
    connect( window_disappears_checkbox, SIGNAL( clicked()),
        module, SLOT( changed()));
    connect( window_activates_checkbox, SIGNAL( clicked()),
        module, SLOT( changed()));
    connect( window_deactivates_checkbox, SIGNAL( clicked()),
        module, SLOT( changed()));
    } 

void Window_trigger_widget::clear_data()
    {
    windowdef_list_widget->clear_data();
    window_appears_checkbox->setChecked( false );
    window_disappears_checkbox->setChecked( false );
    window_activates_checkbox->setChecked( false );
    window_deactivates_checkbox->setChecked( false );
    }
    
void Window_trigger_widget::set_data( const Window_trigger* trigger_P )
    {
    if( trigger_P == NULL )
        {
        clear_data();
        return;
        }
    windowdef_list_widget->set_data( trigger_P->windows());
    window_appears_checkbox->setChecked(
        trigger_P->triggers_on( Window_trigger::WINDOW_APPEARS ));
    window_disappears_checkbox->setChecked(
        trigger_P->triggers_on( Window_trigger::WINDOW_DISAPPEARS ));
    window_activates_checkbox->setChecked(
        trigger_P->triggers_on( Window_trigger::WINDOW_ACTIVATES ));
    window_deactivates_checkbox->setChecked(
        trigger_P->triggers_on( Window_trigger::WINDOW_DEACTIVATES ));
    }
    
Window_trigger* Window_trigger_widget::get_data( Action_data* data_P ) const
    {
    return new Window_trigger( data_P, windowdef_list_widget->get_data(),
        ( window_appears_checkbox->isChecked() ? Window_trigger::WINDOW_APPEARS : 0 )
        | ( window_disappears_checkbox->isChecked() ? Window_trigger::WINDOW_DISAPPEARS : 0 )
        | ( window_activates_checkbox->isChecked() ? Window_trigger::WINDOW_ACTIVATES : 0 )
        | ( window_deactivates_checkbox->isChecked() ? Window_trigger::WINDOW_DEACTIVATES : 0 ));
    }

} // namespace KHotKeys

#include "window_trigger_widget.moc"
