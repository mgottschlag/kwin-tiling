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

ActionDataGroup::ActionDataGroup( KConfigGroup& cfg_P, ActionDataGroup* parent_P )
    : ActionDataBase( cfg_P, parent_P )
    {
    unsigned int system_group_tmp = cfg_P.readEntry( "SystemGroup", 0 );
    if( system_group_tmp >= SYSTEM_MAX )
        system_group_tmp = 0;
    _system_group = static_cast< system_group_t >( system_group_tmp );
    }


ActionDataGroup::ActionDataGroup( ActionDataGroup* parent_P, const QString& name_P,
    const QString& comment_P, Condition_list* conditions_P, system_group_t system_group_P,
    bool enabled_P )
    : ActionDataBase( parent_P, name_P, comment_P, conditions_P, enabled_P ),
        _system_group( system_group_P )
    {
    }


ActionDataGroup::~ActionDataGroup()
    {
    kDebug( 1217 ) << "~ActionDataGroup() :" << list.count();
    qDeleteAll(list);
    list.clear();
    }


ActionDataGroup::ConstIterator ActionDataGroup::first_child() const
    {
    return list.begin();
    }


ActionDataGroup::ConstIterator ActionDataGroup::after_last_child() const
    {
    return list.end();
    }


bool ActionDataGroup::is_system_group() const
    {
    return _system_group != SYSTEM_NONE;
    }


ActionDataGroup::system_group_t ActionDataGroup::system_group() const
    {
    return _system_group;
    }


void ActionDataGroup::add_child( ActionDataBase* child_P )
    {
    list.append( child_P ); // CHECKME tohle asi znemozni je mit nejak rucne serazene
    }

int ActionDataGroup::child_count() const
    {
    return list.size();
    }


void ActionDataGroup::remove_child( ActionDataBase* child_P )
    {
    list.removeAll( child_P ); // is not auto-delete
    }


void ActionDataGroup::cfg_write( KConfigGroup& cfg_P ) const
    {
    kDebug() << "Writing group " << cfg_P.name();
    ActionDataBase::cfg_write( cfg_P );
    cfg_P.writeEntry( "SystemGroup", int(system_group()));
    cfg_P.writeEntry( "Type", "ACTION_DATA_GROUP" );
    }


void ActionDataGroup::update_triggers()
    {
    for( ActionDataGroup::ConstIterator it = first_child();
         it != after_last_child();
         ++it )
        ( *it )->update_triggers();
    }


} // namespace KHotKeys
