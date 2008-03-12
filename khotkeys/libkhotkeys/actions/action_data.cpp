/****************************************************************************

 KHotKeys
 
 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

#define _ACTION_DATA_CPP_

#include "action_data.h"

#include <kconfig.h>
#include <kconfiggroup.h>

#include "actions.h"

namespace KHotKeys
{

// Action_data_base

Action_data_base::Action_data_base( Action_data_group* parent_P, const QString& name_P,
    const QString& comment_P, Condition_list* conditions_P, bool enabled_P )
    : _parent( parent_P ), _conditions( conditions_P ), _name( name_P ), _comment( comment_P ),
        _enabled( enabled_P )
    {
    if( parent())
        parent()->add_child( this );
    if( _conditions != 0 )
        _conditions->set_data( this );
    }
    
Action_data_base::Action_data_base( KConfigGroup& cfg_P, Action_data_group* parent_P )
    : _parent( parent_P )
    {
    _name = cfg_P.readEntry( "Name" );
    _comment = cfg_P.readEntry( "Comment" );
    _enabled = cfg_P.readEntry( "Enabled", true);
    KConfigGroup conditionsConfig( cfg_P.config(), cfg_P.name() + "Conditions" );
    _conditions = new Condition_list( conditionsConfig, this );
    if( parent())
        parent()->add_child( this );
    }

Action_data_base::~Action_data_base()
    {
//    kDebug( 1217 ) << "~Action_data_base() :" << this;
    if( parent())
        parent()->remove_child( this );
    delete _conditions;
    }
    
void Action_data_base::cfg_write( KConfigGroup& cfg_P ) const
    {
    cfg_P.writeEntry( "Type", "ERROR" ); // derived classes should call with their type
    cfg_P.writeEntry( "Name", name());
    cfg_P.writeEntry( "Comment", comment());
    cfg_P.writeEntry( "Enabled", enabled( true ));
    KConfigGroup conditionsConfig( cfg_P.config(), cfg_P.name() + "Conditions" );
    assert( conditions() != 0 );
    conditions()->cfg_write( conditionsConfig );
    }


Action_data_base* Action_data_base::create_cfg_read( KConfigGroup& cfg_P, Action_data_group* parent_P )
    {
    QString type = cfg_P.readEntry( "Type" );
    if( type == "ACTION_DATA_GROUP" )
        {
        if( cfg_P.readEntry( "AllowMerge", false ))
            {
            for( Action_data_group::ConstIterator it = parent_P->first_child();
                 it != parent_P->after_last_child();
                 ++it )
                {
                if( Action_data_group* existing = dynamic_cast< Action_data_group* >( *it ))
                    {
                    if( cfg_P.readEntry( "Name" ) == existing->name())
                        return existing;
                    }
                }
            }
        return new Action_data_group( cfg_P, parent_P );
        }
    if( type == "GENERIC_ACTION_DATA" )
        return new Generic_action_data( cfg_P, parent_P );
    if( type == "COMMAND_URL_SHORTCUT_ACTION_DATA" )
        return new Command_url_shortcut_action_data( cfg_P, parent_P );
    if( type == "MENUENTRY_SHORTCUT_ACTION_DATA" )
        return new Menuentry_shortcut_action_data( cfg_P, parent_P );
    if( type == "DCOP_SHORTCUT_ACTION_DATA" || type == "DBUS_SHORTCUT_ACTION_DATA" )
        return new Dbus_shortcut_action_data( cfg_P, parent_P );
    if( type == "KEYBOARD_INPUT_SHORTCUT_ACTION_DATA" )
        return new Keyboard_input_shortcut_action_data( cfg_P, parent_P );
    if( type == "KEYBOARD_INPUT_GESTURE_ACTION_DATA" )
        return new Keyboard_input_gesture_action_data( cfg_P, parent_P );
    if( type == "ACTIVATE_WINDOW_SHORTCUT_ACTION_DATA" )
        return new Activate_window_shortcut_action_data( cfg_P, parent_P );
    kWarning( 1217 ) << "Unknown Action_data_base type read from cfg file\n";
    return 0;
    }
    
bool Action_data_base::cfg_is_enabled( KConfigGroup& cfg_P )
    {
    return cfg_P.readEntry( "Enabled", true);
    }
    
void Action_data_base::reparent( Action_data_group* new_parent_P )
    {
    if( parent())
        parent()->remove_child( this );
    _parent = new_parent_P;
    if( parent())
        parent()->add_child( this );
    }

bool Action_data_base::enabled( bool ignore_group_P ) const
    {
    if( ignore_group_P )
        return _enabled;
    else
        return _enabled && ( parent() == 0 || parent()->enabled( false ));
    }

void Action_data_base::set_conditions( Condition_list* conditions_P )
    {
    assert( _conditions == 0 );
    _conditions = conditions_P;
    }

bool Action_data_base::conditions_match() const
    {
    return ( conditions() ? conditions()->match() : true )
        && ( parent() ? parent()->conditions_match() : true );
    }
    
// Action_data_group

Action_data_group::Action_data_group( KConfigGroup& cfg_P, Action_data_group* parent_P )
    : Action_data_base( cfg_P, parent_P )
    {
    unsigned int system_group_tmp = cfg_P.readEntry( "SystemGroup", 0 );
    if( system_group_tmp >= SYSTEM_MAX )
        system_group_tmp = 0;
    _system_group = static_cast< system_group_t >( system_group_tmp );
    }

void Action_data_group::cfg_write( KConfigGroup& cfg_P ) const
    {
    Action_data_base::cfg_write( cfg_P );
    cfg_P.writeEntry( "SystemGroup", int(system_group()));
    cfg_P.writeEntry( "Type", "ACTION_DATA_GROUP" );
    }
    
void Action_data_group::update_triggers()
    {
    for( Action_data_group::ConstIterator it = first_child();
         it != after_last_child();
         ++it )
        ( *it )->update_triggers();
    }

// Action_data

Action_data::Action_data( KConfigGroup& cfg_P, Action_data_group* parent_P )
    : Action_data_base( cfg_P, parent_P )
    {
    KConfigGroup triggersGroup( cfg_P.config(), cfg_P.name() + "Triggers" );
    _triggers = new Trigger_list( triggersGroup, this );
    KConfigGroup actionsGroup( cfg_P.config(), cfg_P.name() + "Actions" );
    _actions = new Action_list( actionsGroup, this );
    }

Action_data::~Action_data()
    {
//    kDebug( 1217 ) << "~Action_data" << this;
    delete _triggers;
    delete _actions;
    // CHECKME jeste remove z parenta ?
    }
    
void Action_data::cfg_write( KConfigGroup& cfg_P ) const
    {
    Action_data_base::cfg_write( cfg_P );
    KConfigGroup triggersGroup( cfg_P.config(), cfg_P.name() + "Triggers" );
    triggers()->cfg_write( triggersGroup );
    KConfigGroup actionsGroup( cfg_P.config(), cfg_P.name() + "Actions" );
    actions()->cfg_write( actionsGroup );
    }

void Action_data::execute()
    {
    for( Action_list::Iterator it = _actions->begin();
         it != _actions->end();
         ++it )
        (*it)->execute();
// CHECKME nebo nejak zpozdeni ?
    }

void Action_data::add_trigger( Trigger* trigger_P )
    {
    _triggers->append( trigger_P );
    }

void Action_data::add_triggers( Trigger_list* triggers_P )
    {
    while (!triggers_P->isEmpty())
        {
        _triggers->append( triggers_P->takeFirst() );
        }
    Q_ASSERT( triggers_P->isEmpty());
    delete triggers_P;
    }

void Action_data::set_triggers( Trigger_list* triggers_P )
    {
    assert( _triggers == 0 );
    _triggers = triggers_P;
    }

void Action_data::add_action( Action* action_P, Action* after_P )
    {
    int index = 0;
    for( Action_list::Iterator it = _actions->begin();
         it != _actions->end();
         ++it )
        {
        ++index;
        if( *it == after_P )
            break;
        }
    _actions->insert( index, action_P );
    }
    
void Action_data::add_actions( Action_list* actions_P, Action* after_P )
    {
    int index = 0;
    for( Action_list::Iterator it = _actions->begin();
         it != _actions->end();
         ++it )
        {
        ++index;
        if( *it == after_P )
            break;
        }

    while (!actions_P->empty())
        {
        // Insert the actions to _actions after removing them from actions_P
        // to prevent their deletion upon delete actions_P below.
        _actions->insert( ++index, actions_P->takeFirst() );
        }
    Q_ASSERT( actions_P->isEmpty());
    delete actions_P;
    }

void Action_data::set_actions( Action_list* actions_P )
    {
    assert( _actions == 0 );
    _actions = actions_P;
    }

void Action_data::update_triggers()
    {
    bool activate = conditions_match() && enabled( false );
    kDebug( 1217 ) << "Update triggers: " << name() << ":" << activate;
    for( Trigger_list::Iterator it = _triggers->begin();
         it != _triggers->end();
         ++it )
        {
        (*it)->activate( activate );
        }
    }
        
// Generic_action_data

void Generic_action_data::cfg_write( KConfigGroup& cfg_P ) const
    {
    base::cfg_write( cfg_P );
    cfg_P.writeEntry( "Type", "GENERIC_ACTION_DATA" );
    }

// Command_url_shortcut_action_data

Command_url_shortcut_action_data::Command_url_shortcut_action_data( Action_data_group* parent_P,
    const QString& name_P, const QString& comment_P,
    const KShortcut& shortcut_P, const QString& command_url_P, bool enabled_P )
    : Simple_action_data< Shortcut_trigger, Command_url_action >( parent_P, name_P,
        comment_P, enabled_P )
    {
    set_action( new Command_url_action( this, command_url_P ));
    set_trigger( new Shortcut_trigger( this, shortcut_P ));
    }

template<> KDE_EXPORT
void Simple_action_data< Shortcut_trigger, Command_url_action >
    ::cfg_write( KConfigGroup& cfg_P ) const
    {
    base::cfg_write( cfg_P );
    cfg_P.writeEntry( "Type", "COMMAND_URL_SHORTCUT_ACTION_DATA" );
    }

// Menuentry_shortcut_action_data

Menuentry_shortcut_action_data::Menuentry_shortcut_action_data( Action_data_group* parent_P,
    const QString& name_P, const QString& comment_P,
    const KShortcut& shortcut_P, const QString& menuentry_P, bool enabled_P )
    : Simple_action_data< Shortcut_trigger, Menuentry_action >( parent_P, name_P,
        comment_P, enabled_P )
    {
    set_action( new Menuentry_action( this, menuentry_P ));
    set_trigger( new Shortcut_trigger( this, shortcut_P ));
    }
    
template<> KDE_EXPORT
void Simple_action_data< Shortcut_trigger, Menuentry_action >
    ::cfg_write( KConfigGroup& cfg_P ) const
    {
    base::cfg_write( cfg_P );
    cfg_P.writeEntry( "Type", "MENUENTRY_SHORTCUT_ACTION_DATA" );
    }

// Dbus_shortcut_action_data

template<> KDE_EXPORT
void Simple_action_data< Shortcut_trigger, Dbus_action >
    ::cfg_write( KConfigGroup& cfg_P ) const
    {
    base::cfg_write( cfg_P );
    cfg_P.writeEntry( "Type", "DBUS_SHORTCUT_ACTION_DATA" );
    }

// Keyboard_input_shortcut_action_data

template<> KDE_EXPORT
void Simple_action_data< Shortcut_trigger, Keyboard_input_action >
    ::cfg_write( KConfigGroup& cfg_P ) const
    {
    base::cfg_write( cfg_P );
    cfg_P.writeEntry( "Type", "KEYBOARD_INPUT_SHORTCUT_ACTION_DATA" );
    }

// Activate_window_shortcut_action_data

template<> KDE_EXPORT
void Simple_action_data< Shortcut_trigger, Activate_window_action >
    ::cfg_write( KConfigGroup& cfg_P ) const
    {
    base::cfg_write( cfg_P );
    cfg_P.writeEntry( "Type", "ACTIVATE_WINDOW_SHORTCUT_ACTION_DATA" );
    }
    
// Keyboard_input_gesture_action_data

void Keyboard_input_gesture_action_data::set_action( Keyboard_input_action* action_P )
    {
    Action_list* tmp = new Action_list( "Keyboard_input_gesture_action_data" );
    tmp->append( action_P );
    set_actions( tmp );
    }

const Keyboard_input_action* Keyboard_input_gesture_action_data::action() const
    {
    if( actions() == 0 || actions()->isEmpty() ) // CHECKME tohle poradne zkontrolovat
        return 0;
    return static_cast< Keyboard_input_action* >( const_cast< Action_list* >( actions())->first());
    }

void Keyboard_input_gesture_action_data::cfg_write( KConfigGroup& cfg_P ) const
    {
    base::cfg_write( cfg_P );
    cfg_P.writeEntry( "Type", "KEYBOARD_INPUT_GESTURE_ACTION_DATA" );
    }

} // namespace KHotKeys
