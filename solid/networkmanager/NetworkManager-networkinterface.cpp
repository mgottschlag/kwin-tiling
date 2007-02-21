/*  This file is part of the KDE project
    Copyright (C) 2007 Will Stephenson <wstephenson@kde.org>

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
#include "NetworkManager-networkinterface.h"

void dump( const NMDBusDeviceProperties& device )
{
	kDebug() << "Object path: " << device.path.path() << "\nInterface: " << device.interface
		<< "\nType: " << device.type << "\nUdi: " << device.udi << "\nActive: "<< device.active
		<< "\nActivation stage: " << device.activationStage << "\nIPV4 address: " << device.ipv4Address
		<< "\nsubnet mask: " << device.subnetMask << "\nBroadcast: " << device.broadcast
		<< "\nroute: " << device.route << "\nprimary dns: " << device.primaryDNS 
		<< "\nsecondary dns: " << device.secondaryDNS << "\nmode: " << device.mode 
		<< "\nStrength: " << device.strength << "\nLink active: " << device.linkActive
		<< "\nSpeed: " << device.speed << "\nCapabilities: " << device.capabilities 
		<< "\nCapabilities type: " << device.capabilitiesType << "\nactive net path: "
		<< device.activeNetPath << "\nNetworks:" << device.networks << endl;
}

void deserialize( const QDBusMessage &message, NMDBusDeviceProperties & device )
{
	kDebug() << /*"deserialize args: " << message.arguments() << */"signature: " << message.signature() << endl;
	QList<QVariant> args = message.arguments();
	device.path.setPath( args.takeFirst().toString() );
	device.interface = args.takeFirst().toString();
	device.type = args.takeFirst().toUInt();
	device.udi = args.takeFirst().toString();
	device.active = args.takeFirst().toBool();
	device.activationStage = args.takeFirst().toUInt();
	device.ipv4Address = args.takeFirst().toString();
	device.subnetMask = args.takeFirst().toString();
	device.broadcast = args.takeFirst().toString();
	device.hardwareAddress = args.takeFirst().toString();
	device.route = args.takeFirst().toString();
	device.primaryDNS = args.takeFirst().toString();
	device.secondaryDNS = args.takeFirst().toString();
	device.mode = args.takeFirst().toInt();
	device.strength = args.takeFirst().toInt();
	device.linkActive = args.takeFirst().toBool();
	device.speed = args.takeFirst().toInt();
	device.capabilities = args.takeFirst().toUInt();
	device.capabilitiesType = args.takeFirst().toUInt();
	device.activeNetPath = args.takeFirst().toString();
	device.networks = args.takeFirst().toStringList();
}

typedef void NMNetwork;

class NMNetworkInterfacePrivate
{
public:
    NMNetworkInterfacePrivate( const QString & objPath )
     : iface( "org.freedesktop.NetworkManager",
              objPath,
              "org.freedesktop.NetworkManager.Devices",
              QDBusConnection::systemBus() ),
       objectPath( objPath ) { }
    QDBusInterface iface;
    QString objectPath;
    bool active;
    Solid::NetworkInterface::Type type;
    int activationStage;
    bool carrier;
    int signalStrength;
    int designSpeed;
    QMap<QString,NMNetwork *> networks;
    Solid::NetworkInterface::Capabilities capabilities;
};

NMNetworkInterface::NMNetworkInterface( const QString & objectPath )
: NetworkInterface( 0 ), d( new NMNetworkInterfacePrivate( objectPath ) )
{
    QDBusMessage reply = d->iface.call( "getProperties" );
    NMDBusDeviceProperties dev;
    deserialize( reply, dev );
    dump( dev );
}

NMNetworkInterface::~NMNetworkInterface()
{
    delete d;
}

QString NMNetworkInterface::uni() const
{
    return d->objectPath;
}

bool NMNetworkInterface::isActive() const
{
    return d->active;
}

Solid::NetworkInterface::Type NMNetworkInterface::type() const
{
    return d->type;
}

Solid::NetworkInterface::ConnectionState NMNetworkInterface::connectionState() const
{
    // do something with activationstage
    return Solid::NetworkInterface::UnknownState;
}

int NMNetworkInterface::signalStrength() const
{
    return d->signalStrength;
}

int NMNetworkInterface::designSpeed() const
{
    return d->designSpeed;
}

bool NMNetworkInterface::isLinkUp() const
{
    return d->carrier;
}

Solid::NetworkInterface::Capabilities NMNetworkInterface::capabilities() const
{
    return d->capabilities;
}

QObject * NMNetworkInterface::createNetwork( const QString & uni )
{
    return 0; // TODO implement
}

QStringList NMNetworkInterface::networks() const
{
    return d->networks.keys();
}

void NMNetworkInterface::setProperties( const NMDBusDeviceProperties & props )
{
    // store props
}

void NMNetworkInterface::setSignalStrength( int strength )
{
    d->signalStrength = strength;
}

void NMNetworkInterface::setCarrierOn( bool on )
{
    d->carrier = on;
}

void NMNetworkInterface::setActivationStage( uint activationStage )
{
    // convert 
}

#include "NetworkManager-networkinterface.moc"
