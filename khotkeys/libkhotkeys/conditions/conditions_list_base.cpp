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

#ifdef KHOTKEYS_DEBUG
#include <typeinfo>
#endif

namespace KHotKeys {

Condition_list_base::Condition_list_base( KConfigGroup& cfg_P, Condition_list_base* parent_P )
    : Condition( parent_P )
    {
    int cnt = cfg_P.readEntry( "ConditionsCount", 0 );
    for( int i = 0;
         i < cnt;
         ++i )
        {
        KConfigGroup conditionConfig( cfg_P.config(), cfg_P.name() + QString::number( i ) );
        (void) Condition::create_cfg_read( conditionConfig, this );
        }
    }

Condition_list_base::~Condition_list_base()
    {
    while( !isEmpty())
        {
        Condition* c = getFirst();
        remove( c );
        delete c;
        }
    }

void Condition_list_base::cfg_write( KConfigGroup& cfg_P ) const
    {
    int i = 0;
    for( Iterator it( *this );
         it;
         ++it, ++i )
        {
        KConfigGroup conditionConfig( cfg_P.config(), cfg_P.name() + QString::number( i ) );
        it.current()->cfg_write( conditionConfig );
        }
    cfg_P.writeEntry( "ConditionsCount", i );
    }

bool Condition_list_base::accepts_children() const
    {
    return true;
    }

#ifdef KHOTKEYS_DEBUG
void Condition_list_base::debug( int depth_P )
    {
    char tmp[ 1024 ];
    int i;
    for( i = 0;
         i < depth_P;
         ++i )
        tmp[ i ] = ' ';
    tmp[ i ] = '\0';
    kDebug( 1217 ) << tmp << typeid( *this ).name() << ":(" << this << ")";
    debug_list( *this, depth_P + 1 );
    }
#endif

} // namespace KHotKeys

