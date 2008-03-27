/*
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
#include "windows.h"

#include <KDE/KConfigGroup>

// Has to be behind all qt related stuff. Else the build fails miserably
#include <X11/X.h>

namespace KHotKeys {


Activate_window_action::Activate_window_action( Action_data* data_P,
    const Windowdef_list* window_P )
    : Action( data_P ), _window( window_P )
    {
    }


Activate_window_action::Activate_window_action( KConfigGroup& cfg_P, Action_data* data_P )
    : Action( cfg_P, data_P )
    {
    QString save_cfg_group = cfg_P.name();
    KConfigGroup windowGroup( cfg_P.config(), save_cfg_group + "Window" );
    _window = new Windowdef_list( windowGroup );
    }


const Windowdef_list* Activate_window_action::window() const
    {
    return _window;
    }


Activate_window_action::~Activate_window_action()
    {
    delete _window;
    }


void Activate_window_action::cfg_write( KConfigGroup& cfg_P ) const
    {
    base::cfg_write( cfg_P );
    cfg_P.writeEntry( "Type", "ACTIVATE_WINDOW" ); // overwrites value set in base::cfg_write()
    KConfigGroup windowGroup( cfg_P.config(), cfg_P.name() + "Window" );
    window()->cfg_write( windowGroup );
    }


void Activate_window_action::execute()
    {
    if( window()->match( windows_handler->active_window()))
        return; // is already active
    WId win_id = windows_handler->find_window( window());
    if( win_id != None )
        windows_handler->activate_window( win_id );
    }


const QString Activate_window_action::description() const
    {
    return i18n( "Activate window : " ) + window()->comment();
    }


Action* Activate_window_action::copy( Action_data* data_P ) const
    {
    return new Activate_window_action( data_P, window()->copy());
    }

} // namespace KHotKeys

