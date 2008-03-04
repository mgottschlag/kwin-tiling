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

#include "triggers.h"

#include <KDE/KConfigGroup>

namespace KHotKeys {

Trigger_list::Trigger_list( KConfigGroup& cfg_P, Action_data* data_P )
    : Q3PtrList< Trigger >()
    {
    setAutoDelete( true );
    _comment = cfg_P.readEntry( "Comment" );
    int cnt = cfg_P.readEntry( "TriggersCount", 0 );
    for( int i = 0;
         i < cnt;
         ++i )
        {
        KConfigGroup triggerConfig( cfg_P.config(), cfg_P.name() + QString::number( i ));
        Trigger* trigger = Trigger::create_cfg_read( triggerConfig, data_P );
        if( trigger )
            append( trigger );
        }
    }

void Trigger_list::cfg_write( KConfigGroup& cfg_P ) const
    {
    cfg_P.writeEntry( "Comment", comment());
    int i = 0;
    for( Iterator it( *this );
         it;
         ++it, ++i )
        {
        KConfigGroup triggerConfig( cfg_P.config(), cfg_P.name() + QString::number( i ));
        it.current()->cfg_write( triggerConfig );
        }
    cfg_P.writeEntry( "TriggersCount", i );
    }

Trigger_list* Trigger_list::copy( Action_data* data_P ) const
    {
    Trigger_list* ret = new Trigger_list( comment());
    for( Iterator it( *this );
         it;
         ++it )
        ret->append( it.current()->copy( data_P ));
    return ret;
    }

void Trigger_list::activate( bool activate_P )
    {
    for( Iterator it( *this );
         it;
         ++it )
        ( *it )->activate( activate_P );
    }

} // namespace KHotKeys

