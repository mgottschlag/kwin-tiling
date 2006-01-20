/****************************************************************************

 KHotKeys
 
 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

#ifndef MAIN_BUTTONS_WIDGET_H
#define MAIN_BUTTONS_WIDGET_H

#include <main_buttons_widget_ui.h>

namespace KHotKeys
{

class Main_buttons_widget
    : public Main_buttons_widget_ui
    {
    Q_OBJECT
    public:
        Main_buttons_widget( QWidget* parent_P = NULL, const char* name_P = NULL );
        void enable_delete( bool enable_P );
    Q_SIGNALS:
        void new_action_pressed();
        void new_action_group_pressed();
        void delete_action_pressed();
        void global_settings_pressed();
    };
    
//***************************************************************************
// Inline
//***************************************************************************

} // namespace KHotKeys

#endif
