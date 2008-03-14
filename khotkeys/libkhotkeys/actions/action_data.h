/****************************************************************************

 KHotKeys
 
 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

#ifndef _ACTION_DATA_H_
#define _ACTION_DATA_H_

#include "conditions.h"
#include "triggers.h"
#include "actions.h"

#include <QtCore/QList>

namespace KHotKeys
{

class Action_data_group;

class KDE_EXPORT Action_data_base
    {
    public:
        Action_data_base( Action_data_group* parent_P, const QString& name_P,
            const QString& comment_P, Condition_list* condition_P, bool enabled_P );
        Action_data_base( KConfigGroup& cfg_P, Action_data_group* parent_P );
        virtual ~Action_data_base();
        virtual void cfg_write( KConfigGroup& cfg_P ) const = 0;
        const Condition_list* conditions() const;
        Action_data_group* parent() const;
        void reparent( Action_data_group* new_parent_P );
        virtual void update_triggers() = 0;
        bool conditions_match() const;
        const QString& name() const;
        void set_name( const QString& name_P );
        const QString& comment() const;
        bool enabled( bool ignore_group_P ) const;
        static Action_data_base* create_cfg_read( KConfigGroup& cfg_P, Action_data_group* parent_P );
        static bool cfg_is_enabled( KConfigGroup& cfg_P );
    protected:
        void set_conditions( Condition_list* conditions_P );
    private:
        Action_data_group* _parent;
        Condition_list* _conditions;
        QString _name;
        QString _comment;
        bool _enabled; // is not really important, only used in conf. module and when reading cfg. file
    KHOTKEYS_DISABLE_COPY( Action_data_base );
    };

class KDE_EXPORT Action_data_group
    : public Action_data_base
    {
    public:
        enum system_group_t { SYSTEM_NONE, SYSTEM_MENUENTRIES,
            SYSTEM_ROOT, /* last one*/ SYSTEM_MAX }; // don't remove entries
        Action_data_group( Action_data_group* parent_P, const QString& name_P,
            const QString& comment_P, Condition_list* conditions_P, system_group_t system_group_P,
            bool enabled_P );
        Action_data_group( KConfigGroup& cfg_P, Action_data_group* parent_P );
        virtual ~Action_data_group();
        virtual void update_triggers();
        virtual void cfg_write( KConfigGroup& cfg_P ) const;
        typedef QList< Action_data_base* >::iterator Iterator; // CHECKME neni const :(
        typedef QList< Action_data_base* >::const_iterator ConstIterator; // CHECKME neni const :(
        ConstIterator first_child() const;
        ConstIterator after_last_child() const;
        bool is_system_group() const;
        system_group_t system_group() const;
        using Action_data_base::set_conditions; // make public
    protected:
        QList< Action_data_base* > list;
        system_group_t _system_group; // e.g. menuedit entries, can't be deleted or renamed
        friend class Action_data_base; // CHECKME
        void add_child( Action_data_base* child_P );
        void remove_child( Action_data_base* child_P );
    };
        
// this one represents a "whole" action, i.e. triggers, resulting actions, etc.
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
#if 0
        action_type_t _type;
        static const char* const types[];
#endif
    };        

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
        using Action_data_base::set_conditions; // make public
        using Action_data::add_trigger; // make public
        using Action_data::add_triggers; // make public
        using Action_data::set_triggers; // make public
        using Action_data::add_action; // make public
        using Action_data::add_actions; // make public
        using Action_data::set_actions; // make public
    };

template< typename T, typename A >
class KDE_EXPORT Simple_action_data
    : public Action_data
    {
        typedef Action_data base;
    public:
        Simple_action_data( Action_data_group* parent_P, const QString& name_P,
            const QString& comment_P, bool enabled_P = true );
        Simple_action_data( KConfigGroup& cfg_P, Action_data_group* parent_P );
        const A* action() const;
        const T* trigger() const;
        // CHECKME kontrola, ze se dava jen jedna akce ?
        void set_action( A* action_P );
        void set_trigger( T* trigger_P );
        virtual void cfg_write( KConfigGroup& cfg_P ) const;
    };

