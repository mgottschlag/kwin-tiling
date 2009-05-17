/****************************************************************************

 KHotKeys

 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.

****************************************************************************/

#ifndef GENERIC_ACTION_DATA_H
#define GENERIC_ACTION_DATA_H

#include "action_data.h"


namespace KHotKeys {

class ActionDataGroup;

class KDE_EXPORT Generic_action_data
    : public ActionData
    {
    typedef ActionData base;

    public:

        Generic_action_data(
                ActionDataGroup* parent,
                const QString& name,
                const QString& comment,
                Trigger_list* triggers,
                Condition_list* conditions,
                ActionList* actions);

        Generic_action_data(const KConfigGroup& cfg_P, ActionDataGroup* parent_P );

        ~Generic_action_data();

        /**
         * Visitor pattern
         * @reimp
         */
        virtual void accept(ActionDataVisitor *visitor) const;

        // CHECKME: Why this?
        using ActionDataBase::set_conditions; // make public
        using ActionData::add_trigger; // make public
        using ActionData::add_triggers; // make public
        using ActionData::set_triggers; // make public
        using ActionData::add_action; // make public
        using ActionData::add_actions; // make public
        using ActionData::set_actions; // make public
    };

} // namespace KHotKeys

#endif
