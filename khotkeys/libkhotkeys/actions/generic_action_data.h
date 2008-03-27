/****************************************************************************

 KHotKeys

 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.

****************************************************************************/

#ifndef GENERIC_ACTION_DATA_H
#define GENERIC_ACTION_DATA_H

#include "action_data.h"


namespace KHotKeys {

class Action_data_group;

class KDE_EXPORT Generic_action_data
    : public Action_data
    {
    typedef Action_data base;

    public:

        Generic_action_data( Action_data_group* parent_P, const QString& name_P,
            const QString& comment_P, Trigger_list* triggers_P, Condition_list* conditions_P,
            Action_list* actions_P, bool enabled_P = true );

        Generic_action_data( KConfigGroup& cfg_P, Action_data_group* parent_P );

        virtual void cfg_write( KConfigGroup& cfg_P ) const;

        // CHECKME: Why this?
        using Action_data_base::set_conditions; // make public
        using Action_data::add_trigger; // make public
        using Action_data::add_triggers; // make public
        using Action_data::set_triggers; // make public
        using Action_data::add_action; // make public
        using Action_data::add_actions; // make public
        using Action_data::set_actions; // make public
    };

} // namespace KHotKeys

#endif
