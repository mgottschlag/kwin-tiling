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

#include <NetworkManager.h>
#include <QtDBus>

#include <kdebug.h>

#include "NetworkManager-network.h"

#include "NetworkManager-networkinterface.h"

void dump( const NMDBusDeviceProperties& device )
{
    kDebug() << "Object path: " << device.path.path() << "\nInterface: " << device.interface
        << "\nType: " << device.type << "\nUdi: " << device.udi << "\nActive: "<< device.active
        << "\nActivation stage: " << device.activationStage 
        << "\nHardware address: " << device.hardwareAddress << "\nmode: " << device.mode
        << "\nStrength: " << device.strength << "\nLink active: " << device.linkActive
        << "\nSpeed: " << device.speed << "\nCapabilities: " << device.capabilities 
        << "\nCapabilities type: " << device.capabilitiesType << "\nactive net path: "
        << device.activeNetPath << "\nNetworks:" << device.networks << endl;
}

void dump( const NMDBusNetworkProperties & network )
{
    kDebug() << "\nIPV4 address: " << network.ipv4Address
        << "\nsubnet mask: " << network.subnetMask << "\nBroadcast: " << network.broadcast
        << "\nroute: " << network.route << "\nprimary dns: " << network.primaryDNS 
        << "\nsecondary dns: " << network.secondaryDNS << endl;
}

void deserialize( const QDBusMessage &message, NMDBusDeviceProperties & device, NMDBusNetworkProperties & network )
{
    kDebug() << /*"deserialize args: " << message.arguments() << */"signature: " << message.signature() << endl;
    QList<QVariant> args = message.arguments();
    device.path.setPath( args.takeFirst().toString() );
    device.interface = args.takeFirst().toString();
    device.type = args.takeFirst().toUInt();
    device.udi = args.takeFirst().toString();
    device.active = args.takeFirst().toBool();
    device.activationStage = args.takeFirst().toUInt();
    network.ipv4Address = args.takeFirst().toString();
    network.subnetMask = args.takeFirst().toString();
    network.broadcast = args.takeFirst().toString();
    device.hardwareAddress = args.takeFirst().toString();
    network.route = args.takeFirst().toString();
    network.primaryDNS = args.takeFirst().toString();
    network.secondaryDNS = args.takeFirst().toString();
    device.mode = args.takeFirst().toInt();
    device.strength = args.takeFirst().toInt();
    device.linkActive = args.takeFirst().toBool();
    device.speed = args.takeFirst().toInt();
    device.capabilities = args.takeFirst().toUInt();
    device.capabilitiesType = args.takeFirst().toUInt();
    device.activeNetPath = args.takeFirst().toString();
    device.networks = args.takeFirst().toStringList();
}

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
    NMDBusNetworkProperties net;
    deserialize( reply, dev, net );
    dump( dev );
    dump( net );
    setProperties( dev );
    // insert empty networks in our map.  These will be expanded on demand
    foreach( QString netPath, dev.networks )
        d->networks.insert( netPath, 0 );

    if ( d->type == Solid::NetworkInterface::Ieee8023 )
        setNetwork( net );
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
    return ( Solid::NetworkInterface::ConnectionState )d->activationStage;
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
    kDebug() << "NMNetworkInterface::createNetwork() - " << uni << endl;
    NMNetwork * net = 0;
    if ( d->networks.contains( uni ) && d->networks[ uni ] != 0 )
        net = d->networks[ uni ];
    else
    {
        net = new NMNetwork( uni );
        d->networks.insert( uni, net );
    }
    // maybe look up cached NetworkProperties for this uni here...
    return net;
}

QStringList NMNetworkInterface::networks() const
{
    return d->networks.keys();
}

void NMNetworkInterface::setProperties( const NMDBusDeviceProperties & props )
{
    switch ( props.type )
    {
        case DEVICE_TYPE_UNKNOWN:
            d->type = Solid::NetworkInterface::UnknownType;
            break;
        case DEVICE_TYPE_802_3_ETHERNET:
            d->type = Solid::NetworkInterface::Ieee8023;
            break;
        case DEVICE_TYPE_802_11_WIRELESS:
            d->type = Solid::NetworkInterface::Ieee80211;
            break;
        default:
            d->type = Solid::NetworkInterface::UnknownType;
            break;
    }
    d->active = props.active;
    d->activationStage = props.activationStage;
    d->carrier = props.linkActive;
    d->signalStrength = props.strength;
    d->designSpeed = props.speed;
    //d->networks
    d->capabilities = 0;
    if ( props.capabilities & NM_DEVICE_CAP_NM_SUPPORTED )
        d->capabilities |= Solid::NetworkInterface::IsManageable;
    if ( props.capabilities & NM_DEVICE_CAP_CARRIER_DETECT )
        d->capabilities |= Solid::NetworkInterface::SupportsCarrierDetect;
    if ( props.capabilities & NM_DEVICE_CAP_WIRELESS_SCAN )
        d->capabilities |= Solid::NetworkInterface::SupportsWirelessScan;
}

void NMNetworkInterface::setNetwork( const NMDBusNetworkProperties & net )
{
    // we are a wired device, create a network instance which describes the IP network to which we are connected
    QString netPath = d->objectPath + "/networks/ethernet";
    NMNetwork * network = qobject_cast<NMNetwork*>( createNetwork( netPath ) );
    network->setProperties( net );
    network->setActivated( d->active );
}

void NMNetworkInterface::setSignalStrength( int strength )
{
    d->signalStrength = strength;
    emit signalStrengthChanged( strength );
}

void NMNetworkInterface::setCarrierOn( bool on )
{
    d->carrier = on;
    emit linkUpChanged( on );
}

void NMNetworkInterface::setActive( bool active )
{
    d->active = active;
    emit activeChanged( active );
}

void NMNetworkInterface::setActivationStage( int activationStage )
{
    d->activationStage = activationStage;
    emit connectionStateChanged( activationStage );
}

void NMNetworkInterface::addNetwork( const QDBusObjectPath & netPath )
{
    // check that it's not already present, as NetworkManager may
    // detect networks that aren't really new.
    if ( !d->networks.contains( netPath.path() ) )
        d->networks.insert( netPath.path(), 0 );
}

void NMNetworkInterface::removeNetwork( const QDBusObjectPath & netPath )
{
    d->networks.remove( netPath.path() );
}

void NMNetworkInterface::updateNetworkStrength( const QDBusObjectPath & netPath, int strength )
{
    typedef void NMWirelessNetwork;
    // check that it's not already present, as NetworkManager may
    // detect networks that aren't really new.
    if ( !d->networks.contains( netPath.path() ) )
    {
        NMNetwork * net = d->networks[ netPath.path() ];
        if ( net != 0 )
        {
            NMWirelessNetwork * wlan = 0; // qobject_cast<NMWirelessNetwork*>( net );
            if ( wlan != 0 )
                ;
                // fixme wlan->setSignalStrength( strength );
        }
    }
}
#include "NetworkManager-networkinterface.moc"
