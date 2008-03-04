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

namespace KHotKeys {

Existing_window_condition::Existing_window_condition( KConfigGroup& cfg_P, Condition_list_base* parent_P )
    : Condition( cfg_P, parent_P )
    {
    KConfigGroup windowConfig( cfg_P.config(), cfg_P.name() + "Window" );
    _window = new Windowdef_list( windowConfig );
    init();
    set_match();
    }

void Existing_window_condition::init()
    {
    connect( windows_handler, SIGNAL( window_added( WId )), this, SLOT( window_added( WId )));
    connect( windows_handler, SIGNAL( window_removed( WId )), this, SLOT( window_removed( WId )));
    }

bool Existing_window_condition::match() const
    {
    return is_match;
    }

void Existing_window_condition::set_match( WId w_P )
    {
    if( w_P != None && !is_match )
        is_match = window()->match( Window_data( w_P ));
    else
        is_match = windows_handler->find_window( window()) != None;
    kDebug( 1217 ) << "Existing_window_condition::set_match :" << is_match;
    updated();
    }

void Existing_window_condition::cfg_write( KConfigGroup& cfg_P ) const
    {
    base::cfg_write( cfg_P );
    KConfigGroup windowConfig( cfg_P.config(), cfg_P.name() + "Window" );
    window()->cfg_write( windowConfig );
    cfg_P.writeEntry( "Type", "EXISTING_WINDOW" ); // overwrites value set in base::cfg_write()
    }

#ifdef HAVE_COVARIANT_RETURN
Existing_window_condition* Existing_window_condition::copy( Condition_list_base* parent_P ) const
#else
Condition* Existing_window_condition::copy( Condition_list_base* parent_P ) const
#endif
    {
    return new Existing_window_condition( window()->copy(), parent_P );
    }

const QString Existing_window_condition::description() const
    {
    return i18n( "Existing window: " ) + window()->comment();
    }

void Existing_window_condition::window_added( WId w_P )
    {
    set_match( w_P );
    }

void Existing_window_condition::window_removed( WId )
    {
    set_match();
    }

Existing_window_condition::~Existing_window_condition()
    {
    disconnect( windows_handler, NULL, this, NULL );
    delete _window;
    }

} // namespace KHotKeys
