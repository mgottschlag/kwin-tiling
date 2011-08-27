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

#include "conditions/active_window_condition.h"

#include "windows_helper/window_selection_list.h"
#include "windows_handler.h"
#include "khotkeysglobal.h"

#include <KDE/KConfig>
#include <KDE/KConfigGroup>
#include <KDE/KDebug>

namespace KHotKeys {

Active_window_condition::Active_window_condition( KConfigGroup& cfg_P, Condition_list_base* parent_P )
    : Condition( cfg_P, parent_P )
    {
    KConfigGroup windowConfig( cfg_P.config(), cfg_P.name() + "Window" );
    _window = new Windowdef_list( windowConfig );
    init();
    set_match();
    }


Active_window_condition::Active_window_condition( Windowdef_list* window_P,
    Condition_list_base* parent_P )
    : Condition( parent_P ), _window( window_P )
    {
    init();
    set_match();
    }


Active_window_condition::~Active_window_condition()
    {
    disconnect( windows_handler, NULL, this, NULL );
    delete _window;
    }


void Active_window_condition::active_window_changed( WId )
    {
    set_match();
    }


void Active_window_condition::cfg_write( KConfigGroup& cfg_P ) const
    {
    base::cfg_write( cfg_P );
    KConfigGroup windowConfig( cfg_P.config(), cfg_P.name() + "Window" );
    window()->cfg_write( windowConfig );
    cfg_P.writeEntry( "Type", "ACTIVE_WINDOW" ); // overwrites value set in base::cfg_write()
    }


Active_window_condition* Active_window_condition::copy() const
    {
    kDebug();
    return new Active_window_condition(window()->copy());
    }


const QString Active_window_condition::description() const
    {
    return i18n( "Active window: " ) + window()->comment();
    }


void Active_window_condition::init()
    {
    connect( windows_handler, SIGNAL(active_window_changed(WId)),
        this, SLOT(active_window_changed(WId)));
    }


bool Active_window_condition::match() const
    {
    return is_match;
    }


void Active_window_condition::set_match()
    {
    is_match = window()->match( Window_data( windows_handler->active_window()));
    updated();
    }


const Windowdef_list* Active_window_condition::window() const
    {
    return _window;
    }


Windowdef_list* Active_window_condition::window()
    {
    return _window;
    }


} // namespace KHotKeys

