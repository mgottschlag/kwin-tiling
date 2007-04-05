/*  This file is part of the KDE project
    Copyright (C) 2007 Will Stephenson <wstephenson@kde.org>
    Copyright (C) 2007 Daniel Gollub <dgollub@suse.de>


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

#include <QtDBus>

#include <kdebug.h>

#include <solid/bluetoothinputdevice.h>

#include "bluez-bluetoothinputdevice.h"

BluezBluetoothInputDevice::BluezBluetoothInputDevice(const QString &objectPath,
        const QString &dest) : BluetoothInputDevice(0), m_objectPath(objectPath)
{
    device = new QDBusInterface(dest, m_objectPath,
                                "org.bluez.input.Device", QDBusConnection::systemBus());

}

BluezBluetoothInputDevice::~BluezBluetoothInputDevice()
{
    delete device;
}

QString BluezBluetoothInputDevice::ubi() const
{
    return m_objectPath;
}

QString BluezBluetoothInputDevice::address() const
{
    kDebug() << k_funcinfo << endl;

    QDBusReply< QString > path = device->call("GetAddress");
    if (!path.isValid())
        return QString::null;

    return path.value();
}

bool BluezBluetoothInputDevice::isConnected() const
{
    kDebug() << k_funcinfo << endl;

    QDBusReply< bool > path = device->call("IsConnected");
    if (!path.isValid())
        return false;

    return path.value();
}

QString BluezBluetoothInputDevice::name() const
{
    kDebug() << k_funcinfo << endl;

    QDBusReply< QString > path = device->call("GetName");
    if (!path.isValid())
        return QString::null;

    return path.value();
}


QString BluezBluetoothInputDevice::productID() const
{
    kDebug() << k_funcinfo << endl;

    QDBusReply< QString > path = device->call("GetProductID");
    if (!path.isValid())
        return QString::null;

    return path.value();
}


QString BluezBluetoothInputDevice::vendorID() const
{
    kDebug() << k_funcinfo << endl;

    QDBusReply< QString > path = device->call("GetVendorID");
    if (!path.isValid())
        return QString::null;

    return path.value();
}

void BluezBluetoothInputDevice::slotConnect()
{
    kDebug() << k_funcinfo << endl;
    device->call("Connect");
}

void BluezBluetoothInputDevice::slotDisconnect()
{
    kDebug() << k_funcinfo << endl;
    device->call("Disconnect");
}

#include "bluez-bluetoothinputdevice.moc"
