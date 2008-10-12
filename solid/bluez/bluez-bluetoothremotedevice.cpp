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

#include "bluez-bluetoothremotedevice.h"

#include <QQueue>
#include <QTimer>

#include <kdebug.h>

#include <solid/control/bluetoothremotedevice.h>

#include "bluezcalljob.h"

Q_DECLARE_METATYPE(QList<uint>)

BluezBluetoothRemoteDevice::BluezBluetoothRemoteDevice(const QString &objectPath)
	: BluetoothRemoteDevice(0), m_objectPath(objectPath)
{

    // size("/FF:FF:FF:FF:FF:FF") == 18
	//Q_ASSERT(objectPath.startsWith('/'));
	m_adapter = m_objectPath.left(objectPath.size() - 18);
	m_address = m_objectPath.right(17);


	device = new QDBusInterface("org.bluez", objectPath,
				    "org.bluez.Device", QDBusConnection::systemBus());
	#define connectDeviceToThis(signal, slot) \
		device->connection().connect("org.bluez", \
			objectPath, \
			"org.bluez.Device", \
			signal, this, SLOT(slot))
        connectDeviceToThis("PropertyChanged",slotPropertyChanged(const QString &,const QDBusVariant &));
        connectDeviceToThis("DisconnectRequested",slotDisconnectRequested());
        connectDeviceToThis("NodeCreated",slotNodeCreated(const QDBusObjectPath &));
        connectDeviceToThis("NodeRemoved",slotNodeRemoved(const QDBusObjectPath &));


}

BluezBluetoothRemoteDevice::~BluezBluetoothRemoteDevice()
{
	delete device;
}

QString BluezBluetoothRemoteDevice::ubi()
{
    return device->path();
}

QMap<QString,QVariant> BluezBluetoothRemoteDevice::getProperties()
{
    QDBusReply< QMap<QString,QVariant> > path = device->call("GetProperties");
        if (!path.isValid())
	    return QMap<QString,QVariant>();

	return path.value();
}

void BluezBluetoothRemoteDevice::setProperty(const QString &name, const QVariant &value)
{
    device->call("SetProperty",name,value);
}

void BluezBluetoothRemoteDevice::discoverServices(const QString& pattern)
{
    QList<QVariant> args;
    args << pattern;
    device->callWithCallback("DiscoverServices",
            args,
            (QObject*)this,
            SLOT(slotServiceDiscover(const QMap<uint,QString> &)),
            SLOT(dbusErrorServiceDiscover(const QDBusError &)));
   
}

void BluezBluetoothRemoteDevice::cancelDiscovery()
{
    device->call("CancelDiscovery");
}

void BluezBluetoothRemoteDevice::disconnect()
{
    device->call("Disconnect");
}

QStringList BluezBluetoothRemoteDevice::listNodes()
{
    QStringList list;
    QDBusReply< QList<QDBusObjectPath> > path = device->call("ListNodes");
        if (path.isValid()) {
            foreach(QDBusObjectPath objectPath, path.value()) {
                list.append(objectPath.path());
            }
	    return list;
        }

	return QStringList();
}

/*
KJob *BluezBluetoothRemoteDevice::createBonding()
{
	QList<QVariant> params;
	params << m_address;

	return new BluezCallJob(QDBusConnection::systemBus(), "org.bluez", m_adapter,
				"org.bluez.Adapter", "CreateBonding", params);
}


void BluezBluetoothRemoteDevice::serviceHandles(const QString &filter) const
{
	QList<QVariant> args;
	args << m_address << filter;
	device->callWithCallback("GetRemoteServiceHandles",
				 args,
				 (QObject*)this,
				 SLOT(slotServiceHandles(const QList<uint> &)),
				 SLOT(dbusErrorHandles(const QDBusError &)));

}

void BluezBluetoothRemoteDevice::serviceRecordAsXml(uint handle) const
{
	QList<QVariant> args;
	args << m_address << handle;
	device->callWithCallback("GetRemoteServiceRecordAsXML",
				 args,
				 (QObject*)this,
				 SLOT(slotServiceRecordAsXml(const QString &)),
				 SLOT(dbusErrorRecordAsXml(const QDBusError &)));
}

void BluezBluetoothRemoteDevice::slotServiceRecordAsXml(const QString & record)
{
	emit serviceRecordXmlAvailable(ubi(),record);
}
*/
void BluezBluetoothRemoteDevice::slotServiceDiscover(const QMap< uint,QString > & handles)
{
	emit serviceDiscoverAvailable("success",handles);
}


/******************************/

QStringList BluezBluetoothRemoteDevice::listReply(const QString &method) const
{
	QDBusReply< QStringList > reply = device->call(method, m_address);
	if (!reply.isValid())
		return QStringList();

	return reply.value();
}

QString BluezBluetoothRemoteDevice::stringReply(const QString &method) const
{
	QDBusReply< QString > reply = device->call(method, m_address);
	if (!reply.isValid())
		return QString();

	return reply.value();
}

bool BluezBluetoothRemoteDevice::boolReply(const QString &method) const
{
	QDBusReply< bool > reply = device->call(method, m_address);
	if (!reply.isValid())
		return false;

	return reply.value();
}

void BluezBluetoothRemoteDevice::dbusErrorServiceDiscover(const QDBusError &error)
{
	kDebug() << "Error on dbus call for DiscoverServices: " << error.message();
	emit serviceDiscoverAvailable("failed",QMap<uint,QString>());
}

void BluezBluetoothRemoteDevice::slotPropertyChanged(const QString &prop, const QDBusVariant &value)
{
    emit propertyChanged(prop, value.variant());
}

void BluezBluetoothRemoteDevice::slotDisconnectRequested()
{
    emit disconnectRequested();
}

void BluezBluetoothRemoteDevice::slotNodeCreated(const QDBusObjectPath &path)
{
    emit nodeCreated(path.path());
}

void BluezBluetoothRemoteDevice::slotNodeRemoved(const QDBusObjectPath &path)
{
    emit nodeRemoved(path.path());
}


#include "bluez-bluetoothremotedevice.moc"
