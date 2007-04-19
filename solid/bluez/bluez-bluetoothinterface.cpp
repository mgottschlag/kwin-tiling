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

#include <solid/experimental/bluetoothinterface.h>

#include "bluez-bluetoothremotedevice.h"
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

    QMap<QString, BluezBluetoothRemoteDevice*> devices;
};

BluezBluetoothInterface::BluezBluetoothInterface(const QString & objectPath)
        : BluetoothInterface(0), d(new BluezBluetoothInterfacePrivate(objectPath))
{

#define connectInterfaceToThis( signal, slot ) \
    d->iface.connection().connect( "org.bluez", \
                                   objectPath, \
                                   "org.bluez.Adapter", \
                                   signal, this, SLOT(slot) );

    connectInterfaceToThis("ModeChanged", slotModeChanged(const QString&));
    connectInterfaceToThis("DiscoverableTimeoutChanged", slotDiscoverableTimeoutChanged(int));
    connectInterfaceToThis("MinorClassChanged", slotMinorClassChanged(const QString&));
    connectInterfaceToThis("NameChanged", slotNameChanged(const QString&));
    connectInterfaceToThis("DiscoveryStarted", slotDiscoveryStarted());
    connectInterfaceToThis("DiscoveryCompleted", slotDiscoveryCompleted());
    connectInterfaceToThis("RemoteDeviceDisappeared", slotRemoteDeviceDisappeared(const QString&));
    connectInterfaceToThis("RemoteDeviceFound", slotRemoteDeviceFound(const QString&, uint, short));
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
    return stringReply("GetAddress");
}

QString BluezBluetoothInterface::version() const
{
    return stringReply("GetVersion");
}

QString BluezBluetoothInterface::revision() const
{
    return stringReply("GetRevision");
}

QString BluezBluetoothInterface::manufacturer() const
{
    return stringReply("GetManufacturer");
}

QString BluezBluetoothInterface::company() const
{
    return stringReply("GetCompany");
}

SolidExperimental::BluetoothInterface::Mode BluezBluetoothInterface::mode() const
{
    QString theMode = stringReply("GetMode");
    SolidExperimental::BluetoothInterface::Mode modeEnum;
    if ( theMode == "off" )
    {
        modeEnum = SolidExperimental::BluetoothInterface::Off;
    }
    else if ( theMode == "connectable" )
    {
        modeEnum = SolidExperimental::BluetoothInterface::Connectable;
    }
    else if ( theMode == "discoverable" )
    {
        modeEnum = SolidExperimental::BluetoothInterface::Discoverable;
    }
    return modeEnum;
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
    return boolReply("IsDiscoverable");
}

QStringList BluezBluetoothInterface::listConnections() const
{
    return listReply("ListConnections");
}

QString BluezBluetoothInterface::majorClass() const
{
    return stringReply("GetMajorClass");
}

QStringList BluezBluetoothInterface::listAvailableMinorClasses() const
{
    return listReply("ListAvailableMinorClasses");
}

QString BluezBluetoothInterface::minorClass() const
{
    return stringReply("GetMinorClass");
}

QStringList BluezBluetoothInterface::serviceClasses() const
{
    return listReply("GetServiceClasses");
}

QString BluezBluetoothInterface::name() const
{
    return stringReply("GetName");
}

QStringList BluezBluetoothInterface::listBondings() const
{
    return listReply("ListBondings");
}

bool BluezBluetoothInterface::isPeriodicDiscoveryActive() const
{
    return boolReply("IsPeriodicDiscovery");
}

bool BluezBluetoothInterface::isPeriodicDiscoveryNameResolvingActive() const
{
    return boolReply("IsPeriodicDiscoveryNameResolving");
}

QStringList BluezBluetoothInterface::listRemoteDevices() const
{
    return listReply("ListRemoteDevces");
}

QStringList BluezBluetoothInterface::listRecentRemoteDevices(const QDateTime&) const
{
    return listReply("ListRecentRemoteDevices");
}

void BluezBluetoothInterface::setMode(const SolidExperimental::BluetoothInterface::Mode mode)
{
    d->iface.call("SetMode", mode);
}

void BluezBluetoothInterface::setDiscoverableTimeout(int timeout)
{
    d->iface.call("SetDiscoverableTimeout", timeout);
}

void BluezBluetoothInterface::setMinorClass(const QString &minorClass)
{
    d->iface.call("SetMinorClass", minorClass);
}

void BluezBluetoothInterface::setName(const QString &name)
{
    d->iface.call("SetName", name);
}

void BluezBluetoothInterface::discoverDevices()
{
    d->iface.call("DiscoverDevices");
}

void BluezBluetoothInterface::discoverDevicesWithoutNameResolving()
{
    d->iface.call("DiscoverDevicesWithoutNameResolving");
}

void BluezBluetoothInterface::cancelDiscovery()
{
    d->iface.call("CancelDiscovery");
}

void BluezBluetoothInterface::startPeriodicDiscovery()
{
    d->iface.call("StartPeriodicDiscovery");
}

void BluezBluetoothInterface::stopPeriodicDiscovery()
{
    d->iface.call("StopPeriodicDiscovery");
}

void BluezBluetoothInterface::setPeriodicDiscoveryNameResolving(bool nameResolving)
{
    d->iface.call("SetPeriodicDiscoveryNameResolving", nameResolving);
}

void BluezBluetoothInterface::slotModeChanged(const SolidExperimental::BluetoothInterface::Mode mode)
{
    emit modeChanged(mode);
}

void BluezBluetoothInterface::slotDiscoverableTimeoutChanged(int timeout)
{
    emit discoverableTimeoutChanged(timeout);
}

void BluezBluetoothInterface::slotMinorClassChanged(const QString &minorClass)
{
    emit minorClassChanged(minorClass);
}

void BluezBluetoothInterface::slotNameChanged(const QString &name)
{
    emit nameChanged(name);
}

void BluezBluetoothInterface::slotDiscoveryStarted()
{
    emit discoveryStarted();
}

void BluezBluetoothInterface::slotDiscoveryCompleted()
{
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
    kDebug() << k_funcinfo << "address: " << address << endl;
    QString remoteubi = QString("%1/%2").arg(ubi()).arg(address);
    emit remoteDeviceDisappeared(remoteubi);
}

QObject *BluezBluetoothInterface::createBluetoothRemoteDevice(const QString& ubi)
{
    BluezBluetoothRemoteDevice *bluetoothInterface;
    if (d->devices.contains(ubi)) {
        bluetoothInterface = d->devices[ubi];
    } else {
        bluetoothInterface = new BluezBluetoothRemoteDevice(ubi);
        d->devices.insert(ubi, bluetoothInterface);
    }
    return bluetoothInterface;
}

/*******************************/

QStringList BluezBluetoothInterface::listReply(const QString &method) const
{
    QDBusReply< QStringList > list = d->iface.call(method);
    if (!list.isValid()) {
        return QStringList();
    }

    return list.value();
}

QString BluezBluetoothInterface::stringReply(const QString &method) const
{
    QDBusReply< QString > reply = d->iface.call(method);
    if (reply.isValid()) {
        return reply.value();
    }

    return QString::null;
}

bool BluezBluetoothInterface::boolReply(const QString &method) const
{
    QDBusReply< bool > reply = d->iface.call(method);
    if (reply.isValid()) {
        return reply.value();
    }

    return false;
}

#include "bluez-bluetoothinterface.moc"
