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

#include <solid/bluetoothinterface.h>

#include "bluez-bluetoothinterface.h"

class BluezBluetoothInterfacePrivate
{
public:
    BluezBluetoothInterfacePrivate(const QString & objPath)
            : iface("org.bluez",
                    objPath,
                    "org.bluez.Adapter",
                    QDBusConnection::systemBus()),
            objectPath(objPath)
    { }
    QDBusInterface iface;
    QString objectPath;
};

BluezBluetoothInterface::BluezBluetoothInterface(const QString & objectPath)
        : BluetoothInterface(0), d(new BluezBluetoothInterfacePrivate(objectPath))
{
#define connectInterfaceToThis( signal, slot ) \
    d->iface.connection().connect( "org.bluez", \
                                   objectPath, \
                                   "org.bluez.Adapter", \
                                   signal, this, SLOT(slot) );

    connectInterfaceToThis("RemoteDeviceFound", slotRemoteDeviceFound(const QString&, uint, short));


    connectInterfaceToThis("ModeChanged", slotModeChanged(const QString&));
    connectInterfaceToThis("DiscoverableTimeoutChanged", slotDiscoverableTimeoutChanged(int));
    connectInterfaceToThis("MinorClassChanged", slotMinorClassChanged(const QString&));
    connectInterfaceToThis("NameChanged", slotNameChanged(const QString&));
    connectInterfaceToThis("DiscoveryStarted", slotDiscoveryStarted());
    connectInterfaceToThis("DiscoveryCompleted", slotDiscoveryCompleted());
    connectInterfaceToThis("RemoteDeviceDisappeared", slotDiscoveryDisappeared(const QString&));

}

BluezBluetoothInterface::~BluezBluetoothInterface()
{
    delete d;
}

QString BluezBluetoothInterface::ubi() const
{
    return d->objectPath;
}

QString BluezBluetoothInterface::address() const
{
    kDebug() << k_funcinfo << endl;

    QDBusReply< QString > address = d->iface.call("GetAddress");
    if (address.isValid()) {
        return address.value();
    }

    return QString::null;
}

QString BluezBluetoothInterface::version() const
{
    kDebug() << k_funcinfo << endl;

    QDBusReply< QString > version = d->iface.call("GetVersion");
    if (version.isValid()) {
        return version.value();
    }

    return QString::null;
}

QString BluezBluetoothInterface::revision() const
{
    kDebug() << k_funcinfo << endl;

    QDBusReply< QString > revision = d->iface.call("GetRevision");
    if (revision.isValid()) {
        return revision.value();
    }

    return QString::null;
}

QString BluezBluetoothInterface::manufacturer() const
{
    kDebug() << k_funcinfo << endl;

    QDBusReply< QString > manufacturer = d->iface.call("GetManufacturer");
    if (manufacturer.isValid()) {
        return manufacturer.value();
    }

    return QString::null;
}

QString BluezBluetoothInterface::company() const
{
    kDebug() << k_funcinfo << endl;

    QDBusReply< QString > company = d->iface.call("GetCompany");
    if (company.isValid()) {
        return company.value();
    }

    return QString::null;
}

QString BluezBluetoothInterface::mode() const
{
    kDebug() << k_funcinfo << endl;

    QDBusReply< QString > mode = d->iface.call("GetMode");
    if (mode.isValid()) {
        return mode.value();
    }

    return QString::null;
}

int BluezBluetoothInterface::discoverableTimeout() const
{
    kDebug() << k_funcinfo << endl;

    QDBusReply< int > timeout = d->iface.call("GetDiscoverableTimeout");
    if (timeout.isValid()) {
        return timeout.value();
    }

    return -1;
}

bool BluezBluetoothInterface::isDiscoverable() const
{
    kDebug() << k_funcinfo << endl;

    QDBusReply< bool > discoverable = d->iface.call("IsDiscoverable");
    if (discoverable.isValid()) {
        return discoverable.value();
    }

    return false;
}

QStringList BluezBluetoothInterface::listConnections() const
{
    return QStringList();
}

QString BluezBluetoothInterface::majorClass() const
{
    kDebug() << k_funcinfo << endl;

    QDBusReply< QString > majorClass = d->iface.call("GetMajorClass");
    if (majorClass.isValid()) {
        return majorClass.value();
    }

    return QString::null;
}

QStringList BluezBluetoothInterface::listAvailableMinorClasses() const
{
    return QStringList();
}

QString BluezBluetoothInterface::minorClass() const
{
    kDebug() << k_funcinfo << endl;

    QDBusReply< QString > minorClass = d->iface.call("GetMinorClass");
    if (minorClass.isValid()) {
        return minorClass.value();
    }

    return QString::null;
}

QStringList BluezBluetoothInterface::serviceClasses() const
{
    return QStringList();
}

