/****************************************************************************

 KHotKeys

 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.

****************************************************************************/

#ifndef ACTION_DATA_GROUP_H
#define ACTION_DATA_GROUP_H

#include "action_data_base.h"
#include "actions/actions.h"
#include "triggers/triggers.h"

#include "QtCore/QList"

#include "kdemacros.h"


class KHotkeysModel;

namespace KHotKeys {


/**
 * A group of \c ActionDataBase objects
 *
 * # The group can contain actions or action groups.
 * # The group has its own list of conditions. These conditions apply to all children.
 */
class KDE_EXPORT ActionDataGroup
    : public ActionDataBase
    {
    Q_OBJECT

    public:

        enum system_group_t {
            SYSTEM_NONE,             //!< TODO
            SYSTEM_MENUENTRIES,      //!< Shortcuts for menu entries.
            SYSTEM_ROOT,             //!< TODO
            /* last one*/ SYSTEM_MAX //!< End of enum marker
            }; // don't remove entries

        /**
         * Create a \c ActionDataGroup object.
         *
         * \param parent_P  A ActionDataGroup or 0. If provided this action is
         *        registered with the group.
         * \param name_P    Name for the object.
         * \param comment_P Comment for the object.
         * \param condition_P Conditions for the object or 0
         * \param system_group_t Group type
         * \param enabled_P Is the action enabled?
         */
        ActionDataGroup( 
            ActionDataGroup* parent_P,
            const QString& name_P = QString(),
            const QString& comment_P = QString(),
            Condition_list* conditions_P = NULL,
            system_group_t system_group_P = SYSTEM_NONE);

        virtual ~ActionDataGroup();

        /**
         * Visitor pattern
         */
        virtual void accept(ActionDataVisitor *visitor);
        virtual void accept(ActionDataConstVisitor *visitor) const;

        virtual void update_triggers();

        /**
         * What kind of actions are allowed for this group?
         */
        Action::ActionTypes allowedActionTypes() const;

        /**
         * What kind of trigger are allowed for this group?
         */
        Trigger::TriggerTypes allowedTriggerTypes() const;

        /**
         * Get a shallow copy of the list of children.
         */
        const QList<ActionDataBase*> children() const;

        /**
         * Number of childrens.
         */
        int size() const;

        /**
         * @reimp
         */
        void aboutToBeErased();

        /**
         * Is this a system group?
         *
         * @{
         */
        bool is_system_group() const;
        system_group_t system_group() const;
        void set_system_group(system_group_t group);
        //@}

        // CHECKME : Why this?
        using ActionDataBase::set_conditions; // make public

        //! Add a child to this collection
        void add_child( ActionDataBase* child_P, int position );

        //! Add a child to this collection
        void add_child( ActionDataBase* child_P );

        //! Remove a child from this collection
        void remove_child( ActionDataBase* child_P );

    Q_SIGNALS:

        void stateChanged(bool isEnabled);

    protected:

        //! The children
        QList< ActionDataBase* > _list;

        //! System group type
        system_group_t _system_group; // e.g. menuedit entries, can't be deleted or renamed

        virtual void doEnable();

        virtual void doDisable();

    };



} // namespace KHotKeys

#endif
