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
#include <KDE/KDebug>

namespace KHotKeys {


And_condition::And_condition( KConfigGroup& cfg_P, Condition_list_base* parent_P )
    : Condition_list_base( cfg_P, parent_P )
    {
    }


And_condition::And_condition( Condition_list_base* parent_P )
    : Condition_list_base( parent_P )
    {
    }


void And_condition::cfg_write( KConfigGroup& cfg_P ) const
    {
    kDebug() << description() << " with " << count() << " children";;
    base::cfg_write( cfg_P );
    cfg_P.writeEntry( "Type", "AND" ); // overwrites value set in base::cfg_write()
    }


And_condition* And_condition::copy() const
    {
    And_condition* ret = new And_condition();
    for (ConstIterator it=begin(); it!=end(); ++it)
        {
        ret->append((*it)->copy());
        }
    return ret;
    }


const QString And_condition::description() const
    {
    return i18nc( "And_condition", "And" );
    }


bool And_condition::match() const
    {
    for (ConstIterator it=begin(); it!=end(); ++it)
        {
        if (!(*it)->match())
            {
            return false;
            }
        }
    return true; // all true (or empty)
    }


} // namespace KHotKeys
