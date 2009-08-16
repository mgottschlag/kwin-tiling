/****************************************************************************

 KHotKeys
 
 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

#include "generic_action_data.h"

#include "action_data/action_data_visitor.h"
#include "actions/actions.h"

#include <kconfiggroup.h>


namespace KHotKeys
{


Generic_action_data::Generic_action_data(
        ActionDataGroup* parent,
        const QString& name,
        const QString& comment,
        Trigger_list* triggers,
        Condition_list* conditions,
        ActionList* actions)
    :   ActionData(parent, name, comment, triggers, conditions, actions)
    {
    }


Generic_action_data::~Generic_action_data()
    {}


void Generic_action_data::accept(ActionDataVisitor *visitor)
    {
    visitor->visitGenericActionData(this);
    }


void Generic_action_data::accept(ActionDataConstVisitor *visitor) const
    {
    visitor->visitGenericActionData(this);
    }


} // namespace KHotKeys
