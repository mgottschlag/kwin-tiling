/****************************************************************************

 KHotKeys

 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.

****************************************************************************/

#ifndef ACTION_DATA_H
#define ACTION_DATA_H

#include "action_data_base.h"

namespace KHotKeys {

class Action;
class Action_data_group;
class Action_list;
class Trigger_list;
class Trigger;


// TODO : code documentation
class KDE_EXPORT Action_data
    : public Action_data_base
    {

        typedef Action_data_base base;

    public:

        Action_data( Action_data_group* parent_P, const QString& name_P,
            const QString& comment_P, Trigger_list* triggers_P, Condition_list* conditions_P,
            Action_list* actions_P, bool enabled_P = true );

        Action_data( KConfigGroup& cfg_P, Action_data_group* parent_P );

        virtual ~Action_data();

        virtual void update_triggers();

        virtual void cfg_write( KConfigGroup& cfg_P ) const = 0;

        virtual void execute();

        const Trigger_list* triggers() const;

        const Action_list* actions() const;

    protected:
        virtual void add_trigger( Trigger* trigger_P );

        virtual void add_triggers(
            Trigger_list* triggers_P ); // Trigger_list instance will be deleted

        virtual void set_triggers( Trigger_list* triggers_P );

        virtual void add_action( Action* action_P, Action* after_P = 0 );

        virtual void add_actions( Action_list* actions_P,
            Action* after_P = 0 ); // Action_list will be deleted

        virtual void set_actions( Action_list* actions_P );

    private:

        Trigger_list* _triggers;
        Action_list* _actions;

    };


} // namespace KHotKeys

#endif
