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

#include "wicddbusinterface.h"

#include "wicd-defines.h"

#include <kglobal.h>

class WicdDbusInterface::Private
{
public:
    Private()
            : manager(WICD_DBUS_SERVICE, WICD_DAEMON_DBUS_PATH, WICD_DAEMON_DBUS_INTERFACE, QDBusConnection::systemBus())
            , wireless(WICD_DBUS_SERVICE, WICD_WIRELESS_DBUS_PATH, WICD_WIRELESS_DBUS_INTERFACE, QDBusConnection::systemBus())
            , wired(WICD_DBUS_SERVICE, WICD_WIRED_DBUS_PATH, WICD_WIRED_DBUS_INTERFACE, QDBusConnection::systemBus()) {};

    QDBusInterface manager;
    QDBusInterface wireless;
    QDBusInterface wired;
};

class WicdDbusInterfaceHelper
{
public:
    WicdDbusInterfaceHelper() : q(0) {}
    ~WicdDbusInterfaceHelper() {
        delete q;
    }
    WicdDbusInterface *q;
};

K_GLOBAL_STATIC(WicdDbusInterfaceHelper, s_globalWicdDbusInterface)

WicdDbusInterface *WicdDbusInterface::instance()
{
    if (!s_globalWicdDbusInterface->q) {
        new WicdDbusInterface;
    }

    return s_globalWicdDbusInterface->q;
}

WicdDbusInterface::WicdDbusInterface()
        : d(new Private())
{
    Q_ASSERT(!s_globalWicdDbusInterface->q);
    s_globalWicdDbusInterface->q = this;
}

WicdDbusInterface::~WicdDbusInterface()
{
    delete d;
}

QDBusInterface &WicdDbusInterface::daemon() const
{
    return d->manager;
}

QDBusInterface &WicdDbusInterface::wireless() const
{
    return d->wireless;
}

QDBusInterface &WicdDbusInterface::wired() const
{
    return d->wired;
}
