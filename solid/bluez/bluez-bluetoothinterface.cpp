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

#include "bluez-bluetoothinterface.h"

#include <solid/control/bluetoothinterface.h>

#include "bluez-bluetoothremotedevice.h"
#include "bluez-bluetoothinputdevice.h"
#include <KDebug>



class BluezBluetoothInterfacePrivate
{
public:
    BluezBluetoothInterfacePrivate(const QString  & objPath)
            : iface("org.bluez",
                    objPath,
                    "org.bluez.Adapter",
                    QDBusConnection::systemBus()),
            objectPath(objPath)
    { }
    QDBusInterface iface;
    QString objectPath;

    QMap<QString, BluezBluetoothRemoteDevice *> devices;
    QMap<QString, BluezBluetoothInputDevice *> inputDevices;
};



BluezBluetoothInterface::BluezBluetoothInterface(const QString  & objectPath)
        : BluetoothInterface(0), d(new BluezBluetoothInterfacePrivate(objectPath))
{

#define connectInterfaceToThis(signal, slot) \
    d->iface.connection().connect("org.bluez", \
                                   objectPath, \
                                   "org.bluez.Adapter", \
                                   signal, this, SLOT(slot));

    connectInterfaceToThis("PropertyChanged", slotPropertyChanged(const QString &, const QVariant &));
    connectInterfaceToThis("DeviceCreated", slotDeviceCreated(const QDBusObjectPath &));
    connectInterfaceToThis("DeviceRemoved", slotDeviceRemoved(const QDBusObjectPath &));
    connectInterfaceToThis("DeviceDisappeared", slotDeviceDisappeared(const QString &));
    connectInterfaceToThis("DeviceFound", slotDeviceFound(const QString &, const QMap< QString,QVariant > &));


}

BluezBluetoothInterface::~BluezBluetoothInterface()
{
    delete d;
}

QString BluezBluetoothInterface::ubi() const
{
    return d->objectPath;
}

void BluezBluetoothInterface::cancelDeviceCreation(const QString &addr)
{
    d->iface.call("CancelDeviceCreation",addr);
}

void BluezBluetoothInterface::createDevice(const QString &addr) const
{
    d->iface.call("CreateDevice",addr);
}

void BluezBluetoothInterface::createPairedDevice(const QString &addr, const QString &agentUBI, const QString &capab) const
{
    d->iface.call("CreatePairedDevice",addr,qVariantFromValue(QDBusObjectPath(agentUBI)),capab);
}

QString BluezBluetoothInterface::findDevice(const QString &addr) const
{
    QDBusObjectPath path = objectReply("FindDevice",addr);
    return path.path();
}


QMap<QString, QVariant> BluezBluetoothInterface::getProperties() const
{
    QDBusReply< QMap<QString,QVariant> > prop = d->iface.call("GetProperties");
    if (!prop.isValid()) {
        return QMap< QString,QVariant >();
    }
    return prop.value();
}

QStringList BluezBluetoothInterface::listDevices() const
{
    QStringList deviceList;

    QDBusReply< QList<QDBusObjectPath> > devices = d->iface.call("ListDevices");
    if(!devices.isValid()) {
        return QStringList();
    }
    foreach(QDBusObjectPath path, devices.value()) {
        deviceList.append(path.path());
    }
    return deviceList;
}

void BluezBluetoothInterface::registerAgent(const QString &agentUBI, const QString &capab)
{
    d->iface.call("RegisterAgent",qVariantFromValue(QDBusObjectPath(agentUBI)),capab);
}

void BluezBluetoothInterface::releaseSession()
{
    d->iface.call("ReleaseSession");
}

void BluezBluetoothInterface::removeDevice(const QString &deviceUBI )
{
    d->iface.call("RemoveDevice",qVariantFromValue(QDBusObjectPath(deviceUBI)));
}

void BluezBluetoothInterface::requestSession()
{
    d->iface.call("RequestSession");
}

