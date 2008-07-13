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

#include <X11/X.h>

namespace KHotKeys {

WindowTrigger::WindowTrigger( ActionData* data_P, Windowdef_list* windows_P,
     int window_actions_P )
    : Trigger( data_P ), _windows( windows_P ), window_actions( window_actions_P ),
      last_active_window( None ), active( false )
    {
    init();
    }


WindowTrigger::WindowTrigger( KConfigGroup& cfg_P, ActionData* data_P )
    : Trigger( cfg_P, data_P ), active( false )
    {
//    kDebug( 1217 ) << "WindowTrigger";
    KConfigGroup windowsConfig( cfg_P.config(), cfg_P.name() + "Windows" );
    _windows = new Windowdef_list( windowsConfig );
    window_actions = cfg_P.readEntry( "WindowActions",0 );
    init();
    }


WindowTrigger::~WindowTrigger()
    {
//    kDebug( 1217 ) << "~WindowTrigger :" << this;
    disconnect( windows_handler, NULL, this, NULL );
    delete _windows;
    }

void WindowTrigger::init()
    {
    kDebug( 1217 ) << "WindowTrigger::init()";
    connect( windows_handler, SIGNAL( window_added( WId )), this, SLOT( window_added( WId )));
    connect( windows_handler, SIGNAL( window_removed( WId )), this, SLOT( window_removed( WId )));
    if( window_actions & ( WINDOW_ACTIVATES | WINDOW_DEACTIVATES /*| WINDOW_DISAPPEARS*/ ))
        connect( windows_handler, SIGNAL( active_window_changed( WId )),
            this, SLOT( active_window_changed( WId )));
    connect( windows_handler, SIGNAL( window_changed( WId, unsigned int )),
        this, SLOT( window_changed( WId, unsigned int )));
    }


void WindowTrigger::activate( bool activate_P )
    {
    active = activate_P && khotkeys_active();
    }


void WindowTrigger::active_window_changed( WId window_P )
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
    kDebug( 1217 ) << "WindowTrigger::a_w_changed() : " << was_match << "|" << matches;
    last_active_window = window_P;
    }


void WindowTrigger::cfg_write( KConfigGroup& cfg_P ) const
    {
    base::cfg_write( cfg_P );
    KConfigGroup windowsConfig( cfg_P.config(), cfg_P.name() + "Windows" );
    windows()->cfg_write( windowsConfig );
    cfg_P.writeEntry( "WindowActions", window_actions );
    cfg_P.writeEntry( "Type", "WINDOW" ); // overwrites value set in base::cfg_write()
    }


const QString WindowTrigger::description() const
    {
    return i18n( "Window trigger: " ) + windows()->comment();
    }


bool WindowTrigger::triggers_on( window_action_t w_action_P ) const
    {
    return window_actions & w_action_P;
    }


void WindowTrigger::window_added( WId window_P )
    {
    bool matches = windows()->match( Window_data( window_P ));
    existing_windows[ window_P ] = matches;
    kDebug( 1217 ) << "WindowTrigger::w_added() : " << matches;
    if( active && matches && ( window_actions & WINDOW_APPEARS ))
        {
        windows_handler->set_action_window( window_P );
        data->execute();
        }
    }


void WindowTrigger::window_removed( WId window_P )
    {
    if( existing_windows.contains( window_P ))
        {
        bool matches = existing_windows[ window_P ];
        kDebug( 1217 ) << "WindowTrigger::w_removed() : " << matches;
        if( active && matches && ( window_actions & WINDOW_DISAPPEARS ))
            {
            windows_handler->set_action_window( window_P );
            data->execute();
            }
        existing_windows.remove( window_P );
        // CHECKME jenze co kdyz se window_removed zavola pred active_window_changed ?
        }
    else
        kDebug( 1217 ) << "WindowTrigger::w_removed()";
    }


void WindowTrigger::window_changed( WId window_P, unsigned int dirty_P )
    { // CHECKME snad nebude mit vliv, kdyz budu kaslat na properties_P a zkratka
      // kontrolovat kazdou zmenu
      // CHECKME kdyz se zmeni okno z match na non-match, asi to nebrat jako DISAPPEAR
    if( ! ( dirty_P & ( NET::WMName | NET::WMWindowType )))
        return;
    kDebug( 1217 ) << "WindowTrigger::w_changed()";
    bool was_match = false;
    if( existing_windows.contains( window_P ))
        was_match = existing_windows[ window_P ];
    bool matches = windows()->match( Window_data( window_P ));
    existing_windows[ window_P ] = matches;
    if( active && matches && !was_match )
        {
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
        }
    kDebug( 1217 ) << "WindowTrigger::w_changed() : " << was_match << "|" << matches;
    }


const Windowdef_list* WindowTrigger::windows() const
    {
    return _windows;
    }


#ifdef HAVE_COVARIANT_RETURN    // stupid gcc, it doesn't even warn it can't do this
WindowTrigger* WindowTrigger::copy( ActionData* data_P ) const
#else
Trigger* WindowTrigger::copy( ActionData* data_P ) const
#endif
    {
    WindowTrigger* ret = new WindowTrigger( data_P ? data_P : data, windows()->copy(),
        window_actions );
    ret->existing_windows = existing_windows; // CHECKME je tohle vazne treba ?
    return ret;
    }

} // namespace KHotKeys

