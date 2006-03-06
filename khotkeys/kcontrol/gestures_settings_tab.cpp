/****************************************************************************

 KHotKeys
 
 Copyright (C) 2003 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

#define _GESTURES_SETTINGS_TAB_CPP_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gestures_settings_tab.h"

#include <klocale.h>
#include <qcombobox.h>
#include <knuminput.h>
#include <qcheckbox.h>

#include "kcmkhotkeys.h"
#include "windowdef_list_widget.h"

namespace KHotKeys
{

Gestures_settings_tab::Gestures_settings_tab( QWidget* parent_P, const char* name_P )
    : Gestures_settings_tab_ui( parent_P, name_P )
    {
    mouse_button_combo->insertItem( i18n( "Button 2 (middle)" ), 0 );
    mouse_button_combo->insertItem( i18n( "Button 3 (secondary)" ), 1 );
    mouse_button_combo->insertItem( i18n( "Button 4 (often wheel up)" ), 2 );
    mouse_button_combo->insertItem( i18n( "Button 5 (often wheel down)" ), 3 );
    mouse_button_combo->insertItem( i18n( "Button 6 (if available)" ), 4 );
    mouse_button_combo->insertItem( i18n( "Button 7 (if available)" ), 5 );
    mouse_button_combo->insertItem( i18n( "Button 8 (if available)" ), 6 );
    mouse_button_combo->insertItem( i18n( "Button 9 (if available)" ), 7 );
    // KHotKeys::Module::changed()
    connect( mouse_gestures_globally, SIGNAL( clicked()),
        module, SLOT( changed()));
    connect( mouse_button_combo, SIGNAL( activated( int )),
        module, SLOT( changed()));
    connect( timeout_input, SIGNAL( valueChanged( int )),
        module, SLOT( changed()));
    }

void Gestures_settings_tab::read_data()
    {
    mouse_gestures_globally->setChecked( module->gestures_disabled());
    mouse_button_combo->setCurrentIndex( module->gesture_button() - 2 );
    timeout_input->setValue( module->gesture_timeout());
    if( module->gestures_exclude() != NULL )
        windowdef_list_widget->set_data( module->gestures_exclude());
    else
        windowdef_list_widget->clear_data();
    }

void Gestures_settings_tab::write_data() const
    {
    module->set_gestures_disabled( mouse_gestures_globally->isChecked());
    module->set_gesture_button( mouse_button_combo->currentIndex() + 2 );
    module->set_gesture_timeout( timeout_input->value());
    module->set_gestures_exclude( windowdef_list_widget->get_data());
    }

void Gestures_settings_tab::clear_data()
    {
    // "global" tab, not action specific, do nothing
    }
    
} // namespace KHotKeys

#include "gestures_settings_tab.moc"
