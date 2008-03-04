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

#include <KDE/KConfigGroup>

namespace KHotKeys {

Or_condition::Or_condition( KConfigGroup& cfg_P, Condition_list_base* parent_P )
    : Condition_list_base( cfg_P, parent_P )
    {
    // CHECKME kontrola poctu ?
    }

bool Or_condition::match() const
    {
    if( count() == 0 ) // empty => ok
        return true;
    for( Iterator it( *this );
         it;
         ++it )
        if( it.current()->match()) // OR
            return true;
    return false;
    }

void Or_condition::cfg_write( KConfigGroup& cfg_P ) const
    {
    base::cfg_write( cfg_P );
    cfg_P.writeEntry( "Type", "OR" ); // overwrites value set in base::cfg_write()
    }

Or_condition* Or_condition::copy( Condition_list_base* parent_P ) const
    {
    Or_condition* ret = new Or_condition( parent_P );
    for( Iterator it( *this );
         it;
         ++it )
        ret->append( (*it)->copy( ret ));
    return ret;
    }

const QString Or_condition::description() const
    {
    return i18nc( "Or_condition", "Or" );
    }

} // namespace KHotKeys

