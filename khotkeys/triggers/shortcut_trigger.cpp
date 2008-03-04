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
#include "action_data.h"
#include "windows.h"

#include <KDE/KConfigGroup>
#include <KDE/KDebug>

namespace KHotKeys {

Shortcut_trigger::Shortcut_trigger( Action_data* data_P, const KShortcut& shortcut_P )
    : Trigger( data_P ), _shortcut( shortcut_P )
    {
    keyboard_handler->insert_item( shortcut(), this );
    }

Shortcut_trigger::Shortcut_trigger( KConfigGroup& cfg_P, Action_data* data_P )
    : Trigger( cfg_P, data_P ), _shortcut( cfg_P.readEntry( "Key", QString() ))
    {
    keyboard_handler->insert_item( shortcut(), this );
    }

Shortcut_trigger::~Shortcut_trigger()
    {
    keyboard_handler->remove_item( shortcut(), this );
    }

void Shortcut_trigger::cfg_write( KConfigGroup& cfg_P ) const
    {
    base::cfg_write( cfg_P );
    cfg_P.writeEntry( "Key", _shortcut.toString());
    cfg_P.writeEntry( "Type", "SHORTCUT" ); // overwrites value set in base::cfg_write()
    }

Shortcut_trigger* Shortcut_trigger::copy( Action_data* data_P ) const
    {
    kDebug( 1217 ) << "Shortcut_trigger::copy()";
    return new Shortcut_trigger( data_P ? data_P : data, shortcut());
    }

const QString Shortcut_trigger::description() const
    {
    // CHECKME vice mods
    return i18n( "Shortcut trigger: " ) + _shortcut.toString();
    // CHECKME i18n pro toString() ?
    }

bool Shortcut_trigger::handle_key( const KShortcut& shortcut_P )
    {
    if( shortcut() == shortcut_P )
        {
        windows_handler->set_action_window( 0 ); // use active window
        data->execute();
        return true;
        }
    return false;
    }

void Shortcut_trigger::activate( bool activate_P )
    {
    if( activate_P && khotkeys_active())
        keyboard_handler->activate_receiver( this );
    else
        keyboard_handler->deactivate_receiver( this );
    }

} // namespace KHotKeys
