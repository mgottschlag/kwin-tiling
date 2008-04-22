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
#ifndef LIBKHOTKEYSFWD_H
#define LIBKHOTKEYSFWD_H

namespace KHotKeys
    {
    class Action;
    class ActionDataBase;
    class ActionDataGroup;
    class Action_list;
    class Command_url_action;
    class Command_url_shortcut_action_data;
    class Dbus_action;
    class Menuentry_action;
    class Menuentry_shortcut_action_data;
    class Shortcut_trigger;
    class Simple_action_data_base;
    class Trigger;
    typedef Action_list ActionList;
    typedef Command_url_action CommandUrlAction;
    typedef Command_url_shortcut_action_data CommandUrlShortcutActionData;
    typedef Dbus_action DbusAction;
    typedef Menuentry_action MenuentryAction;
    typedef Menuentry_shortcut_action_data MenuentryShortcutActionData;
    typedef Shortcut_trigger ShortcutTrigger;
    typedef Simple_action_data_base SimpleActionData;
    };


#endif /* #ifndef LIBKHOTKEYSFWD_H */
