/****************************************************************************

 KHotKeys
 
 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

#include "action_data.h"
#include "actions.h"

#include <kconfiggroup.h>

namespace KHotKeys
{


void Keyboard_input_gesture_action_data::set_action( Keyboard_input_action* action_P )
    {
    Action_list* tmp = new Action_list( "Keyboard_input_gesture_action_data" );
    tmp->append( action_P );
    set_actions( tmp );
    }


const Keyboard_input_action* Keyboard_input_gesture_action_data::action() const
    {
    if( actions() == 0 || actions()->isEmpty() ) // CHECKME tohle poradne zkontrolovat
        return 0;
    return static_cast< Keyboard_input_action* >( const_cast< Action_list* >( actions())->first());
    }


void Keyboard_input_gesture_action_data::cfg_write( KConfigGroup& cfg_P ) const
    {
    base::cfg_write( cfg_P );
    cfg_P.writeEntry( "Type", "KEYBOARD_INPUT_GESTURE_ACTION_DATA" );
    }


Keyboard_input_gesture_action_data::Keyboard_input_gesture_action_data(
    Action_data_group* parent_P, const QString& name_P, const QString& comment_P, bool enabled_P )
    : Action_data( parent_P, name_P, comment_P, 0,
        new Condition_list( "", this ), 0, enabled_P )
    {
    }


Keyboard_input_gesture_action_data::Keyboard_input_gesture_action_data( KConfigGroup& cfg_P,
    Action_data_group* parent_P )
    : Action_data( cfg_P, parent_P )
    { // CHECKME nothing ?
    }


} // namespace KHotKeys
