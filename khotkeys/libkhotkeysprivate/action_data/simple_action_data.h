/****************************************************************************

 KHotKeys

 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.

****************************************************************************/

#ifndef SIMPLE_ACTION_DATA_H
#define SIMPLE_ACTION_DATA_H

#include "action_data/action_data.h"

#include "actions/actions.h"
#include "triggers/triggers.h"

namespace KHotKeys {


class KDE_EXPORT SimpleActionData : public ActionData
    {
    typedef ActionData base;

public:

    SimpleActionData(
        ActionDataGroup* parent_P,
        const QString& name_P = QString(),
        const QString& comment_P = QString());

    /**
     * Visitor pattern
     * @reimp
     */
    virtual void accept(ActionDataVisitor *visitor);
    virtual void accept(ActionDataConstVisitor *visitor) const;

    //! The action associated with this hotkey
    virtual const Action* action() const;
    virtual Action* action();

    //! The trigger for this hotkey
    virtual const Trigger* trigger() const;
    virtual Trigger* trigger();

    void set_action( Action* action_P );
    void set_trigger( Trigger* trigger_P );

protected:

    void doEnable();

    void doDisable();

    }; // class SimpleActionData


/**
 * A template adding convenience methods to SimpleActionData.
 */
template< typename T, typename A >
class KDE_EXPORT SimpleActionDataHelper
    : public SimpleActionData
    {
        typedef SimpleActionData base;

    public:

        SimpleActionDataHelper(
                ActionDataGroup* parent_P,
                const QString& name_P,
                const QString& comment_P)
            : base( parent_P, name_P, comment_P)
            {}

        //! The action associated with this hotkey
        const A* action() const;
        A* action();

        //! The trigger for this hotkey
        const T* trigger() const;
        T* trigger();

        void set_action( Action *action_P );
        void set_trigger( Trigger *trigger_P );

    };

// ==========================================================================
// TEMPLATE METHOD DEFINITIONS


template< typename T, typename A >
void SimpleActionDataHelper< T, A >::set_action( Action* action_P )
    {
    Q_ASSERT( dynamic_cast<A*>( action_P ) );
    base::set_action(action_P);
    }

template< typename T, typename A >
void SimpleActionDataHelper< T, A >::set_trigger( Trigger* trigger_P )
    {
    Q_ASSERT( dynamic_cast<T*>( trigger_P ) );
    base::set_trigger(trigger_P);
    }

template< typename T, typename A >
const A* SimpleActionDataHelper< T, A >::action() const
    {
    if( actions() == 0 || actions()->isEmpty() )
        return 0;
    return dynamic_cast< const A* >( actions()->first());
    }

template< typename T, typename A >
A* SimpleActionDataHelper< T, A >::action()
    {
    if( actions() == 0 || actions()->isEmpty() )
        return 0;
    return dynamic_cast< A* >(actions()->first());
    }

template< typename T, typename A >
const T* SimpleActionDataHelper< T, A >::trigger() const
    {
    if( triggers() == 0 || triggers()->isEmpty() )
        return 0;
    return dynamic_cast< const T* >( triggers()->first());
    }

template< typename T, typename A >
T* SimpleActionDataHelper< T, A >::trigger()
    {
    if( triggers() == 0 || triggers()->isEmpty() )
        return 0;
    return dynamic_cast< T* >( triggers()->first());
    }

} // namespace KHotKeys

#endif
