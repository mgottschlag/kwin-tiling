/*  This file is part of the KDE project
    Copyright (C) 2007 Will Stephenson <wstephenson@kde.org>
    Copyright (C) 2007 Daniel Gollub <dgollub@suse.de>
    Copyright (C) 2008 Tom Patzig <tpatzig@suse.de>


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

#include "bluez-bluetoothinputdevice.h"

#include <QtDBus>

#include <kdebug.h>

BluezBluetoothInputDevice::BluezBluetoothInputDevice(const QString &objectPath) : BluetoothInputDevice(0), m_objectPath(objectPath)
{
    device = new QDBusInterface("org.bluez", m_objectPath,
                                "org.bluez.Input", QDBusConnection::systemBus());
    #define connectInputDeviceToThis(signal, slot) \
        device->connection().connect("org.bluez", \
            objectPath, \
            "org.bluez.Input", \
            signal, this, SLOT(slot))
    connectInputDeviceToThis("PropertyChanged",slotPropertyChanged(const QString &,const QDBusVariant &));

}

BluezBluetoothInputDevice::~BluezBluetoothInputDevice()
{
    delete device;
}

QString BluezBluetoothInputDevice::ubi() const
{
    return m_objectPath;
}

QMap<QString,QVariant> BluezBluetoothInputDevice::getProperties() const
{
    QDBusReply< QMap<QString,QVariant> > path = device->call("GetProperties");
        if (!path.isValid())
	    return QMap<QString,QVariant>();

	return path.value();
}

void BluezBluetoothInputDevice::disconnect()
{
    device->call("Disconnect");
}

void BluezBluetoothInputDevice::connect()
{
    device->call("Connect");
}


/******************************/

QString BluezBluetoothInputDevice::stringReply(const QString &method) const
{
    QDBusReply< QString > reply = device->call(method);
    if (!reply.isValid())
        return QString();

    return reply.value();
}

bool BluezBluetoothInputDevice::boolReply(const QString &method) const
{
    QDBusReply< bool > reply = device->call(method);
    if (!reply.isValid())
        return false;

    return reply.value();
}

/******************************/

void BluezBluetoothInputDevice::slotPropertyChanged(const QString & name, const QDBusVariant& value)
{
    emit propertyChanged(name,value.variant());
}

#include "bluez-bluetoothinputdevice.moc"
