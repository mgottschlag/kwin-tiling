/****************************************************************************

 KHotKeys
 
 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

#include "action_data_group.h"

#include "action_data/action_data_visitor.h"
#include "actions/actions.h"

#include <kconfiggroup.h>
#include <kdebug.h>


namespace KHotKeys
{

ActionDataGroup::ActionDataGroup(
        ActionDataGroup* parent_P,
        const QString& name_P,
        const QString& comment_P,
        Condition_list* conditions_P,
        system_group_t system_group_P)
            : ActionDataBase(parent_P, name_P, comment_P, conditions_P)
              ,_list()
              ,_system_group(system_group_P)
    {}


ActionDataGroup::~ActionDataGroup()
    {
    while (!_list.isEmpty())
        {
        delete _list.takeFirst();
        }
    }


void ActionDataGroup::accept(ActionDataVisitor *visitor)
    {
    visitor->visitActionDataGroup(this);
    }



void ActionDataGroup::accept(ActionDataConstVisitor *visitor) const
    {
    visitor->visitActionDataGroup(this);
    }



Action::ActionTypes ActionDataGroup::allowedActionTypes() const
    {
    switch (_system_group)
        {
        case SYSTEM_MENUENTRIES:
            return Action::MenuEntryActionType;

        default:
            return Action::AllTypes;
        }
    }


Trigger::TriggerTypes ActionDataGroup::allowedTriggerTypes() const
    {
    switch (_system_group)
        {
        case SYSTEM_MENUENTRIES:
            return Trigger::ShortcutTriggerType;

        default:
            return Trigger::AllTypes;
        }
    }


bool ActionDataGroup::is_system_group() const
    {
    return _system_group != SYSTEM_NONE and _system_group != SYSTEM_ROOT;
    }


void ActionDataGroup::set_system_group(system_group_t group)
    {
    _system_group = group;
    }


ActionDataGroup::system_group_t ActionDataGroup::system_group() const
    {
    return _system_group;
    }


void ActionDataGroup::add_child(ActionDataBase* child_P)
    {
    // Just make sure we don't get the same child twice
    Q_ASSERT(!_list.contains(child_P));
    if (_list.contains(child_P)) return;

    if (child_P->parent())
        {
        child_P->parent()->remove_child( child_P );
        }

    child_P->_parent = this;

    _list.append( child_P );
    }


void ActionDataGroup::add_child(ActionDataBase* child_P, int position)
    {
    // Just make sure we don't get the same child twice
    Q_ASSERT(!_list.contains(child_P));
    if (_list.contains(child_P)) return;

    if (child_P->parent())
        {
        child_P->parent()->remove_child( child_P );
        }

    child_P->_parent = this;

    _list.insert( position, child_P );
    }


void ActionDataGroup::aboutToBeErased()
    {
    Q_FOREACH( ActionDataBase *child, _list)
        {
        child->aboutToBeErased();
        }
    }


const QList<ActionDataBase*> ActionDataGroup::children() const
    {
    return _list;
    }


void ActionDataGroup::doDisable()
    {
    Q_FOREACH( ActionDataBase *child, _list)
        {
        child->update_triggers();
        }
    }


void ActionDataGroup::doEnable()
    {
    Q_FOREACH( ActionDataBase *child, _list)
        {
        child->update_triggers();
        }

    }


void ActionDataGroup::remove_child( ActionDataBase* child_P )
    {
    child_P->_parent = NULL;
    _list.removeAll( child_P ); // is not auto-delete
    }


int ActionDataGroup::size() const
    {
    return _list.size();
    }


void ActionDataGroup::update_triggers()
    {
    Q_FOREACH(ActionDataBase *child, children())
        {
        child->update_triggers();
        }
    }


} // namespace KHotKeys

#include "moc_action_data_group.cpp"
