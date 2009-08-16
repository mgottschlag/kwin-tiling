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
class ActionDataGroup;
class ActionList;
class Trigger_list;
class Trigger;


// TODO : code documentation
class KDE_EXPORT ActionData
    : public ActionDataBase
    {

        typedef ActionDataBase base;

    public:

        ActionData(
                ActionDataGroup* parent_P,
                const QString& name_P,
                const QString& comment_P,
                Trigger_list* triggers_P,
                Condition_list* conditions_P,
                ActionList* actions_P);

        virtual ~ActionData();

        /**
         * Visitor pattern
         * @reimp
         */
        virtual void accept(ActionDataVisitor *visitor);
        virtual void accept(ActionDataConstVisitor *visitor) const;

        virtual void update_triggers();

        virtual void execute();

        /**
         * @reimp
         */
        void aboutToBeErased();

        const Trigger_list* triggers() const;
        Trigger_list* triggers();

        const ActionList* actions() const;
        ActionList* actions();

        virtual void add_trigger( Trigger* trigger_P );

        virtual void add_triggers(
            Trigger_list* triggers_P ); // Trigger_list instance will be deleted

        virtual void set_triggers( Trigger_list* triggers_P );

        virtual void add_action( Action* action_P, Action* after_P = 0 );

        virtual void add_actions( ActionList* actions_P,
            Action* after_P = 0 ); // ActionList will be deleted

        virtual void set_actions( ActionList* actions_P );

    private:

        Trigger_list* _triggers;
        ActionList* _actions;

        virtual void doEnable();
        virtual void doDisable();
    };


} // namespace KHotKeys

#endif
