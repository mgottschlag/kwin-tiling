/*
   Copyright (C) 2008 Michael Jansen <kde@michael-jansen.biz>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "simple_action_data.h"

#include "conditions.h"

#include <KDE/KConfigGroup>


namespace KHotKeys {


Simple_action_data_base::Simple_action_data_base(
        Action_data_group* parent_P,
        const QString& name_P,
        const QString& comment_P,
        bool enabled_P )
    : Action_data(
        parent_P,
        name_P,
        comment_P,
        0,
        new Condition_list( "", this ),
        0,
        enabled_P )
    {}


Simple_action_data_base::Simple_action_data_base( KConfigGroup& cfg_P, Action_data_group* parent_P )
    : Action_data( cfg_P, parent_P )
    {}


void Simple_action_data_base::cfg_write( KConfigGroup& cfg_P ) const
    {
    base::cfg_write( cfg_P );
    cfg_P.writeEntry( "Type", "SIMPLE_ACTION_DATA" );
    }


void Simple_action_data_base::set_action( Action* action_P )
    {
    Action_list* tmp = new Action_list( "Simple_action_data" );
    tmp->append( action_P );
    set_actions( tmp );
    }


void Simple_action_data_base::set_trigger( Trigger* trigger_P )
    {
    Trigger_list* tmp = new Trigger_list( "Simple_action" );
    tmp->append( trigger_P );
    set_triggers( tmp );
    }


const Action* Simple_action_data_base::action() const
    {
    if( actions() == 0 || actions()->isEmpty() )
        return 0;
    return actions()->first();
    }


Action* Simple_action_data_base::action()
    {
    if( actions() == 0 || actions()->isEmpty() )
        return 0;
    return actions()->first();
    }


const Trigger* Simple_action_data_base::trigger() const
    {
    if( triggers() == 0 || triggers()->isEmpty() )
        return 0;
    return triggers()->first();
    }


Trigger* Simple_action_data_base::trigger()
    {
    if( triggers() == 0 || triggers()->isEmpty() )
        return 0;
    return triggers()->first();
    }

} // namespace KHotKeys
