/****************************************************************************

 KHotKeys

 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.

****************************************************************************/

#ifndef ACTION_DATA_BASE_H
#define ACTION_DATA_BASE_H

#include "kdemacros.h"

#include <QtCore/QObject>
#include <QtCore/QString>


class KConfigGroup;


namespace KHotKeys {

class ActionDataGroup;
class ActionDataVisitor;
class ActionDataConstVisitor;
class Condition_list;


/**
 * Base class for all actions.
 */
class KDE_EXPORT ActionDataBase : public QObject
    {
    Q_OBJECT

    Q_DISABLE_COPY( ActionDataBase )

    public:

        /**
         * Create a action data base object.
         *
         * \param parent  A ActionDataGroup or 0. If provided this action is
         *        registered with the group.
         * \param name    Name for the object.
         * \param comment Comment for the object.
         * \param condition Conditions for the object or 0
         */
        ActionDataBase( 
            ActionDataGroup* parent,
            const QString& name,
            const QString& comment,
            Condition_list* condition);

        /**
         * Destructor
         */
        virtual ~ActionDataBase();

        /**
         * Visitor pattern
         */
        virtual void accept(ActionDataVisitor *visitor);
        virtual void accept(ActionDataConstVisitor *visitor) const;

        /**
         * Get the conditions for this action or 0 if the action has no
         * conditions.
         */
        const Condition_list* conditions() const;
        Condition_list* conditions();

        /**
         * Prepare this action for complete removal.
         */
        virtual void aboutToBeErased() = 0;

        /**
         * Get the \c ActionDataGroup this object belongs to or 0 if it
         * does not belong to a group.
         */
        ActionDataGroup* parent() const;

        /**
         * \todo document this method
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
        void set_name( const QString& name );
        //@}


        //@{
        /**
         * A comment for this action.
         */
        QString comment() const;
        void set_comment( const QString &comment );
        //@}

        /**
         */
        void setImportId(const QString&);
        QString importId() const;

        /**
         */
        void setAllowMerging(bool);
        bool allowMerging() const;

        /**
         * Is the action enabled?
         *
         * The action has different states.
         *
         * Absolute State:  Has the action an enabled/disabled state?
         * Effective State: Even if the action is enabled it's effective state
         *                  is disabled if it's parent is disabled.
         */
        enum IgnoreParent
            {
            Ignore,
            DontIgnore
            };
        bool isEnabled(IgnoreParent ip = DontIgnore) const;

        /**
         * Enable the action.
         */
        void enable();
        void disable();
        //@}

        /**
         * See if it the config group is enabled.
         */
        static bool cfg_is_enabled(const KConfigGroup& cfg);

        //! Set the list of condition for this element
        void set_conditions(Condition_list* conditions);

    protected:

        virtual void doEnable() = 0;

        virtual void doDisable() = 0;

    private:

        friend class ActionDataGroup;

        //! The parent or NULL
        ActionDataGroup* _parent;

        //! List of conditions for this element
        Condition_list* _conditions;

        //! Name for element
        QString _name;

        //! The comment for this element
        QString _comment;

        //! If this element is enabled
        bool _enabled; // is not really important, only used in conf. module and when reading cfg. file

        //! The importId from the file we were imported
        QString _importId;

        //! Is it allowed to merge this directory with others. Used when
        //exporting and importig the directory
        bool _allowMerging;

    };


} // namespace KHotKeys

#endif // ACTION_DATA_BASE_H
