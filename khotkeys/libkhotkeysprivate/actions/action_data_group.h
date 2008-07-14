/****************************************************************************

 KHotKeys

 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.

****************************************************************************/

#ifndef ACTION_DATA_GROUP_H
#define ACTION_DATA_GROUP_H

#include "action_data_base.h"

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
            const QString& name_P,
            const QString& comment_P,
            Condition_list* conditions_P = 0,
            system_group_t system_group_P = SYSTEM_NONE,
            bool enabled_P = false );

        ActionDataGroup( KConfigGroup& cfg_P, ActionDataGroup* parent_P );
        virtual ~ActionDataGroup();
        virtual void update_triggers();
        virtual void cfg_write( KConfigGroup& cfg_P ) const;
        typedef QList< ActionDataBase* >::iterator Iterator; // CHECKME neni const :(
        typedef QList< ActionDataBase* >::const_iterator ConstIterator; // CHECKME neni const :(
        ConstIterator first_child() const;
        ConstIterator after_last_child() const;
        int child_count() const;


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
        //@}

        // CHECKME : Why this?
        using ActionDataBase::set_conditions; // make public

    protected:

        // TODO : Make this unnessecary
        friend class ::KHotkeysModel;

        //! The childs
        QList< ActionDataBase* > list;

        //! System group type
        system_group_t _system_group; // e.g. menuedit entries, can't be deleted or renamed

        friend class ActionDataBase;

        //! Add a child to this collection
        void add_child( ActionDataBase* child_P );

        //! Remove a child from this collection
        void remove_child( ActionDataBase* child_P );

    };



} // namespace KHotKeys

#endif
