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
    return stringReply("GetAddress");
}

bool BluezBluetoothInputDevice::isConnected() const
{
    return boolReply("IsConnected");
}

QString BluezBluetoothInputDevice::name() const
{
    return stringReply("GetName");
}

QString BluezBluetoothInputDevice::productID() const
{
    return stringReply("GetProductID");
}

QString BluezBluetoothInputDevice::vendorID() const
{
    return stringReply("GetVendorID");
}

void BluezBluetoothInputDevice::slotConnect()
{
    device->call("Connect");
}

void BluezBluetoothInputDevice::slotDisconnect()
{
    device->call("Disconnect");
}

/******************************/

QString BluezBluetoothInputDevice::stringReply(const QString &method) const
{
    QDBusReply< QString > reply = device->call(method);
    if (!reply.isValid())
        return QString::null;

    return reply.value();
}

bool BluezBluetoothInputDevice::boolReply(const QString &method) const
{
    QDBusReply< bool > reply = device->call(method);
    if (!reply.isValid())
        return false;

    return reply.value();
}

#include "bluez-bluetoothinputdevice.moc"