void BluezBluetoothInterface::setProperty(const QString &property, const QVariant &value)
{
    d->iface.call("SetProperty",property, qVariantFromValue(QDBusVariant(value)));
}


void BluezBluetoothInterface::startDiscovery()
{
    d->iface.call("StartDiscovery");
}

void BluezBluetoothInterface::stopDiscovery()
{
    d->iface.call("StopDiscovery");
}

void BluezBluetoothInterface::unregisterAgent(const QString &agentUBI)
{
    d->iface.call("UnregisterAgent",qVariantFromValue(QDBusObjectPath(agentUBI)));
}



void BluezBluetoothInterface::slotDeviceCreated(const QDBusObjectPath &path)
{
    kDebug() << "device created";

    if (!d->devices.contains(path.path())) {
        BluezBluetoothRemoteDevice* bluetoothRemoteDev = new BluezBluetoothRemoteDevice(path.path());
        d->devices.insert(path.path(), bluetoothRemoteDev);
    }

    emit deviceCreated(path.path());
}

void BluezBluetoothInterface::slotDeviceDisappeared(const QString &address)
{
    kDebug() << "device disappeared";
    emit deviceDisappeared(address);
}

void BluezBluetoothInterface::slotDeviceFound(const QString &address, const QMap< QString, QVariant > &properties)
{
    kDebug() << "device found " << address << " " << properties["Name"];
    emit deviceFound(address,properties);
}

void BluezBluetoothInterface::slotDeviceRemoved(const QDBusObjectPath &path)
{
    kDebug() << "device removed";
    emit deviceRemoved(path.path());
}

void BluezBluetoothInterface::slotPropertyChanged(const QString & property, const QVariant &value)
{
    kDebug() << "Property " << property << " changed to " << value;
    emit propertyChanged(property,value);
}



QObject *BluezBluetoothInterface::createBluetoothRemoteDevice(const QString &ubi)
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

QObject *BluezBluetoothInterface::createBluetoothInputDevice(const QString &ubi)
{
    BluezBluetoothInputDevice *bluetoothInputDev;
    if (d->inputDevices.contains(ubi)) {
        bluetoothInputDev = d->inputDevices[ubi];
    } else {
        bluetoothInputDev = new BluezBluetoothInputDevice(ubi);
        d->inputDevices.insert(ubi, bluetoothInputDev);
    }
    return bluetoothInputDev;
}



/******************* DBus Calls *******************************/

QStringList BluezBluetoothInterface::listReply(const QString &method) const
{
    QDBusReply< QStringList > list = d->iface.call(method);
    if (!list.isValid()) {
        return QStringList();
    }

    return list.value();
}

QString BluezBluetoothInterface::stringReply(const QString &method, const QString &param) const
{
    QDBusReply< QString > reply;

    if (param.isEmpty())
	    reply = d->iface.call(method);
    else
	    reply = d->iface.call(method, param);
	    	
    if (reply.isValid()) {
        return reply.value();
    }

    return QString();
}

bool BluezBluetoothInterface::boolReply(const QString &method, const QString &param) const
{
    QDBusReply< bool > reply; 

    if (param.isEmpty())
	    reply = d->iface.call(method);
    else
	    reply = d->iface.call(method, param);

    if (reply.isValid()) {
        return reply.value();
    }

    return false;
}

QDBusObjectPath BluezBluetoothInterface::objectReply(const QString &method, const QString &param) const
{
    QDBusReply< QDBusObjectPath > reply;

    if (param.isEmpty())
	    reply = d->iface.call(method);
    else {
            qDebug() << "ObjectReply calling: " << method << " " << param;
	    reply = d->iface.call(method, param);
    }
	    	
    if (reply.isValid()) {
        qDebug() << "ObjectReply Valid? "<<  reply.value().path();
        return reply.value();
    }

    return QDBusObjectPath();
}

#include "bluez-bluetoothinterface.moc"
