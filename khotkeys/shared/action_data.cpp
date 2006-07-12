/****************************************************************************

 KHotKeys
 
 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

#define _ACTION_DATA_CPP_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "action_data.h"

#include <kconfig.h>

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
    if( _conditions != NULL )
        _conditions->set_data( this );
    }
    
Action_data_base::Action_data_base( KConfig& cfg_P, Action_data_group* parent_P )
    : _parent( parent_P )
    {
    QString save_cfg_group = cfg_P.group();
    _name = cfg_P.readEntry( "Name" );
    _comment = cfg_P.readEntry( "Comment" );
    _enabled = cfg_P.readEntry( "Enabled", QVariant(true )).toBool();
    cfg_P.setGroup( save_cfg_group + "Conditions" );
    _conditions = new Condition_list( cfg_P, this );
    cfg_P.setGroup( save_cfg_group );
    if( parent())
        parent()->add_child( this );
    }
    
Action_data_base::~Action_data_base()
    {
//    kDebug( 1217 ) << "~Action_data_base() :" << this << endl;
    if( parent())
        parent()->remove_child( this );
    delete _conditions;
    }
    
void Action_data_base::cfg_write( KConfig& cfg_P ) const
    {
    cfg_P.writeEntry( "Type", "ERROR" ); // derived classes should call with their type
    cfg_P.writeEntry( "Name", name());
    cfg_P.writeEntry( "Comment", comment());
    cfg_P.writeEntry( "Enabled", enabled( true ));
    QString save_cfg_group = cfg_P.group();
    cfg_P.setGroup( save_cfg_group + "Conditions" );
    assert( conditions() != NULL );
    conditions()->cfg_write( cfg_P );
    cfg_P.setGroup( save_cfg_group );
    }


Action_data_base* Action_data_base::create_cfg_read( KConfig& cfg_P, Action_data_group* parent_P )
    {
    QString type = cfg_P.readEntry( "Type" );
    if( type == "ACTION_DATA_GROUP" )
        {
        if( cfg_P.readEntry( "AllowMerge", false ))
            {
            for( Action_data_group::Iterator it = parent_P->first_child();
                 it;
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
    if( type == "DCOP_SHORTCUT_ACTION_DATA" )
        return new Dcop_shortcut_action_data( cfg_P, parent_P );
    if( type == "KEYBOARD_INPUT_SHORTCUT_ACTION_DATA" )
        return new Keyboard_input_shortcut_action_data( cfg_P, parent_P );
    if( type == "KEYBOARD_INPUT_GESTURE_ACTION_DATA" )
        return new Keyboard_input_gesture_action_data( cfg_P, parent_P );
    if( type == "ACTIVATE_WINDOW_SHORTCUT_ACTION_DATA" )
        return new Activate_window_shortcut_action_data( cfg_P, parent_P );
    kWarning( 1217 ) << "Unknown Action_data_base type read from cfg file\n";
    return NULL;
    }
    
bool Action_data_base::cfg_is_enabled( KConfig& cfg_P )
    {
    return cfg_P.readEntry( "Enabled", QVariant(true )).toBool();
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
        return _enabled && ( parent() == NULL || parent()->enabled( false ));
    }

bool Action_data_base::conditions_match() const
    {
    return ( conditions() ? conditions()->match() : true )
        && ( parent() ? parent()->conditions_match() : true );
    }
    
// Action_data_group

Action_data_group::Action_data_group( KConfig& cfg_P, Action_data_group* parent_P )
    : Action_data_base( cfg_P, parent_P )
    {
    unsigned int system_group_tmp = cfg_P.readEntry( "SystemGroup", 0 );
    if( system_group_tmp >= SYSTEM_MAX )
        system_group_tmp = 0;
    _system_group = static_cast< system_group_t >( system_group_tmp );
    }

void Action_data_group::cfg_write( KConfig& cfg_P ) const
    {
    Action_data_base::cfg_write( cfg_P );
    cfg_P.writeEntry( "SystemGroup", int(system_group()));
    cfg_P.writeEntry( "Type", "ACTION_DATA_GROUP" );
    }
    
void Action_data_group::update_triggers()
    {
    for( Action_data_group::Iterator it = first_child();
         it;
         ++it )
        ( *it )->update_triggers();
    }

// Action_data

Action_data::Action_data( KConfig& cfg_P, Action_data_group* parent_P )
    : Action_data_base( cfg_P, parent_P )
    {
    QString save_cfg_group = cfg_P.group();
    cfg_P.setGroup( save_cfg_group + "Triggers" );
    _triggers = new Trigger_list( cfg_P, this );
    cfg_P.setGroup( save_cfg_group + "Actions" );
    _actions = new Action_list( cfg_P, this );
    cfg_P.setGroup( save_cfg_group );
    }

Action_data::~Action_data()
    {
//    kDebug( 1217 ) << "~Action_data" << this << endl;
    delete _triggers;
    delete _actions;
    // CHECKME jeste remove z parenta ?
    }
    
void Action_data::cfg_write( KConfig& cfg_P ) const
    {
    Action_data_base::cfg_write( cfg_P );
    QString save_cfg_group = cfg_P.group();
    cfg_P.setGroup( save_cfg_group + "Triggers" );
    triggers()->cfg_write( cfg_P );
    cfg_P.setGroup( save_cfg_group + "Actions" );
    actions()->cfg_write( cfg_P );
    cfg_P.setGroup( save_cfg_group );
    }

void Action_data::execute()
    {
    for( Action_list::Iterator it( *_actions );
         it;
         ++it )
        it.current()->execute();
// CHECKME nebo nejak zpozdeni ?
    }

void Action_data::add_trigger( Trigger* trigger_P )
    {
    _triggers->append( trigger_P );
    }

void Action_data::add_triggers( Trigger_list* triggers_P )
    {
    for( Trigger_list::Iterator it = *triggers_P;
         it;
         ++it )
        _triggers->append( *it );
    triggers_P->setAutoDelete( false );
    delete triggers_P;
    }
    
void Action_data::set_triggers( Trigger_list* triggers_P )
    {
    assert( _triggers == NULL );
    _triggers = triggers_P;
    }

void Action_data::add_action( Action* action_P, Action* after_P )
    {
    int index = 0;
    for( Action_list::Iterator it = *_actions;
         it;
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
    for( Action_list::Iterator it = *_actions;
         it;
         ++it )
        {
        ++index;
        if( *it == after_P )
            break;
        }
    for( Action_list::Iterator it = *actions_P;
         it;
         ++it )
        _actions->insert( index++, *it );
    actions_P->setAutoDelete( false );
    delete actions_P;
    }

void Action_data::set_actions( Action_list* actions_P )
    {
    assert( _actions == NULL );
    _actions = actions_P;
    }

void Action_data::update_triggers()
    {
    bool activate = conditions_match() && enabled( false );
    kDebug( 1217 ) << "Update triggers: " << name() << ":" << activate << endl;
    for( Trigger_list::Iterator it = ( *triggers());
         it;
         ++it )
        ( *it )->activate( activate );
    }
        
// Generic_action_data

void Generic_action_data::cfg_write( KConfig& cfg_P ) const
    {
    base::cfg_write( cfg_P );
    cfg_P.writeEntry( "Type", "GENERIC_ACTION_DATA" );
    }

// Simple_action_data

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
    if( actions() == NULL || actions()->count() == 0 ) // CHECKME tohle poradne zkontrolovat
        return NULL;
    return static_cast< A* >( const_cast< Action_list* >( actions())->first());
    }

template< typename T, typename A >
const T* Simple_action_data< T, A >::trigger() const
    {
    if( triggers() == NULL || triggers()->count() == 0 ) // CHECKME tohle poradne zkontrolovat
        return NULL;
    return static_cast< T* >( const_cast< Trigger_list* >( triggers())->first());
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

template<>
void Simple_action_data< Shortcut_trigger, Command_url_action >
    ::cfg_write( KConfig& cfg_P ) const
    {
    base::cfg_write( cfg_P );
    cfg_P.writeEntry( "Type", "COMMAND_URL_SHORTCUT_ACTION_DATA" );
    }

template class Simple_action_data< Shortcut_trigger, Command_url_action >;

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
    
template<>
void Simple_action_data< Shortcut_trigger, Menuentry_action >
    ::cfg_write( KConfig& cfg_P ) const
    {
    base::cfg_write( cfg_P );
    cfg_P.writeEntry( "Type", "MENUENTRY_SHORTCUT_ACTION_DATA" );
    }

template class Simple_action_data< Shortcut_trigger, Menuentry_action >;

// Dcop_shortcut_action_data

template<>
void Simple_action_data< Shortcut_trigger, Dcop_action >
    ::cfg_write( KConfig& cfg_P ) const
    {
    base::cfg_write( cfg_P );
    cfg_P.writeEntry( "Type", "DCOP_SHORTCUT_ACTION_DATA" );
    }
template class Simple_action_data< Shortcut_trigger, Dcop_action >;
// Keyboard_input_shortcut_action_data

template<>
void Simple_action_data< Shortcut_trigger, Keyboard_input_action >
    ::cfg_write( KConfig& cfg_P ) const
    {
    base::cfg_write( cfg_P );
    cfg_P.writeEntry( "Type", "KEYBOARD_INPUT_SHORTCUT_ACTION_DATA" );
    }
template class Simple_action_data< Shortcut_trigger, Keyboard_input_action >;

// Activate_window_shortcut_action_data

template<>
void Simple_action_data< Shortcut_trigger, Activate_window_action >
    ::cfg_write( KConfig& cfg_P ) const
    {
    base::cfg_write( cfg_P );
    cfg_P.writeEntry( "Type", "ACTIVATE_WINDOW_SHORTCUT_ACTION_DATA" );
    }
    
template class Simple_action_data< Shortcut_trigger, Activate_window_action >;
// Keyboard_input_gesture_action_data

void Keyboard_input_gesture_action_data::set_action( Keyboard_input_action* action_P )
    {
    Action_list* tmp = new Action_list( "Keyboard_input_gesture_action_data" );
    tmp->append( action_P );
    set_actions( tmp );
    }

const Keyboard_input_action* Keyboard_input_gesture_action_data::action() const
    {
    if( actions() == NULL ) // CHECKME tohle poradne zkontrolovat
        return NULL;
    return static_cast< Keyboard_input_action* >( const_cast< Action_list* >( actions())->first());
    }

void Keyboard_input_gesture_action_data::cfg_write( KConfig& cfg_P ) const
    {
    base::cfg_write( cfg_P );
    cfg_P.writeEntry( "Type", "KEYBOARD_INPUT_GESTURE_ACTION_DATA" );
    }

} // namespace KHotKeys
