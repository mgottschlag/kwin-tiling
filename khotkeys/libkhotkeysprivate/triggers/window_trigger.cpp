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

#include "triggers/triggers.h"
#include "action_data/action_data.h"
#include "windows_handler.h"
#include "windows_helper/window_selection_list.h"

#include <KDE/KConfigGroup>
#include <KDE/KDebug>

#include <X11/X.h>

namespace KHotKeys {


WindowTriggerVisitor::~WindowTriggerVisitor()
    {}


WindowTrigger::WindowTrigger(
        ActionData* data_P,
        Windowdef_list* windows_P,
        WindowEvents window_actions_P)
    :   Trigger( data_P ),
        _windows( windows_P ),
        window_actions( window_actions_P ),
        existing_windows(),
        last_active_window( None ),
        active( true )
    {
    if (!_windows)
        {
        _windows = new Windowdef_list( "Windowdef_list comment");
        }

    Q_ASSERT(_windows->isEmpty());
    init();
    }


WindowTrigger::~WindowTrigger()
    {
//    kDebug() << "~WindowTrigger :" << this;
    disconnect( windows_handler, NULL, this, NULL );
    delete _windows;
    }


void WindowTrigger::accept(TriggerVisitor& visitor)
    {
    if (WindowTriggerVisitor *v = dynamic_cast<WindowTriggerVisitor*>(&visitor))
        {
        v->visit(*this);
        }
    else
        {
        kDebug() << "Visitor error";
        }
    }


void WindowTrigger::init()
    {
    kDebug() << "WindowTrigger::init()";
    connect( windows_handler, SIGNAL(window_added(WId)), this, SLOT(window_added(WId)));
    connect( windows_handler, SIGNAL(window_removed(WId)), this, SLOT(window_removed(WId)));
    if( window_actions & ( WINDOW_ACTIVATES | WINDOW_DEACTIVATES /*| WINDOW_DISAPPEARS*/ ))
        connect( windows_handler, SIGNAL(active_window_changed(WId)),
            this, SLOT(active_window_changed(WId)));
    connect( windows_handler, SIGNAL(window_changed(WId,uint)),
        this, SLOT(window_changed(WId,uint)));
    }


void WindowTrigger::activate( bool activate_P )
    {
    active = activate_P;
    }


void WindowTrigger::active_window_changed( WId window_P )
    {
    if (!existing_windows.contains(window_P))
        {
        existing_windows[window_P] = windows()->match( Window_data( window_P ));
        }

    if (!active || !khotkeys_active())
        {
        // We still keep track of the last active window so we have valid data
        // if khotkeys is switched on again.
        last_active_window = window_P;
        return;
        }

    // Check if the last active window was a match for us
    bool was_match = existing_windows.contains(last_active_window)
        ? existing_windows[last_active_window]
        : false;

    if (was_match && (window_actions & WINDOW_DEACTIVATES))
        {
        windows_handler->set_action_window( window_P );
        data->execute();
        }

    if (existing_windows[window_P] && ( window_actions & WINDOW_ACTIVATES))
        {
        kDebug() << "Executing data";
        windows_handler->set_action_window( window_P );
        data->execute();
        }

    last_active_window = window_P;
    }


void WindowTrigger::cfg_write( KConfigGroup& cfg_P ) const
    {
    base::cfg_write( cfg_P );
    KConfigGroup windowsConfig( cfg_P.config(), cfg_P.name() + "Windows" );
    if (windows())
        {
        windows()->cfg_write( windowsConfig );
        }
    cfg_P.writeEntry( "WindowActions", static_cast<int>(window_actions));
    cfg_P.writeEntry( "Type", "WINDOW" ); // overwrites value set in base::cfg_write()
    }


const QString WindowTrigger::description() const
    {
    return i18n( "Window trigger: " ) + windows()->comment();
    }


void WindowTrigger::setOnWindowEvents(WindowEvents events)
    {
    window_actions = events;
    }


void WindowTrigger::set_window_rules(Windowdef_list *list)
    {
     delete _windows;
    _windows = list;
    }


bool WindowTrigger::triggers_on( window_action_t w_action_P ) const
    {
    return window_actions & w_action_P;
    }


void WindowTrigger::window_added( WId window_P )
    {
    // Always keep track of windows,
    existing_windows[window_P] = windows()->match( Window_data( window_P ));

    if (!active || !khotkeys_active())
        {
        return;
        }

    if (existing_windows[window_P] && (window_actions & WINDOW_APPEARS))
        {
        windows_handler->set_action_window(window_P);
        data->execute();
        }
    }


void WindowTrigger::window_removed( WId window_P )
    {
    // Always keep track of windows,
    bool matches = false;
    if (existing_windows.contains(window_P))
        {
        matches = existing_windows[window_P];
        existing_windows.remove( window_P );
        }

    if (!active || !khotkeys_active())
        {
        return;
        }

    if (matches && (window_actions & WINDOW_DISAPPEARS))
        {
        windows_handler->set_action_window( window_P );
        data->execute();
        }
    }


void WindowTrigger::window_changed( WId window_P, unsigned int dirty_P )
    {
    if (! (dirty_P & (NET::WMName | NET::WMWindowType)))
        return;

    // Check if the old state was a match
    bool was_match = false;
    if (existing_windows.contains(window_P))
        was_match = existing_windows[window_P];

    // Check if the new state is a match
    bool matches = windows()->match( Window_data( window_P ));
    existing_windows[window_P] = matches;

    if (!active || !khotkeys_active())
        {
        return;
        }

    if (matches && !was_match)
        {
        if (window_actions & WINDOW_APPEARS)
            {
            windows_handler->set_action_window( window_P );
            data->execute();
            }
        else if (window_actions & WINDOW_ACTIVATES && window_P == windows_handler->active_window())
            {
            windows_handler->set_action_window( window_P );
            data->execute();
            }
        }
    }


const Windowdef_list* WindowTrigger::windows() const
    {
    return _windows;
    }


Windowdef_list* WindowTrigger::windows()
    {
    return _windows;
    }


WindowTrigger* WindowTrigger::copy( ActionData* data_P ) const
    {
    WindowTrigger* ret = new WindowTrigger( data_P ? data_P : data, windows()->copy(),
        window_actions );
    ret->existing_windows = existing_windows; // CHECKME je tohle vazne treba ?
    return ret;
    }

} // namespace KHotKeys

