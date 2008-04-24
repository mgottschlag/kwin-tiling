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

#include "input.h"
#include <KDE/KConfigGroup>

#include "windows.h"

#include "shortcuts_handler.h"

#include <X11/X.h>
#include <X11/Xlib.h>
#include <QX11Info>


namespace KHotKeys {

KeyboardInputAction::KeyboardInputAction( ActionData* data_P, const QString& input_P,
    const Windowdef_list* dest_window_P, bool active_window_P )
    : Action( data_P ), _input( input_P ), _dest_window( dest_window_P ), _active_window( active_window_P )
    {
    }


KeyboardInputAction::KeyboardInputAction( KConfigGroup& cfg_P, ActionData* data_P )
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


KeyboardInputAction::~KeyboardInputAction()
    {
    delete _dest_window;
    }


const QString& KeyboardInputAction::input() const
    {
    return _input;
    }


const Windowdef_list* KeyboardInputAction::dest_window() const
    {
    return _dest_window;
    }


bool KeyboardInputAction::activeWindow() const
    {
    return _active_window;
    }


void KeyboardInputAction::cfg_write( KConfigGroup& cfg_P ) const
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


void KeyboardInputAction::execute()
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


const QString KeyboardInputAction::description() const
    {
    QString tmp = input();
    tmp.replace( '\n', ' ' );
    tmp.truncate( 30 );
    return i18n( "Keyboard input: " ) + tmp;
    }


Action* KeyboardInputAction::copy( ActionData* data_P ) const
    {
    return new KeyboardInputAction( data_P, input(),
        dest_window() ? dest_window()->copy() : NULL, _active_window );
    }

} // namespace KHotKeys

