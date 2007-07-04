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

#ifndef __BLUEZ_SERVICE_PARSER
#define __BLUEZ_SERVICE_PARSER

#include <QThread>
#include <QReadWriteLock>
#include <QQueue>
#include <QMap>
#include <QXmlSimpleReader>
#include <QXmlInputSource>
#include <QXmlAttributes>
#include <QXmlDefaultHandler>
#include <QtDBus>

#include <solid/control/bluetoothremotedevice.h>


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

class ServiceParser : public QThread
{
	Q_OBJECT
	public: 
		ServiceParser(const QString &adapter,QObject *parent = 0);
		~ServiceParser(){};
	public slots:
		void run();
		void finish();
		void findServices(const QString &ubi,const QString &filter ="");
	signals:
		void serviceDiscoveryStarted(const QString &ubi);
		void remoteServiceFound(const QString &ubi, const Solid::Control::BluetoothServiceRecord &service);
		void serviceDiscoveryFinished(const QString &ubi);
	private:
		bool finished;
		QReadWriteLock queueMutex;		
		QQueue<QString> requestQueue;
		QMap<QString,QString> filters;
		QString m_adapter;
};

#endif
