/****************************************************************************

 KHotKeys

 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.

****************************************************************************/

#ifndef ACTION_DATA_BASE_H
#define ACTION_DATA_BASE_H

#include "kdemacros.h"

#include <QtCore/QString>


class KConfigGroup;


namespace KHotKeys {

class Action_data_group;
class Condition_list;

/**
 * Base class for all actions.
 */
class KDE_EXPORT Action_data_base
    {
    Q_DISABLE_COPY( Action_data_base )

    public:

        /**
         * Create a action data base object.
         *
         * \param parent_P  A Action_data_group or 0. If provided this action is
         *        registered with the group.
         * \param name_P    Name for the object.
         * \param comment_P Comment for the object.
         * \param condition_P Conditions for the object or 0
         * \param enabled_P Is the action enabled?
         */
        Action_data_base( 
            Action_data_group* parent_P,
            const QString& name_P,
            const QString& comment_P,
            Condition_list* condition_P,
            bool enabled_P );

        /**
         * Read the setting for the \c Action_data_base object from the \a
         * cfg_P configuration object.
         *
         * \param cfg_P KConfigGroup to read from
         * \param parent_P  A Action_data_group or 0. If provided the object is
         *        registered with the group.
         */
        Action_data_base( KConfigGroup& cfg_P, Action_data_group* parent_P );

        /**
         * Destructor
         */
        virtual ~Action_data_base();

        /**
         * Write the this action  to the \a cfg_P configuration
         * object.
         */
        virtual void cfg_write( KConfigGroup& cfg_P ) const = 0;

        /**
         * Get the conditions for this action or 0 if the action has no
         * conditions.
         */
        const Condition_list* conditions() const;

        /**
         * Get the \c Action_data_group this object belongs to or 0 if it
         * does not belong to a group.
         */
        Action_data_group* parent() const;

        /**
         * Put this action into the group \a new_parent_P. If the action
         * currently belongs to a group, remove it first. If \a new_parent_P
         * is 0 the action will be removed from its group.
         *
         * \param new_parent_P New group or 0.
         */
        void reparent( Action_data_group* new_parent_P );

        /**
         *
         */
        virtual void update_triggers() = 0;

        /**
         * Check if the conditions match.
         */
        bool conditions_match() const;

        //@{
        /**
         * Name for the action
         */
        QString name() const;
        void set_name( const QString& name_P );
        //@}


        //@{
        /**
         * A comment for this action.
         */
        QString comment() const;
        void set_comment( const QString &comment );
        //@}

        //@{
        /**
         * Is that action enabled
         */
        bool enabled( bool ignore_group_P = false ) const;
        void set_enabled( bool enabled );
        //@}

        /**
         * Factory method.
         */
        static Action_data_base* create_cfg_read( KConfigGroup& cfg_P, Action_data_group* parent_P );

        /**
         * See if it the config group is enabled.
         */
        static bool cfg_is_enabled( KConfigGroup& cfg_P );

    protected:

        void set_conditions( Condition_list* conditions_P );

    private:
        Action_data_group* _parent;
        Condition_list* _conditions;
        QString _name;
        QString _comment;
        bool _enabled; // is not really important, only used in conf. module and when reading cfg. file
    };


} // namespace KHotKeys

#endif // ACTION_DATA_BASE_H