class KDE_EXPORT Command_url_shortcut_action_data
    : public Simple_action_data< Shortcut_trigger, Command_url_action >
    {
        typedef Simple_action_data< Shortcut_trigger, Command_url_action > base;
    public:
        Command_url_shortcut_action_data( Action_data_group* parent_P, const QString& name_P,
            const QString& comment_P, bool enabled_P = true );
        Command_url_shortcut_action_data( Action_data_group* parent_P, const QString& name_P,
            const QString& comment_P, const KShortcut& shortcut_P, const QString& command_url_P,
            bool enabled_P = true );    
        Command_url_shortcut_action_data( KConfigGroup& cfg_P, Action_data_group* parent_P );
    };

class KDE_EXPORT Menuentry_shortcut_action_data
    : public Simple_action_data< Shortcut_trigger, Menuentry_action >
    {
        typedef Simple_action_data< Shortcut_trigger, Menuentry_action > base;
    public:
        Menuentry_shortcut_action_data( Action_data_group* parent_P, const QString& name_P,
            const QString& comment_P, bool enabled_P = true );
        Menuentry_shortcut_action_data( Action_data_group* parent_P, const QString& name_P,
            const QString& comment_P, const KShortcut& shortcut_P, const QString& command_url_P,
            bool enabled_P = true );    
        Menuentry_shortcut_action_data( KConfigGroup& cfg_P, Action_data_group* parent_P );
    };

// See simple_action_data.cpp
typedef Simple_action_data< Shortcut_trigger, Dbus_action > Dbus_shortcut_action_data;
typedef Simple_action_data< Shortcut_trigger, Keyboard_input_action >
    Keyboard_input_shortcut_action_data;
typedef Simple_action_data< Shortcut_trigger, Activate_window_action >
    Activate_window_shortcut_action_data;

class KDE_EXPORT Keyboard_input_gesture_action_data
    : public Action_data
    {
        typedef Action_data base;
    public:
        Keyboard_input_gesture_action_data( Action_data_group* parent_P, const QString& name_P,
            const QString& comment_P, bool enabled_P = true );
        Keyboard_input_gesture_action_data( KConfigGroup& cfg_P, Action_data_group* parent_P );
        const Keyboard_input_action* action() const;
        // CHECKME kontrola, ze se dava jen jedna akce ?
        void set_action( Keyboard_input_action* action_P );
        enum { NUM_TRIGGERS = 3 }; // needs changing code elsewhere
        using Action_data::set_triggers; // make public // CHECKME kontrola poctu?
        virtual void cfg_write( KConfigGroup& cfg_P ) const;
    };


template< typename T, typename A >
Simple_action_data< T, A >::Simple_action_data( Action_data_group* parent_P,
    const QString& name_P, const QString& comment_P, bool enabled_P )
    : Action_data( parent_P, name_P, comment_P, 0,
        new Condition_list( "", this ), 0, enabled_P )
    {
    }

template< typename T, typename A >
Simple_action_data< T, A >::Simple_action_data( KConfigGroup& cfg_P, Action_data_group* parent_P )
    : Action_data( cfg_P, parent_P )
    { // CHECKME nothing ?
    } 

template< typename T, typename A >
void Simple_action_data< T, A >::set_action( A* action_P )
    {
    Action_list* tmp = new Action_list( "Simple_action_data" );
    tmp->append( action_P );
    set_actions( tmp );
    }

template< typename T, typename A >
void Simple_action_data< T, A >::set_trigger( T* trigger_P )
    {
    Trigger_list* tmp = new Trigger_list( "Simple_action" );
    tmp->append( trigger_P );
    set_triggers( tmp );
    }

template< typename T, typename A >
const A* Simple_action_data< T, A >::action() const
    {
    if( actions() == 0 || actions()->isEmpty() ) // CHECKME tohle poradne zkontrolovat
        return 0;
    return static_cast< A* >( const_cast< Action_list* >( actions())->first());
    }

template< typename T, typename A >
const T* Simple_action_data< T, A >::trigger() const
    {
    if( triggers() == 0 || triggers()->isEmpty() ) // CHECKME tohle poradne zkontrolovat
        return 0;
    return static_cast< T* >( const_cast< Trigger_list* >( triggers())->first());
    }


} // namespace KHotKeys

#endif
