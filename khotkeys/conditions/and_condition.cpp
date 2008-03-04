/*
   Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>
   Copyright (C) 2008 Michael Jansen <kde@michael-jansen.biz>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

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

#include <KDE/KConfigGroup>

namespace KHotKeys {

And_condition::And_condition( KConfigGroup& cfg_P, Condition_list_base* parent_P )
    : Condition_list_base( cfg_P, parent_P )
    {
    // CHECKME kontrola poctu ?
    }

bool And_condition::match() const
    {
    for( Iterator it( *this );
         it;
         ++it )
        if( !it.current()->match()) // AND
            return false;
    return true; // all true (or empty)
    }

void And_condition::cfg_write( KConfigGroup& cfg_P ) const
    {
    base::cfg_write( cfg_P );
    cfg_P.writeEntry( "Type", "AND" ); // overwrites value set in base::cfg_write()
    }

And_condition* And_condition::copy( Condition_list_base* parent_P ) const
    {
    And_condition* ret = new And_condition( parent_P );
    for( Iterator it( *this );
         it;
         ++it )
        ret->append( (*it)->copy( ret ));
    return ret;
    }

const QString And_condition::description() const
    {
    return i18nc( "And_condition", "And" );
    }

} // namespace KHotKeys

