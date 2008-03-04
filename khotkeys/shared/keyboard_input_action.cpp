/*
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

#include "actions.h"
#include "input.h"
#include "windows.h"


#include <KDE/KConfigGroup>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <QX11Info>

namespace KHotKeys {

Keyboard_input_action::Keyboard_input_action( KConfigGroup& cfg_P, Action_data* data_P )
    : Action( cfg_P, data_P )
    {
    _input = cfg_P.readEntry( "Input" );
    if( cfg_P.readEntry( "IsDestinationWindow" , false))
        {
        KConfigGroup windowGroup( cfg_P.config(), cfg_P.name() + "DestinationWindow" );
        _dest_window = new Windowdef_list( windowGroup );
        _active_window = false; // ignored with _dest_window set anyway
        }
    else
        {
        _dest_window = NULL;
        _active_window = cfg_P.readEntry( "ActiveWindow" , false);
        }
    }

Keyboard_input_action::~Keyboard_input_action()
    {
    delete _dest_window;
    }

void Keyboard_input_action::cfg_write( KConfigGroup& cfg_P ) const
    {
    base::cfg_write( cfg_P );
    cfg_P.writeEntry( "Type", "KEYBOARD_INPUT" ); // overwrites value set in base::cfg_write()
    cfg_P.writeEntry( "Input", input());
    if( dest_window() != NULL )
        {
        cfg_P.writeEntry( "IsDestinationWindow", true );
        KConfigGroup windowGroup( cfg_P.config(), cfg_P.name() + "DestinationWindow" );
        dest_window()->cfg_write( windowGroup );
        }
    else
        cfg_P.writeEntry( "IsDestinationWindow", false );
    cfg_P.writeEntry( "ActiveWindow", _active_window );
    }

void Keyboard_input_action::execute()
    {
    if( input().isEmpty())
        return;
    Window w = InputFocus;
    if( dest_window() != NULL )
        {
        w = windows_handler->find_window( dest_window());
        if( w == None )
            w = InputFocus;
        }
    else
        {
        if( !_active_window )
            w = windows_handler->action_window();
        if( w == None )
            w = InputFocus;
        }
    int last_index = -1, start = 0;
    while(( last_index = input().indexOf( ':', last_index + 1 )) != -1 ) // find next ';'
        {
        QString key = input().mid( start, last_index - start ).trimmed();
        keyboard_handler->send_macro_key( key, w );
        start = last_index + 1;
        }
    // and the last one
    QString key = input().mid( start, input().length()).trimmed();
    keyboard_handler->send_macro_key( key, w ); // the rest
    XFlush( QX11Info::display());
    }

const QString Keyboard_input_action::description() const
    {
    QString tmp = input();
    tmp.replace( '\n', ' ' );
    tmp.truncate( 30 );
    return i18n( "Keyboard input : " ) + tmp;
    }

Action* Keyboard_input_action::copy( Action_data* data_P ) const
    {
    return new Keyboard_input_action( data_P, input(),
        dest_window() ? dest_window()->copy() : NULL, _active_window );
    }

} // namespace KHotKeys

