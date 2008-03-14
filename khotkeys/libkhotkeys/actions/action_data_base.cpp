/****************************************************************************

 KHotKeys
 
 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

#include "action_data.h"
#include "actions.h"

#include <kconfiggroup.h>
#include <kdebug.h>


namespace KHotKeys
{

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


const Condition_list* Action_data_base::conditions() const
    {
//    Q_ASSERT( _conditions != 0 );
    return _conditions;
    }


Action_data_group* Action_data_base::parent() const
    {
    return _parent;
    }


void Action_data_base::set_name( const QString& name_P )
    {
    _name = name_P;
    }


const QString& Action_data_base::name() const
    {
    return _name;
    }


const QString& Action_data_base::comment() const
    {
    return _comment;
    }


void Action_data_base::cfg_write( KConfigGroup& cfg_P ) const
    {
    cfg_P.writeEntry( "Type", "ERROR" ); // derived classes should call with their type
    cfg_P.writeEntry( "Name", name());
    cfg_P.writeEntry( "Comment", comment());
    cfg_P.writeEntry( "Enabled", enabled( true ));
    KConfigGroup conditionsConfig( cfg_P.config(), cfg_P.name() + "Conditions" );
    Q_ASSERT( conditions() != 0 );
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
    Q_ASSERT( _conditions == 0 );
    _conditions = conditions_P;
    }


bool Action_data_base::conditions_match() const
    {
    return ( conditions() ? conditions()->match() : true )
        && ( parent() ? parent()->conditions_match() : true );
    }

} // namespace KHotKeys
