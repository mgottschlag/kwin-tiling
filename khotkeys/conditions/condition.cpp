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
#include <KDE/KDebug>

#ifdef KHOTKEYS_DEBUG
#include <typeinfo>
#endif

namespace KHotKeys {

Condition::Condition( Condition_list_base* parent_P )
    : _parent( parent_P )
    {
    if( _parent )
        _parent->append( this );
    }

Condition::Condition( KConfigGroup&, Condition_list_base* parent_P )
    : _parent( parent_P )
    {
    if( _parent )
        _parent->append( this );
    }

Condition* Condition::create_cfg_read( KConfigGroup& cfg_P, Condition_list_base* parent_P )
    {
    QString type = cfg_P.readEntry( "Type" );
    if( type == "ACTIVE_WINDOW" )
        return new Active_window_condition( cfg_P, parent_P );
    if( type == "EXISTING_WINDOW" )
        return new Existing_window_condition( cfg_P, parent_P );
    if( type == "NOT" )
        return new Not_condition( cfg_P, parent_P );
    if( type == "AND" )
        return new And_condition( cfg_P, parent_P );
    if( type == "OR" )
        return new Or_condition( cfg_P, parent_P );
    kWarning( 1217 ) << "Unknown Condition type read from cfg file\n";
    return NULL;
    }

Condition::~Condition()
    {
    if( _parent )
        _parent->remove( this );
    }


void Condition::cfg_write( KConfigGroup& cfg_P ) const
    {
    cfg_P.writeEntry( "Type", "ERROR" );
    }

void Condition::updated() const
    {
    if( !khotkeys_active())
        return;
    Q_ASSERT( _parent != NULL );
    _parent->updated();
    }

#ifdef KHOTKEYS_DEBUG
void Condition::debug( int depth_P )
    {
    char tmp[ 1024 ];
    int i;
    for( i = 0;
         i < depth_P;
         ++i )
        tmp[ i ] = ' ';
    tmp[ i ] = '\0';
    kDebug( 1217 ) << tmp << description() << ":(" << this << ")";
    }

void Condition::debug_list( const Q3PtrList< Condition >& list_P, int depth_P )
    {
    char tmp[ 1024 ];
    int i;
    for( i = 0;
         i < depth_P;
         ++i )
        tmp[ i ] = ' ';
    tmp[ i ] = '\0';
    for( Q3PtrListIterator< Condition > it( list_P );
         it;
         ++it )
        (*it)->debug( depth_P + 1 );
    }
#endif

} // namespace KHotKeys
