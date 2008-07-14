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

#include "actions.h"

#include <KDE/KConfigGroup>
#include <KDE/KDebug>

namespace KHotKeys {


Action::Action( ActionData* data_P )
    : data( data_P )
    {
    }


Action::~Action()
    {
    }


void Action::aboutToBeErased()
    {
    // Nothing to do yet.
    }

Action::Action( KConfigGroup&, ActionData* data_P )
    : data( data_P )
    {
    }


Action* Action::create_cfg_read( KConfigGroup& cfg_P, ActionData* data_P )
    {
    QString type = cfg_P.readEntry( "Type" );
    if( type == "COMMAND_URL" )
        return new CommandUrlAction( cfg_P, data_P );
    if( type == "MENUENTRY" )
        return new MenuEntryAction( cfg_P, data_P );
    if( type == "DCOP" || type == "DBUS" )
        return new DBusAction( cfg_P, data_P );
    if( type == "KEYBOARD_INPUT" )
        return new KeyboardInputAction( cfg_P, data_P );
    if( type == "ACTIVATE_WINDOW" )
        return new ActivateWindowAction( cfg_P, data_P );
    kWarning( 1217 ) << "Unknown Action type read from cfg file\n";
    return NULL;
    }


void Action::cfg_write( KConfigGroup& cfg_P ) const
    {
    cfg_P.writeEntry( "Type", "ERROR" ); // derived classes should call with their type
    }


} // namespace KHotKeys

