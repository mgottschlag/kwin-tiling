/**
 * Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>
 * Copyright (C) 2009 Michael Jansen <kde@michael-jansen.biz>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB. If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "keyboard_input_gesture_action_data.h"

#include "actions/actions.h"
#include "conditions/conditions.h"
#include "conditions/conditions_list.h"

#include <KDE/KConfigGroup>

namespace KHotKeys {

Keyboard_input_gesture_action_data::Keyboard_input_gesture_action_data(
        ActionDataGroup* parent,
        const QString& name,
        const QString& comment,
        bool enabled)
    :   ActionData(
            parent,
            name,
            comment,
            0,
            new Condition_list( "", this ),
            0,
            enabled)
    {}



Keyboard_input_gesture_action_data::Keyboard_input_gesture_action_data(
        KConfigGroup& cfg,
        ActionDataGroup* parent)
    :   ActionData(cfg, parent)
    {}


const KeyboardInputAction* Keyboard_input_gesture_action_data::action() const
    {
    if( actions() == 0 || actions()->isEmpty() ) // CHECKME tohle poradne zkontrolovat
        return 0;
    return static_cast< KeyboardInputAction* >( const_cast< ActionList* >( actions())->first());
    }


void Keyboard_input_gesture_action_data::cfg_write( KConfigGroup& cfg ) const
    {
    base::cfg_write( cfg );
    cfg.writeEntry( "Type", "KEYBOARD_INPUT_GESTURE_ACTION_DATA" );
    }


void Keyboard_input_gesture_action_data::set_action( KeyboardInputAction* action )
    {
    ActionList* tmp = new ActionList( "Keyboard_input_gesture_action_data" );
    tmp->append( action );
    set_actions( tmp );
    }


} // namespace KHotKeys
