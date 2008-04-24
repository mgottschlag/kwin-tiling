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

#include "conditions.h"
#include "action_data.h"

#include <KDE/KConfigGroup>

namespace KHotKeys {

Condition_list::Condition_list( KConfigGroup& cfg_P, ActionDataBase* data_P )
    : Condition_list_base( cfg_P, NULL ), data( data_P )
    {
    _comment = cfg_P.readEntry( "Comment" );
    }

void Condition_list::cfg_write( KConfigGroup& cfg_P ) const
    {
    base::cfg_write( cfg_P );
    cfg_P.writeEntry( "Comment", comment());
    }

Condition_list* Condition_list::copy( ActionDataBase* data_P ) const
    {
    Condition_list* ret = new Condition_list( comment(), data_P );
    for( Iterator it( *this );
         it;
         ++it )
        ret->append( it.current()->copy( ret ));
    return ret;
    }


bool Condition_list::match() const
    {
    if( count() == 0 ) // no conditions to match => ok
        return true;
    for( Iterator it( *this );
         it;
         ++it )
        if( it.current()->match()) // OR
            return true;
    return false;
    }

void Condition_list::updated() const
    {
    if( !khotkeys_active())
        return;
    data->update_triggers();
//    base::updated(); no need to, doesn't have parent
    }

// CHECKME tohle je drobet hack, jeste to zvazit
void Condition_list::set_data( ActionDataBase* data_P )
    {
    Q_ASSERT( data == NULL || data == data_P );
    data = data_P;
    }

const QString Condition_list::description() const
    {
    Q_ASSERT( false );
    return QString();
    }

Condition_list* Condition_list::copy( Condition_list_base* ) const
    {
    Q_ASSERT( false );
    return NULL;
    }

} // namespace KHotKeys

