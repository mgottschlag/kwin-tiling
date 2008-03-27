/****************************************************************************

 KHotKeys
 
 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

#include "action_data_group.h"
#include "actions.h"

#include <kconfiggroup.h>
#include <kdebug.h>


namespace KHotKeys
{

Action_data_group::Action_data_group( KConfigGroup& cfg_P, Action_data_group* parent_P )
    : Action_data_base( cfg_P, parent_P )
    {
    unsigned int system_group_tmp = cfg_P.readEntry( "SystemGroup", 0 );
    if( system_group_tmp >= SYSTEM_MAX )
        system_group_tmp = 0;
    _system_group = static_cast< system_group_t >( system_group_tmp );
    }


Action_data_group::Action_data_group( Action_data_group* parent_P, const QString& name_P,
    const QString& comment_P, Condition_list* conditions_P, system_group_t system_group_P,
    bool enabled_P )
    : Action_data_base( parent_P, name_P, comment_P, conditions_P, enabled_P ),
        _system_group( system_group_P )
    {
    }


Action_data_group::~Action_data_group()
    {
    kDebug( 1217 ) << "~Action_data_group() :" << list.count();
    qDeleteAll(list);
    list.clear();
    }


Action_data_group::ConstIterator Action_data_group::first_child() const
    {
    return list.begin();
    }


Action_data_group::ConstIterator Action_data_group::after_last_child() const
    {
    return list.end();
    }


bool Action_data_group::is_system_group() const
    {
    return _system_group != SYSTEM_NONE;
    }


Action_data_group::system_group_t Action_data_group::system_group() const
    {
    return _system_group;
    }


void Action_data_group::add_child( Action_data_base* child_P )
    {
    list.append( child_P ); // CHECKME tohle asi znemozni je mit nejak rucne serazene
    }

int Action_data_group::child_count() const
    {
    return list.size();
    }


void Action_data_group::remove_child( Action_data_base* child_P )
    {
    list.removeAll( child_P ); // is not auto-delete
    }


void Action_data_group::cfg_write( KConfigGroup& cfg_P ) const
    {
    kDebug() << "Writing group " << cfg_P.name();
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


} // namespace KHotKeys
