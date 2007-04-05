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

#include <solid/bluetoothremotedevice.h>

#include "bluez-bluetoothremotedevice.h"

BluezBluetoothRemoteDevice::BluezBluetoothRemoteDevice(const QString &objectPath)
        : BluetoothRemoteDevice(0), m_objectPath(objectPath)
{
    kDebug() << k_funcinfo << " path: " << m_objectPath << endl;

    // size("/FF:FF:FF:FF:FF:FF") == 18
    m_adapter = m_objectPath.left(m_objectPath.size() - 18);
    m_address = m_objectPath.right(17);

    device = new QDBusInterface("org.bluez", m_adapter,
                                "org.bluez.Adapter", QDBusConnection::systemBus());

}

BluezBluetoothRemoteDevice::~BluezBluetoothRemoteDevice()
{
    delete device;
}

QString BluezBluetoothRemoteDevice::ubi() const
{
    return m_objectPath;
}

QString BluezBluetoothRemoteDevice::address() const
{
    return m_address;
}

bool BluezBluetoothRemoteDevice::isConnected() const
{
    kDebug() << k_funcinfo << endl;

    QDBusReply< bool > path = device->call("IsConnected", m_address);
    if (!path.isValid())
        return false;

    return path.value();
}

QString BluezBluetoothRemoteDevice::name() const
{
    kDebug() << k_funcinfo << endl;

    QDBusReply< QString > path = device->call("GetRemoteName", m_address);
    if (!path.isValid())
        return QString::null;

    return path.value();
}

QString BluezBluetoothRemoteDevice::version() const
{
    kDebug() << k_funcinfo << endl;

    QDBusReply< QString > path = device->call("GetRemoteVersion", m_address);
    if (!path.isValid())
        return QString::null;

    return path.value();
}

QString BluezBluetoothRemoteDevice::revision() const
{
    kDebug() << k_funcinfo << endl;

    QDBusReply< QString > path = device->call("GetRemoteRevision", m_address);
    if (!path.isValid())
        return QString::null;

    return path.value();
}

QString BluezBluetoothRemoteDevice::manufacturer() const
{
    kDebug() << k_funcinfo << endl;

    QDBusReply< QString > path = device->call("GetRemoteManufacturer", m_address);
    if (!path.isValid())
        return QString::null;

    return path.value();
}

QString BluezBluetoothRemoteDevice::company() const
{
    kDebug() << k_funcinfo << endl;

    QDBusReply< QString > path = device->call("GetRemoteCompany", m_address);
    if (!path.isValid())
        return QString::null;

    return path.value();
}

QString BluezBluetoothRemoteDevice::majorClass() const
{
    kDebug() << k_funcinfo << endl;

    QDBusReply< QString > path = device->call("GetMajorClass", m_address);
    if (!path.isValid())
        return QString::null;

    return path.value();
}

QString BluezBluetoothRemoteDevice::minorClass() const
{
    kDebug() << k_funcinfo << endl;

    QDBusReply< QString > path = device->call("GetMajorClass", m_address);
    if (!path.isValid())
        return QString::null;

    return path.value();
}

QStringList BluezBluetoothRemoteDevice::serviceClasses() const
{
    kDebug() << k_funcinfo << endl;

    QDBusReply< QStringList > path = device->call("GetServiceClasses", m_address);
    if (!path.isValid())
        return QStringList();

    return path.value();
}

QString BluezBluetoothRemoteDevice::alias() const
{
    kDebug() << k_funcinfo << endl;

    QDBusReply< QString > path = device->call("GetRemoteAlias", m_address);
    if (!path.isValid())
        return QString::null;

    return path.value();
}

QString BluezBluetoothRemoteDevice::lastSeen() const
{
    kDebug() << k_funcinfo << endl;

    QDBusReply< QString > path = device->call("LastSeen", m_address);
    if (!path.isValid())
        return QString::null;

    return path.value();
}

QString BluezBluetoothRemoteDevice::lastUsed() const
{
    kDebug() << k_funcinfo << endl;

    QDBusReply< QString > path = device->call("LastUsed", m_address);
    if (!path.isValid())
        return QString::null;

    return path.value();
}

bool BluezBluetoothRemoteDevice::hasBonding() const
{
    kDebug() << k_funcinfo << endl;

    QDBusReply< bool > path = device->call("HasBonding", m_address);
    if (!path.isValid())
        return false;

    return path.value();
}

int BluezBluetoothRemoteDevice::pinCodeLength() const
{
    kDebug() << k_funcinfo << endl;

    QDBusReply< int > path = device->call("PinCodeLength", m_address);
    if (!path.isValid())
        return false;

    return path.value();
}

int BluezBluetoothRemoteDevice::encryptionKeySize() const
{
    kDebug() << k_funcinfo << endl;

    QDBusReply< int > path = device->call("EncryptionKeySize", m_address);
    if (!path.isValid())
        return false;

    return path.value();
}

void BluezBluetoothRemoteDevice::setAlias(const QString &alias)
{
    kDebug() << k_funcinfo << endl;
    device->call("SetRemoteAlias", m_address, alias);
}

void BluezBluetoothRemoteDevice::clearAlias()
{
    kDebug() << k_funcinfo << endl;
    device->call("ClearRemoteAlias", m_address);
}

void BluezBluetoothRemoteDevice::disconnect()
{
    kDebug() << k_funcinfo << endl;
    device->call("DisconnectRemoteDevice", m_address);
}

void BluezBluetoothRemoteDevice::createBonding()
{
    kDebug() << k_funcinfo << endl;
    device->call("CreateBonding", m_address);
}

void BluezBluetoothRemoteDevice::cancelBondingProcess()
{
    kDebug() << k_funcinfo << endl;
    device->call("CancelBondingProcess", m_address);
}

void BluezBluetoothRemoteDevice::removeBonding()
{
    kDebug() << k_funcinfo << endl;
    device->call("RemoveBonding", m_address);
}

#include "bluez-bluetoothremotedevice.moc"
