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

#include <QTimer>

#include <kdebug.h>

#include <solid/control/bluetoothremotedevice.h>
#include <QXmlDefaultHandler>

#include "bluezcalljob.h"
#include "bluez-bluetoothremotedevice.h"

Q_DECLARE_METATYPE(QList<uint>);
class SdpXmlHandler: public QXmlDefaultHandler
{
	private:
		QString lastAttribute;
	public:
		bool startDocument();
		bool endDocument ();
		bool startElement ( const QString & namespaceURI, const QString & localName, const QString & qName, const QXmlAttributes & atts );
		bool endElement ( const QString & namespaceURI, const QString & localName, const QString & qName );
		
		Solid::Control::BluetoothServiceRecord record;
};
bool SdpXmlHandler::startElement(const QString & , const QString & localName, const QString & , const QXmlAttributes & atts)
{
	//TODO: Should also parse:
	//	BluetoothServiceRecordState 0x0002 32-bit unsigned integer
	//	ServiceID 0x0003 UUID
	//	BrowseGroupList 0x0005 Data Element Sequence
	//	ServiceInfoTimeToLive 0x0007 32-bit unsigned integer
	//	ServiceAvailability 0x0008 8-bit unsigned intege
	//	DocumentationURL 0x000A URL
	//	ClientExecutableURL 0x000B URL
	//	IconURL 0x000C URL
	if(localName=="attribute" && atts.count()==1)
		lastAttribute = atts.value("id");
	if(lastAttribute=="0x0000" && localName == "uint32")//Mandatory
		record.handle = atts.value("value");
	else if(lastAttribute=="0x0001" && localName == "uuid")//Mandatory
		record.serviceClasses << atts.value("value");	
	else if(lastAttribute=="0x0004")
	{
		if(localName == "uuid")
			record.protocolDescriptors << atts.value("value");
		else if(localName == "uint8")
			record.protocolChannels[record.protocolDescriptors.last()] = atts.value("value");
	} 
	else if(lastAttribute=="0x0006" && localName == "uint16")
		record.langBaseAttributes << atts.value("value");
	else if(lastAttribute=="0x0009")
	{
		if(localName == "uuid")
			record.profileDescriptors << atts.value("value");
		else if(localName == "uint16")
			record.profileVersions[record.profileDescriptors.last()] = atts.value("value");
	}
	else if(lastAttribute=="0x0100" && localName == "text")
		record.name = atts.value("value");
	return true;
}

bool SdpXmlHandler::endElement(const QString & , const QString & localName, const QString & )
{
	if(localName=="attribute")
		lastAttribute = "";
	return true;
}

bool SdpXmlHandler::endDocument()
{
	lastAttribute="";	
	return true;
}

bool SdpXmlHandler::startDocument()
{
	record = Solid::Control::BluetoothServiceRecord();
	return true;
}

/********************************************************************************/

BluezBluetoothRemoteDevice::BluezBluetoothRemoteDevice(const QString &objectPath)
	: BluetoothRemoteDevice(0), m_objectPath(objectPath)
{

    // size("/FF:FF:FF:FF:FF:FF") == 18
	m_adapter = m_objectPath.left(objectPath.size() - 18);
	m_address = m_objectPath.right(17);

	kDebug() << k_funcinfo << " path: " << m_adapter << " address: " << m_address << endl;

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
	return stringReply("GetRemoteMajorClass");
}

QStringList BluezBluetoothRemoteDevice::serviceClasses() const
{
	return listReply("GetRemoteServiceClasses");
}

QList<uint> BluezBluetoothRemoteDevice::serviceHandles(const QString &filter) const
{
	kDebug() << k_funcinfo << endl;

	QDBusReply<QList<uint> > path = device->call("GetRemoteServiceHandles", m_address,filter);
	if (!path.isValid())
	{
		kDebug() << k_funcinfo << "Request failed: " << path.error().message() << endl;
		return QList<uint>();
	}
	return path.value();

}
Solid::Control::BluetoothServiceRecord BluezBluetoothRemoteDevice::serviceRecord(uint handle) const
{
	kDebug() << k_funcinfo << endl;

	QDBusReply<QString> path = device->call("GetRemoteServiceRecordAsXML",m_address,handle);
	if (!path.isValid())
	{
		kDebug() << k_funcinfo << "Request failed: " << path.error().message() << endl;
		return Solid::Control::BluetoothServiceRecord();
	}
		
	SdpXmlHandler handler;
	QXmlSimpleReader xmlReader;
	QXmlInputSource input;
	xmlReader.setContentHandler(&handler);
	xmlReader.setErrorHandler(&handler);
	
	input.setData(path.value());
	xmlReader.parse(input); //Result will be available on handler.record
	kDebug() << "In bluez, for ubi " << ubi() << " adding " << handler.record.name << endl;

	return handler.record;
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

KJob *BluezBluetoothRemoteDevice::createBonding()
{
	QList<QVariant> params;
	params << m_address;

	return new BluezCallJob(QDBusConnection::systemBus(), "org.bluez", m_adapter,
				"org.bluez.Adapter", "CreateBonding", params);
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

#include "bluez-bluetoothremotedevice.moc"
