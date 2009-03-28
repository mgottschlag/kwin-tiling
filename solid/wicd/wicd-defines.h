/*  This file is part of the KDE project
    Copyright (C) 2008 Dario Freddi <drf54321@gmail.com>

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

#ifndef WICDDEFINES_H
#define WICDDEFINES_H

// All Wicd Definitions will be stored in here

#define WICD_DBUS_SERVICE "org.wicd.daemon"
#define WICD_DAEMON_DBUS_PATH "/org/wicd/daemon"
#define WICD_DAEMON_DBUS_INTERFACE "org.wicd.daemon"
#define WICD_WIRED_DBUS_PATH "/org/wicd/daemon/wired"
#define WICD_WIRED_DBUS_INTERFACE "org.wicd.daemon.wired"
#define WICD_WIRELESS_DBUS_PATH "/org/wicd/daemon/wireless"
#define WICD_WIRELESS_DBUS_INTERFACE "org.wicd.daemon.wireless"

namespace Wicd
{
enum ConnectionStatus {
    NOT_CONNECTED = 0,
    CONNECTING = 1,
    WIRELESS = 2,
    WIRED = 3,
    SUSPENDED = 4,
    Unknown = 16
};
}

#endif /* WICDDEFINES_H */
