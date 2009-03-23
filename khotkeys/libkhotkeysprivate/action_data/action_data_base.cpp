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
        ,Condition_list* conditions_P
        ,bool enabled_P)
            : _parent(parent_P)
              ,_conditions(conditions_P)
              ,_name(name_P)
              ,_comment(comment_P)
              ,_enabled(enabled_P)
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


ActionDataBase::ActionDataBase(
        const KConfigGroup& cfg_P
        ,ActionDataGroup* parent_P)
            : _parent( parent_P)
              ,_conditions(NULL)
    {
    _name = cfg_P.readEntry( "Name" );
    _comment = cfg_P.readEntry( "Comment" );
    _enabled = cfg_P.readEntry( "Enabled", true);
    KConfigGroup conditionsConfig( cfg_P.config(), cfg_P.name() + "Conditions" );

    // Load the conditions if they exist
    if ( conditionsConfig.exists() )
        {
        _conditions = new Condition_list( conditionsConfig, this );
        }
    else
        {
        _conditions = new Condition_list(QString(), this);
        }

    if (parent()) parent()->add_child( this );
    }


ActionDataBase::~ActionDataBase()
    {
    if( parent())
        parent()->remove_child( this );
    delete _conditions;
    }


void ActionDataBase::accept(ActionDataVisitor *visitor) const
    {
    visitor->visitActionDataBase(this);
    }


bool ActionDataBase::cfg_is_enabled(const KConfigGroup& cfg_P )
    {
    return cfg_P.readEntry( "Enabled", true);
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


bool ActionDataBase::enabled( bool ignore_group_P ) const
    {
    if( ignore_group_P )
        return _enabled;
    else
        return _enabled && ( parent() == 0 || parent()->enabled( false ));
    }


QString ActionDataBase::name() const
    {
    return _name;
    }


ActionDataGroup* ActionDataBase::parent() const
    {
    return _parent;
    }


void ActionDataBase::set_comment( const QString &comment )
    {
    _comment = comment;
    }


void ActionDataBase::set_enabled( bool enabled )
    {
    _enabled = enabled;
    }


void ActionDataBase::set_name( const QString& name_P )
    {
    _name = name_P;
    }


void ActionDataBase::set_conditions( Condition_list* conditions_P )
    {
    Q_ASSERT( _conditions == 0 );
    _conditions = conditions_P;
    }


} // namespace KHotKeys
