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
	Q_ASSERT(objectPath.startsWith('/'));
	m_adapter = m_objectPath.left(objectPath.size() - 18);
	m_address = m_objectPath.right(17);


	device = new QDBusInterface("org.bluez", m_adapter,
				    "org.bluez.Adapter", QDBusConnection::systemBus());
	#define connectAdapterToThis(signal, slot) \
		device->connection().connect("org.bluez", \
			m_adapter, \
			"org.bluez.Adapter", \
			signal, this, SLOT(slot))
	connectAdapterToThis("RemoteClassUpdated",slotClassChanged(const QString&, uint));
	connectAdapterToThis("RemoteNameUpdated",slotNameUpdated(const QString &,const QString &));
	connectAdapterToThis("RemoteNameFailed",slotNameResolvingFailed(const QString &));
	connectAdapterToThis("RemoteAliasChanged",slotAliasChanged(const QString &,const QString &));
	connectAdapterToThis("RemoteAliasCleared",slotAliasCleared(const QString &));
	connectAdapterToThis("RemoteDeviceConnected",slotConnected(const QString &));
	connectAdapterToThis("RemoteDeviceDisconnectRequested",slotRequestDisconnection(const QString &));
	connectAdapterToThis("RemoteDeviceDisconnected",slotDisconnected(const QString &));
	connectAdapterToThis("BondingCreated",slotBonded(const QString &));
	connectAdapterToThis("BondingRemoved",slotUnbonded(const QString &));
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
	return boolReply("IsConnected");
}

QString BluezBluetoothRemoteDevice::name() const
{
	return stringReply("GetRemoteName");
}

QString BluezBluetoothRemoteDevice::version() const
{
	return stringReply("GetRemoteVersion");
}

QString BluezBluetoothRemoteDevice::revision() const
{
	return stringReply("GetRemoteRevision");
}

QString BluezBluetoothRemoteDevice::manufacturer() const
{
	return stringReply("GetRemoteManufacturer");
}

QString BluezBluetoothRemoteDevice::company() const
{
	return stringReply("GetRemoteCompany");
}

QString BluezBluetoothRemoteDevice::majorClass() const
{
	return stringReply("GetRemoteMajorClass");
}

QString BluezBluetoothRemoteDevice::minorClass() const
{
	return stringReply("GetRemoteMinorClass");
}

QStringList BluezBluetoothRemoteDevice::serviceClasses() const
{
	return listReply("GetRemoteServiceClasses");
}

QString BluezBluetoothRemoteDevice::alias() const
{
	return stringReply("GetRemoteAlias");
}

QString BluezBluetoothRemoteDevice::lastSeen() const
{
	return stringReply("LastSeen");
}

QString BluezBluetoothRemoteDevice::lastUsed() const
{
	return stringReply("LastUsed");
}

bool BluezBluetoothRemoteDevice::hasBonding() const
{

	return boolReply("HasBonding");
}

int BluezBluetoothRemoteDevice::pinCodeLength() const
{
	QDBusReply< int > path = device->call("PinCodeLength", m_address);
	if (!path.isValid())
		return false;

	return path.value();
}

int BluezBluetoothRemoteDevice::encryptionKeySize() const
{
	QDBusReply< int > path = device->call("EncryptionKeySize", m_address);
	if (!path.isValid())
		return false;

	return path.value();
}

KJob *BluezBluetoothRemoteDevice::createBonding()
{
	QList<QVariant> params;
	params << m_address;

	return new BluezCallJob(QDBusConnection::systemBus(), "org.bluez", m_adapter,
				"org.bluez.Adapter", "CreateBonding", params);
}

void BluezBluetoothRemoteDevice::setAlias(const QString &alias)
{
	device->call("SetRemoteAlias",m_address,alias);
}

void BluezBluetoothRemoteDevice::clearAlias()
{
	device->call("ClearRemoteAlias", m_address);
}

void BluezBluetoothRemoteDevice::disconnect()
{
	device->call("DisconnectRemoteDevice", m_address);
}

void BluezBluetoothRemoteDevice::cancelBondingProcess()
{
	device->call("CancelBondingProcess", m_address);
}

void BluezBluetoothRemoteDevice::removeBonding()
{
	device->call("RemoveBonding", m_address);
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
void BluezBluetoothRemoteDevice::slotServiceHandles(const QList< uint > & handles)
{
	emit serviceHandlesAvailable(ubi(),handles);
}

void BluezBluetoothRemoteDevice::slotServiceRecordAsXml(const QString & record)
{
	emit serviceRecordXmlAvailable(ubi(),record);
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

void BluezBluetoothRemoteDevice::dbusErrorHandles(const QDBusError &error)
{
	kDebug() << "Error on dbus call for handles: " << error.message();
	emit serviceHandlesAvailable("failed",QList<uint>());
}

void BluezBluetoothRemoteDevice::dbusErrorRecordAsXml(const QDBusError & error)
{
	kDebug() << "Error on dbus call for record as xml: " << error.message();
	emit serviceRecordXmlAvailable("failed","");
}

void BluezBluetoothRemoteDevice::slotClassChanged(const QString & address, uint newClass)
{
	if (address == this->address()) {
		emit classChanged(newClass);
	}
}

void BluezBluetoothRemoteDevice::slotNameResolvingFailed(const QString & address)
{
	if (address == this->address()) {
		emit nameResolvingFailed();
	}
}

void BluezBluetoothRemoteDevice::slotNameUpdated(const QString & address, const QString & newName)
{
	if (address == this->address()) {
		emit nameChanged(newName);
	}
}

void BluezBluetoothRemoteDevice::slotAliasChanged(const QString & address, const QString & newAlias)
{
	if (address == this->address()) {
		emit aliasChanged(newAlias);
	}
}

void BluezBluetoothRemoteDevice::slotAliasCleared(const QString & address)
{
	if (address == this->address()) {
		emit aliasCleared();
	}
}

void BluezBluetoothRemoteDevice::slotConnected(const QString & address)
{
	if (address == this->address()) {
		emit connected();
	}
}

void BluezBluetoothRemoteDevice::slotRequestDisconnection(const QString & address)
{
	if (address == this->address()) {
		emit requestDisconnection();
	}
}

void BluezBluetoothRemoteDevice::slotDisconnected(const QString & address)
{
	if (address == this->address()) {
		emit disconnected();
	}
}

void BluezBluetoothRemoteDevice::slotBonded(const QString & address)
{
	if (address == this->address()) {
		emit bondingCreated();
	}
}

void BluezBluetoothRemoteDevice::slotUnbonded(const QString & address)
{
	if (address == this->address()) {
		emit bondingRemoved();
	}
}

#include "bluez-bluetoothremotedevice.moc"
