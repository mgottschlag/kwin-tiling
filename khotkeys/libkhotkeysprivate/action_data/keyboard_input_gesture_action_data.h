/****************************************************************************

 KHotKeys

 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.

****************************************************************************/

#ifndef KEYBOARD_INPUT_ACTION_DATA_H
#define KEYBOARD_INPUT_ACTION_DATA_H

#include "action_data.h"


namespace KHotKeys {

class KeyboardInputAction;

class KDE_EXPORT Keyboard_input_gesture_action_data : public ActionData
    {
    typedef ActionData base;

    public:

        Keyboard_input_gesture_action_data(
                ActionDataGroup* parent,
                const QString& name,
                const QString& comment,
                bool enabled = true);

        Keyboard_input_gesture_action_data(
                KConfigGroup& cfg,
                ActionDataGroup* parent);

        const KeyboardInputAction* action() const;

        void set_action( KeyboardInputAction* action );
        enum { NUM_TRIGGERS = 3 }; // needs changing code elsewhere

        using ActionData::set_triggers;

        virtual void cfg_write( KConfigGroup& cfg ) const;
    };

} // namespace KHotKeys

#endif
