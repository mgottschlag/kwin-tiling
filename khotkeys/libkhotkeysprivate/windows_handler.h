/* Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   Version 2 as published by the Free Software Foundation;

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef WINDOWS_HANDLER_H
#define WINDOWS_HANDLER_H

#include <kdemacros.h>

#include <QtCore/QObject>
#include <QtGui/qwindowdefs.h>

#include <netwm.h>

namespace KHotKeys
{

const int SUPPORTED_WINDOW_TYPES_MASK = NET::NormalMask | NET::DesktopMask | NET::DockMask
    | NET::ToolbarMask | NET::MenuMask | NET::DialogMask | NET::OverrideMask | NET::TopMenuMask
    | NET::UtilityMask | NET::SplashMask;


class Windowdef_list;

class KDE_EXPORT WindowsHandler : public QObject
                                  #include <QtGui/qwindowdefs.h>
    {
    Q_OBJECT
    public:
        WindowsHandler( bool enable_signals_P, QObject* parent_P );
        virtual ~WindowsHandler();
        QString get_window_class( WId id_P );
        QString get_window_role( WId id_P );
        WId active_window();
        void set_action_window( WId window );
        WId action_window();
        WId find_window( const Windowdef_list* window_P );
        static WId window_at_position( int x, int y );
        static void activate_window( WId id_P );
    Q_SIGNALS:
        void window_added( WId window_P );
        void window_removed( WId window_P );
        void active_window_changed( WId window_P );
        void window_changed( WId window_P );
        void window_changed( WId window_P, unsigned int flags_P );
    protected Q_SLOTS:
        void window_added_slot( WId window_P );
        void window_removed_slot( WId window_P );
        void active_window_changed_slot( WId window_P );
        void window_changed_slot( WId window_P );
        void window_changed_slot( WId window_P, unsigned int flags_P );
    private:
        bool signals_enabled;
        WId _action_window;
    };


} // namespace KHotKeys

#endif
