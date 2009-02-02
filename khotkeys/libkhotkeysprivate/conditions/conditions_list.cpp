/*
   Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>
   Copyright (C) 2008 Michael Jansen <kde@michael-jansen.biz>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "conditions/conditions_list.h"
#include "conditions/conditions_visitor.h"

#include "action_data/action_data.h"
#include "khotkeysglobal.h"

#include <KDE/KConfigGroup>
#include <KDE/KDebug>

namespace KHotKeys {

Condition_list::Condition_list( KConfigGroup& cfg_P, ActionDataBase* data_P )
    : Condition_list_base( cfg_P, NULL ), data( data_P )
    {
    _comment = cfg_P.readEntry( "Comment" );
    }


Condition_list::Condition_list( const QString& comment_P, ActionDataBase* data_P )
    : Condition_list_base( NULL ), _comment( comment_P ), data( data_P )
    {
    }


void Condition_list::cfg_write( KConfigGroup& cfg_P ) const
    {
    base::cfg_write( cfg_P );
    cfg_P.writeEntry( "Comment", comment());
    }


const QString& Condition_list::comment() const
    {
    return _comment;
    }


Condition_list* Condition_list::copy() const
    {
    kDebug() << count();

    Condition_list* ret = new Condition_list(comment());
    for( ConstIterator it = begin();
            it != end();
            ++it )
        {
        ret->append((*it)->copy());
        }
    return ret;
    }


const QString Condition_list::description() const
    {
    return QString();
    }


bool Condition_list::match() const
    {
    if( count() == 0 ) // no conditions to match => ok
        return true;
    for( ConstIterator it(begin());
         it != end();
         ++it )
        if( (*it)->match()) // OR
            return true;
    return false;
    }


void Condition_list::set_data( ActionDataBase* data_P )
    {
    Q_ASSERT( data == NULL || data == data_P );
    data = data_P;
    }


void Condition_list::updated() const
    {
    if( !khotkeys_active())
        return;
    data->update_triggers();
//    base::updated(); no need to, doesn't have parent
    }


void Condition_list::visit( ConditionsVisitor *visitor )
    {
    visitor->visitConditionsList(this);
    }


} // namespace KHotKeys

