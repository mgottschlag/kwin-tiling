/*  This file is part of the KDE project
    Copyright (C) 2007 Juan González <kde@opsiland.info>

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
#include "bluez-serviceparser.h"
#include <QMetaType>
#include <kdebug.h>
typedef QList<uint> QListUint;

Q_DECLARE_METATYPE(QListUint)
Q_DECLARE_METATYPE(Solid::Control::BluetoothServiceRecord)

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

ServiceParser::ServiceParser(const QString &adapter,QObject *parent)
	: QThread(parent),finished(false),m_adapter(adapter)
{	
}
void ServiceParser::findServices(const QString &ubi,const QString &filter)
{
	queueMutex.lockForWrite();
	requestQueue << ubi;
	filters[ubi] = filter;
	queueMutex.unlock();
}
void ServiceParser::run()
{
	QDBusInterface device("org.bluez", m_adapter,
		       "org.bluez.Adapter", QDBusConnection::systemBus());
	
	qDBusRegisterMetaType<QListUint>();
	qRegisterMetaType<Solid::Control::BluetoothServiceRecord>("Solid::Control::BluetoothServiceRecord");
	
	SdpXmlHandler handler;
	QXmlSimpleReader xmlReader;
	QXmlInputSource input;	
	
	xmlReader.setContentHandler(&handler);
	xmlReader.setErrorHandler(&handler);
	
	while(!finished)
	{
		QString ubi;
		queueMutex.lockForRead();
		if(!requestQueue.isEmpty())
		{
			ubi = requestQueue.dequeue();
		}
		queueMutex.unlock();
		if(ubi.isEmpty())
		{
			msleep(100);
		} else
		{	
			const QString n_ubi = ubi;
			emit serviceDiscoveryStarted(n_ubi);
			QDBusReply<QListUint>  reply = device.call("GetRemoteServiceHandles",ubi,filters[ubi]);
			QListUint args = reply.value();
			for(int i = 0; i < args.size(); i++)
			{
				QDBusReply<QString> record = device.call("GetRemoteServiceRecordAsXML",ubi.split("/").last(),args[i]);
				input.setData(record.value());
				xmlReader.parse(input);
				const Solid::Control::BluetoothServiceRecord n_record = handler.record;
// 				kDebug() << "In bluez, for ubi " << n_ubi << " adding " << n_record.name << endl;
				emit remoteServiceFound(n_ubi,n_record);
			}
			emit serviceDiscoveryFinished(n_ubi);
		}
	}
}

void ServiceParser::finish()
{
	finished = true;
}
#include "bluez-serviceparser.moc"

