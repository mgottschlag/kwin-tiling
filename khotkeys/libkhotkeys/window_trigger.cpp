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

#include "triggers.h"
#include "action_data.h"
#include "windows.h"

#include "config-khotkeys.h"

#include <KDE/KConfigGroup>
#include <KDE/KDebug>

namespace KHotKeys {

Window_trigger::Window_trigger( KConfigGroup& cfg_P, Action_data* data_P )
    : Trigger( cfg_P, data_P ), active( false )
    {
//    kDebug( 1217 ) << "Window_trigger";
    KConfigGroup windowsConfig( cfg_P.config(), cfg_P.name() + "Windows" );
    _windows = new Windowdef_list( windowsConfig );
    window_actions = cfg_P.readEntry( "WindowActions",0 );
    init();
    }

Window_trigger::~Window_trigger()
    {
//    kDebug( 1217 ) << "~Window_trigger :" << this;
    disconnect( windows_handler, NULL, this, NULL );
    delete _windows;
    }

void Window_trigger::init()
    {
    kDebug( 1217 ) << "Window_trigger::init()";
    connect( windows_handler, SIGNAL( window_added( WId )), this, SLOT( window_added( WId )));
    connect( windows_handler, SIGNAL( window_removed( WId )), this, SLOT( window_removed( WId )));
    if( window_actions & ( WINDOW_ACTIVATES | WINDOW_DEACTIVATES /*| WINDOW_DISAPPEARS*/ ))
        connect( windows_handler, SIGNAL( active_window_changed( WId )),
            this, SLOT( active_window_changed( WId )));
    connect( windows_handler, SIGNAL( window_changed( WId, unsigned int )),
        this, SLOT( window_changed( WId, unsigned int )));
    }

void Window_trigger::activate( bool activate_P )
    {
    active = activate_P && khotkeys_active();
    }

void Window_trigger::window_added( WId window_P )
    {
    bool matches = windows()->match( Window_data( window_P ));
    existing_windows[ window_P ] = matches;
    kDebug( 1217 ) << "Window_trigger::w_added() : " << matches;
    if( active && matches && ( window_actions & WINDOW_APPEARS ))
        {
        windows_handler->set_action_window( window_P );
        data->execute();
        }
    }

void Window_trigger::window_removed( WId window_P )
    {
    if( existing_windows.contains( window_P ))
        {
        bool matches = existing_windows[ window_P ];
        kDebug( 1217 ) << "Window_trigger::w_removed() : " << matches;
        if( active && matches && ( window_actions & WINDOW_DISAPPEARS ))
            {
            windows_handler->set_action_window( window_P );
            data->execute();
            }
        existing_windows.remove( window_P );
        // CHECKME jenze co kdyz se window_removed zavola pred active_window_changed ?
        }
    else
        kDebug( 1217 ) << "Window_trigger::w_removed()";
    }

void Window_trigger::active_window_changed( WId window_P )
    {
    bool was_match = false;
    if( existing_windows.contains( last_active_window ))
        was_match = existing_windows[ last_active_window ];
    if( active && was_match && ( window_actions & WINDOW_DEACTIVATES ))
        {
        windows_handler->set_action_window( window_P );
        data->execute();
        }
/*    bool matches = windows()->match( Window_data( window_P ));
    existing_windows[ window_P ] = matches;*/
    bool matches = existing_windows.contains( window_P )
        ? existing_windows[ window_P ] : false;
    if( active && matches && ( window_actions & WINDOW_ACTIVATES ))
        {
        windows_handler->set_action_window( window_P );
        data->execute();
        }
    kDebug( 1217 ) << "Window_trigger::a_w_changed() : " << was_match << "|" << matches;
    last_active_window = window_P;
    }

void Window_trigger::window_changed( WId window_P, unsigned int dirty_P )
    { // CHECKME snad nebude mit vliv, kdyz budu kaslat na properties_P a zkratka
      // kontrolovat kazdou zmenu
      // CHECKME kdyz se zmeni okno z match na non-match, asi to nebrat jako DISAPPEAR
    if( ! ( dirty_P & ( NET::WMName | NET::WMWindowType )))
        return;
    kDebug( 1217 ) << "Window_trigger::w_changed()";
    bool was_match = false;
    if( existing_windows.contains( window_P ))
        was_match = existing_windows[ window_P ];
    bool matches = windows()->match( Window_data( window_P ));
    existing_windows[ window_P ] = matches;
    if( active && matches && !was_match )
        if( window_actions & WINDOW_APPEARS )
            {
            windows_handler->set_action_window( window_P );
            data->execute();
            }
        else if( window_actions & WINDOW_ACTIVATES && window_P == windows_handler->active_window())
            {
            windows_handler->set_action_window( window_P );
            data->execute();
            }
    kDebug( 1217 ) << "Window_trigger::w_changed() : " << was_match << "|" << matches;
    }

void Window_trigger::cfg_write( KConfigGroup& cfg_P ) const
    {
    base::cfg_write( cfg_P );
    KConfigGroup windowsConfig( cfg_P.config(), cfg_P.name() + "Windows" );
    windows()->cfg_write( windowsConfig );
    cfg_P.writeEntry( "WindowActions", window_actions );
    cfg_P.writeEntry( "Type", "WINDOW" ); // overwrites value set in base::cfg_write()
    }

#ifdef HAVE_COVARIANT_RETURN    // stupid gcc, it doesn't even warn it can't do this
Window_trigger* Window_trigger::copy( Action_data* data_P ) const
#else
Trigger* Window_trigger::copy( Action_data* data_P ) const
#endif
    {
    Window_trigger* ret = new Window_trigger( data_P ? data_P : data, windows()->copy(),
        window_actions );
    ret->existing_windows = existing_windows; // CHECKME je tohle vazne treba ?
    return ret;
    }

const QString Window_trigger::description() const
    {
    return i18n( "Window trigger: " ) + windows()->comment();
    }

} // namespace KHotKeys

