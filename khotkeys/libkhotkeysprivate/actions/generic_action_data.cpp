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


void Generic_action_data::cfg_write( KConfigGroup& cfg_P ) const
    {
    base::cfg_write( cfg_P );
    cfg_P.writeEntry( "Type", "GENERIC_ACTION_DATA" );
    }


Generic_action_data::Generic_action_data( Action_data_group* parent_P, const QString& name_P,
    const QString& comment_P, Trigger_list* triggers_P, Condition_list* conditions_P,
    Action_list* actions_P, bool enabled_P )
    : Action_data( parent_P, name_P, comment_P, triggers_P, conditions_P, actions_P, enabled_P )
    {
    }


Generic_action_data::Generic_action_data( KConfigGroup& cfg_P, Action_data_group* parent_P )
    : Action_data( cfg_P, parent_P )
    { // CHECKME do nothing ?
    }


} // namespace KHotKeys