QString BluezBluetoothInterface::name() const
{
    kDebug() << k_funcinfo << endl;

    QDBusReply< QString > name = d->iface.call("GetName");
    if (name.isValid()) {
        return name.value();
    }

    return QString::null;
}

QStringList BluezBluetoothInterface::listBondings() const
{
    return QStringList();
}

bool BluezBluetoothInterface::isPeriodicDiscovery() const
{
    kDebug() << k_funcinfo << endl;

    QDBusReply< bool > discovery = d->iface.call("IsPeriodicDiscovery");
    if (discovery.isValid()) {
        return discovery.value();
    }

    return false;
}

bool BluezBluetoothInterface::isPeriodicDiscoveryNameResolving() const
{
    kDebug() << k_funcinfo << endl;

    QDBusReply< bool > discovery = d->iface.call("IsPeriodicDiscoveryNameResolving");
    if (discovery.isValid()) {
        return discovery.value();
    }

    return false;
}

QStringList BluezBluetoothInterface::listRemoteDevices() const
{
    return QStringList();
}

QStringList BluezBluetoothInterface::listRecentRemoteDevices(const QString&) const
{
    return QStringList();
}

void BluezBluetoothInterface::setMode(const QString &mode)
{
    kDebug() << k_funcinfo << endl;
    d->iface.call("SetMode", mode);
}

void BluezBluetoothInterface::setDiscoverableTimeout(int timeout)
{
    kDebug() << k_funcinfo << endl;
    d->iface.call("SetDiscoverableTimeout", timeout);
}

void BluezBluetoothInterface::setMinorClass(const QString &minorClass)
{
    kDebug() << k_funcinfo << endl;
    d->iface.call("SetMinorClass", minorClass);
}

void BluezBluetoothInterface::setName(const QString &name)
{
    kDebug() << k_funcinfo << endl;
    d->iface.call("SetName", name);
}

void BluezBluetoothInterface::discoverDevices()
{
    kDebug() << k_funcinfo << endl;
    d->iface.call("DiscoverDevices");
}

void BluezBluetoothInterface::discoverDevicesWithoutNameResolving()
{
    kDebug() << k_funcinfo << endl;
    d->iface.call("DiscoverDevicesWithoutNameResolving");
}

void BluezBluetoothInterface::cancelDiscovery()
{
    kDebug() << k_funcinfo << endl;
    d->iface.call("CancelDiscovery");
}

void BluezBluetoothInterface::startPeriodicDiscovery()
{
    kDebug() << k_funcinfo << endl;
    d->iface.call("StartPeriodicDiscovery");
}

void BluezBluetoothInterface::stopPeriodicDiscovery()
{
    kDebug() << k_funcinfo << endl;
    d->iface.call("StopPeriodicDiscovery");
}

void BluezBluetoothInterface::setPeriodicDiscoveryNameResolving(bool nameResolving)
{
    kDebug() << k_funcinfo << endl;
    d->iface.call("SetPeriodicDiscoveryNameResolving", nameResolving);
}

void BluezBluetoothInterface::slotModeChanged(const QString &mode)
{
    kDebug() << k_funcinfo << endl;
    emit modeChanged(mode);
}

void BluezBluetoothInterface::slotDiscoverableTimeoutChanged(int timeout)
{
    kDebug() << k_funcinfo << endl;
    emit discoverableTimeoutChanged(timeout);
}

void BluezBluetoothInterface::slotMinorClassChanged(const QString &minorClass)
{
    kDebug() << k_funcinfo << endl;
    emit minorClassChanged(minorClass);
}

void BluezBluetoothInterface::slotNameChanged(const QString &name)
{
    kDebug() << k_funcinfo << endl;
    emit nameChanged(name);
}

void BluezBluetoothInterface::slotDiscoveryStarted()
{
    kDebug() << k_funcinfo << endl;
    emit discoveryStarted();
}

void BluezBluetoothInterface::slotDiscoveryCompleted()
{
    kDebug() << k_funcinfo << endl;
    emit discoveryCompleted();
}

void BluezBluetoothInterface::slotRemoteDeviceFound(const QString &address, uint deviceClass, short rssi)
{
    kDebug() << k_funcinfo << "address: " << address << " class: " << deviceClass << " RSSI: " << rssi << endl;

    QString remoteubi = QString("%1/%2").arg(ubi()).arg(address);
    emit remoteDeviceFound(remoteubi, deviceClass, rssi);
}

void BluezBluetoothInterface::slotRemoteDeviceDisappeared(const QString &address)
{
    kDebug() << k_funcinfo << endl;
    QString remoteubi = QString("%1/%2").arg(ubi()).arg(address);
    emit remoteDeviceDisappeared(remoteubi);
}

// TODO: Write BluetoothDevices iface object ...
QObject *BluezBluetoothInterface::createBluetoothRemoteDevice(const QString& ubi)
{
    return NULL;
}

#include "bluez-bluetoothinterface.moc"
