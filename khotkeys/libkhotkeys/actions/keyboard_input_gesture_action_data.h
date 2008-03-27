/****************************************************************************

 KHotKeys

 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.

****************************************************************************/

#ifndef KEYBOARD_INPUT_ACTION_DATA_H
#define KEYBOARD_INPUT_ACTION_DATA_H

#include "action_data.h"


namespace KHotKeys {

class Keyboard_input_action;

class KDE_EXPORT Keyboard_input_gesture_action_data
    : public Action_data
    {
        typedef Action_data base;
    public:
        Keyboard_input_gesture_action_data( Action_data_group* parent_P, const QString& name_P,
            const QString& comment_P, bool enabled_P = true );
        Keyboard_input_gesture_action_data( KConfigGroup& cfg_P, Action_data_group* parent_P );
        const Keyboard_input_action* action() const;
        // CHECKME kontrola, ze se dava jen jedna akce ?
        void set_action( Keyboard_input_action* action_P );
        enum { NUM_TRIGGERS = 3 }; // needs changing code elsewhere
        using Action_data::set_triggers; // make public // CHECKME kontrola poctu?
        virtual void cfg_write( KConfigGroup& cfg_P ) const;
    };

} // namespace KHotKeys

#endif
