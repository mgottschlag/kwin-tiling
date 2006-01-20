/****************************************************************************

 KHotKeys
 
 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

#ifndef _KEYBOARD_INPUT_WIDGET_H_
#define _KEYBOARD_INPUT_WIDGET_H_

#include <keyboard_input_widget_ui.h>

namespace KHotKeys
{

class Keyboard_input_action;
class Action_data;

class Keyboard_input_widget
    : public Keyboard_input_widget_ui
    {
    Q_OBJECT
    public:
        Keyboard_input_widget( QWidget* parent_P = NULL, const char* name_P = NULL );
        void set_data( const Keyboard_input_action* data_P );
        Keyboard_input_action* get_data( Action_data* data_P ) const;
    public Q_SLOTS:
        void clear_data();
    protected Q_SLOTS:
        virtual void modify_pressed(); 
    };

typedef Keyboard_input_widget Keyboard_input_tab;

//***************************************************************************
// Inline
//***************************************************************************

} // namespace KHotKeys

#endif
