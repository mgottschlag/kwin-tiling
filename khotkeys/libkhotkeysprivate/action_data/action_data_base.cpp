/****************************************************************************

 KHotKeys
 
 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

#include "action_data_base.h"

#include "action_data/action_data_visitor.h"
#include "action_data/action_data_group.h"

#include "conditions/conditions_list.h"

#include <kconfiggroup.h>
#include <kdebug.h>


namespace KHotKeys
{

ActionDataBase::ActionDataBase(
        ActionDataGroup* parent_P
        ,const QString& name_P
        ,const QString& comment_P
        ,Condition_list* conditions_P)
    :   _parent(parent_P)
        ,_conditions(conditions_P)
        ,_name(name_P)
        ,_comment(comment_P)
        ,_enabled(false)
        ,_importId()
        ,_allowMerging(false)
    {
    if (parent()) parent()->add_child( this );

    if (!_conditions)
        {
        _conditions = new Condition_list(QString(), this);
        }
    else
        {
        _conditions->set_data( this );
        }
    }


ActionDataBase::~ActionDataBase()
    {
    if( parent())
        parent()->remove_child( this );
    delete _conditions;
    }


void ActionDataBase::accept(ActionDataVisitor *visitor)
    {
    visitor->visitActionDataBase(this);
    }


void ActionDataBase::accept(ActionDataConstVisitor *visitor) const
    {
    visitor->visitActionDataBase(this);
    }


bool ActionDataBase::cfg_is_enabled(const KConfigGroup& cfg_P )
    {
    return cfg_P.readEntry("Enabled", false);
    }


bool ActionDataBase::allowMerging() const
    {
    return _allowMerging;
    }


QString ActionDataBase::comment() const
    {
    return _comment;
    }


const Condition_list* ActionDataBase::conditions() const
    {
    return _conditions;
    }


Condition_list* ActionDataBase::conditions()
    {
    return _conditions;
    }


bool ActionDataBase::conditions_match() const
    {
    return ( conditions() ? conditions()->match() : true )
        && ( parent() ? parent()->conditions_match() : true );
    }


bool ActionDataBase::isEnabled(IgnoreParent ip ) const
    {
    if( ip == Ignore )
        return _enabled;
    else
        return _enabled && ( parent() == 0 || parent()->isEnabled());
    }


QString ActionDataBase::name() const
    {
    return _name;
    }


ActionDataGroup* ActionDataBase::parent() const
    {
    return _parent;
    }


void ActionDataBase::setAllowMerging(bool allowMerging)
    {
    _allowMerging = allowMerging;
    }


void ActionDataBase::set_comment( const QString &comment )
    {
    _comment = comment;
    }


void ActionDataBase::disable()
    {
    if (!_enabled)
        return;

    _enabled = false;
    doDisable();
    }


void ActionDataBase::enable()
    {
    if (_enabled)
        return;

    _enabled = true;

    // Enable only if the parent is enabled too
    if (isEnabled())
        // FIXME: let doEnable decide if it makes sense to enable (No trigger
        // .... )
        doEnable();
    }


QString
ActionDataBase::importId() const
    {
    return _importId;
    }


void ActionDataBase::set_name( const QString& name_P )
    {
    _name = name_P;
    }


void ActionDataBase::set_conditions(Condition_list* conditions)
    {
    if (_conditions)
        {
        delete _conditions;
        }

    _conditions = conditions;
    }


void
ActionDataBase::setImportId(const QString &id)
    {
    _importId = id;
    }


} // namespace KHotKeys

#include "moc_action_data_base.cpp"
