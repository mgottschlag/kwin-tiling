/****************************************************************************

 KHotKeys

 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.

****************************************************************************/

#ifndef SIMPLE_ACTION_DATA_H
#define SIMPLE_ACTION_DATA_H

#include "action_data.h"

#include "actions.h"
#include "triggers.h"

namespace KHotKeys {


class KDE_EXPORT SimpleActionData : public Action_data
    {
    typedef Action_data base;

public:

    SimpleActionData(
        ActionDataGroup* parent_P,
        const QString& name_P,
        const QString& comment_P,
        bool enabled_P = true );

    SimpleActionData(
        KConfigGroup& cfg_P,
        ActionDataGroup* parent_P );

    //! The action associated with this hotkey
    virtual const Action* action() const;
    virtual Action* action();

    //! The trigger for this hotkey
    virtual const Trigger* trigger() const;
    virtual Trigger* trigger();

    void set_action( Action* action_P );
    void set_trigger( Trigger* trigger_P );

    virtual void cfg_write( KConfigGroup& cfg_P ) const;
    }; // class SimpleActionData


/**
 * A template adding convenience methods to SimpleActionData.
 */
template< typename T, typename A >
class KDE_EXPORT Simple_action_data
    : public SimpleActionData
    {
        typedef SimpleActionData base;

    public:

        Simple_action_data(
                ActionDataGroup* parent_P,
                const QString& name_P,
                const QString& comment_P,
                bool enabled_P = true )
            : base( parent_P, name_P, comment_P, enabled_P )
            {}

        Simple_action_data(
                KConfigGroup& cfg_P,
                ActionDataGroup* parent_P )
            : base( cfg_P, parent_P )
            {}

        //! The action associated with this hotkey
        const A* action() const;
        A* action();

        //! The trigger for this hotkey
        const T* trigger() const;
        T* trigger();

        void set_action( Action *action_P );
        void set_trigger( Trigger *trigger_P );

        virtual void cfg_write( KConfigGroup& cfg_P ) const;
    };

// ==========================================================================
// TYPEDEFS

//! A keyboard shortcut to dbus call action
typedef Simple_action_data< Shortcut_trigger, Dbus_action > Dbus_shortcut_action_data;

//! A keyboard shortcut to keyboard input action
typedef Simple_action_data< Shortcut_trigger, Keyboard_input_action >
    Keyboard_input_shortcut_action_data;

//! A keyboard shortcut to activate window action
typedef Simple_action_data< Shortcut_trigger, Activate_window_action >
    Activate_window_shortcut_action_data;


// ==========================================================================
// TEMPLATE METHOD DEFINITIONS


template< typename T, typename A >
void Simple_action_data< T, A >::set_action( Action* action_P )
    {
    Q_ASSERT( dynamic_cast<A*>( action_P ) );
    base::set_action(action_P);
    }

template< typename T, typename A >
void Simple_action_data< T, A >::set_trigger( Trigger* trigger_P )
    {
    Q_ASSERT( dynamic_cast<T*>( trigger_P ) );
    base::set_trigger(trigger_P);
    }

template< typename T, typename A >
const A* Simple_action_data< T, A >::action() const
    {
    if( actions() == 0 || actions()->isEmpty() )
        return 0;
    return dynamic_cast< const A* >( actions()->first());
    }

template< typename T, typename A >
A* Simple_action_data< T, A >::action()
    {
    if( actions() == 0 || actions()->isEmpty() )
        return 0;
    return dynamic_cast< A* >(actions()->first());
    }

template< typename T, typename A >
const T* Simple_action_data< T, A >::trigger() const
    {
    if( triggers() == 0 || triggers()->isEmpty() )
        return 0;
    return dynamic_cast< const T* >( triggers()->first());
    }

template< typename T, typename A >
T* Simple_action_data< T, A >::trigger()
    {
    if( triggers() == 0 || triggers()->isEmpty() )
        return 0;
    return dynamic_cast< T* >( triggers()->first());
    }

} // namespace KHotKeys

#endif
