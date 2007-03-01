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

// Copied from wireless.h
/* Modes of operation */
#define IW_MODE_AUTO    0   /* Let the driver decides */
#define IW_MODE_ADHOC   1   /* Single cell network */
#define IW_MODE_INFRA   2   /* Multi cell network, roaming, ... */
#define IW_MODE_MASTER  3   /* Synchronisation master or Access Point */
#define IW_MODE_REPEAT  4   /* Wireless Repeater (forwarder) */
#define IW_MODE_SECOND  5   /* Secondary master/repeater (backup) */
#define IW_MODE_MONITOR 6   /* Passive monitor (listen only) */

#include <NetworkManager/NetworkManager.h>

#include <kdebug.h>

#include "NetworkManager-wirelessnetwork.h"

void dump( const Solid::WirelessNetwork::Capabilities & cap )
{
    kDebug() << "WEP      " << ( cap & Solid::WirelessNetwork::Wep ? "X " : " O" ) << endl;
    kDebug() << "WPA      " << ( cap & Solid::WirelessNetwork::Wpa ? "X " : " O" ) << endl;
    kDebug() << "WPA2     " << ( cap & Solid::WirelessNetwork::Wpa2 ? "X " : " O" ) << endl;
    kDebug() << "PSK      " << ( cap & Solid::WirelessNetwork::Psk ? "X " : " O" ) << endl;
    kDebug() << "Ieee8021x" << ( cap & Solid::WirelessNetwork::Ieee8021x ? "X " : " O" ) << endl;
    kDebug() << "Wep40    " << ( cap & Solid::WirelessNetwork::Wep40 ? "X " : " O" ) << endl;
    kDebug() << "Wep104   " << ( cap & Solid::WirelessNetwork::Wep104 ? "X " : " O" ) << endl;
    kDebug() << "Wep192   " << ( cap & Solid::WirelessNetwork::Wep192 ? "X " : " O" ) << endl;
    kDebug() << "Wep256   " << ( cap & Solid::WirelessNetwork::Wep256 ? "X " : " O" ) << endl;
    kDebug() << "WepOther " << ( cap & Solid::WirelessNetwork::WepOther ? "X " : " O" ) << endl;
    kDebug() << "TKIP     " << ( cap & Solid::WirelessNetwork::Tkip ? "X " : " O" ) << endl;
    kDebug() << "CCMP     " << ( cap & Solid::WirelessNetwork::Ccmp ? "X " : " O" ) << endl;
}

void dump( const NMDBusWirelessNetworkProperties & network )
{
    kDebug() << "Object path: " << network.path.path() << "\nESSID: " << network.essid
        << "\nHardware address: " << network.hwAddr << "\nSignal strength: " << network.strength
        << "\nFrequency: " << network.frequency << "\nBit rate: " << network.rate
        << "\nMode: " << network.mode
        << "\nBroadcast: " << network.broadcast << "\nCapabilities: " << endl;
    dump( network.capabilities );
}

Solid::WirelessNetwork::Capabilities getCapabilities( const int nm )
{
    Solid::WirelessNetwork::Capabilities caps;
    if ( nm & NM_802_11_CAP_NONE )
        caps |= Solid::WirelessNetwork::Unencrypted;
    if ( nm & NM_802_11_CAP_PROTO_WEP )
        caps |= Solid::WirelessNetwork::Wep;
    if ( nm & NM_802_11_CAP_PROTO_WPA )
        caps |= Solid::WirelessNetwork::Wpa;
    if ( nm & NM_802_11_CAP_PROTO_WPA2 )
        caps |= Solid::WirelessNetwork::Wpa2;
    if ( nm & NM_802_11_CAP_KEY_MGMT_PSK )
        caps |= Solid::WirelessNetwork::Psk;
    if ( nm & NM_802_11_CAP_KEY_MGMT_802_1X )
        caps |= Solid::WirelessNetwork::Ieee8021x;
    if ( nm & NM_802_11_CAP_CIPHER_WEP40 )
        caps |= Solid::WirelessNetwork::Wep40;
    if ( nm & NM_802_11_CAP_CIPHER_WEP104 )
        caps |= Solid::WirelessNetwork::Wep104;
    if ( nm & NM_802_11_CAP_CIPHER_TKIP )
        caps |= Solid::WirelessNetwork::Tkip;
    if ( nm & NM_802_11_CAP_CIPHER_CCMP )
        caps |= Solid::WirelessNetwork::Ccmp;
    return caps;
}

Solid::WirelessNetwork::OperationMode getOperationMode( const int nm )
{
    Solid::WirelessNetwork::OperationMode mode = Solid::WirelessNetwork::Unassociated;
    switch ( nm )
    {
        case IW_MODE_ADHOC:
            mode = Solid::WirelessNetwork::Adhoc;
            break;
        case IW_MODE_INFRA:
        case IW_MODE_MASTER:
            mode = Solid::WirelessNetwork::Managed;
            break;
        case IW_MODE_REPEAT:
            mode = Solid::WirelessNetwork::Repeater;
            break;
    }
    return mode;
}

void deserialize( const QDBusMessage & message, NMDBusWirelessNetworkProperties & network )
{
    kDebug() << "signature: " << message.signature() << endl;
    QList<QVariant> args = message.arguments();
    network.path.setPath( args.takeFirst().toString() );
    network.essid = args.takeFirst().toString();
    network.hwAddr = args.takeFirst().toString();
    network.strength = args.takeFirst().toInt();
    network.frequency = args.takeFirst().toDouble();
    network.rate = args.takeFirst().toInt();
    network.mode = getOperationMode( args.takeFirst().toInt() );
    network.capabilities = getCapabilities( args.takeFirst().toInt() );
    network.broadcast = args.takeFirst().toBool();
}



typedef void Encryption;

class NMWirelessNetworkPrivate
{
public:
    NMWirelessNetworkPrivate( const QString & netPath )
        : iface( "org.freedesktop.NetworkManager",
                netPath,
                "org.freedesktop.NetworkManager.Devices",
                QDBusConnection::systemBus() ),
        strength( 0 ), frequency( 0.0 ), rate( 0 ), broadcast( true ), authentication( 0 ) { }
    QDBusInterface iface;
    QString essid;
    MacAddressList hwAddr; // MACs of the APs
    int strength;
    double frequency;
    int rate;
    Solid::WirelessNetwork::OperationMode mode;
    Solid::WirelessNetwork::Capabilities capabilities;
    bool broadcast;
    Authentication * authentication;
};

NMWirelessNetwork::NMWirelessNetwork( const QString & networkPath )
 : NMNetwork( networkPath ), d( new NMWirelessNetworkPrivate( networkPath ) )
{
    kDebug() << "NMWirelessNetwork::NMWirelessNetwork() - " << networkPath << endl;
    QDBusMessage reply = d->iface.call( "getProperties" );
    NMDBusWirelessNetworkProperties wlan;
    deserialize( reply, wlan );
    dump( wlan );
}

NMWirelessNetwork::~NMWirelessNetwork()
{
    delete d;
}

void NMWirelessNetwork::setProperties( const NMDBusWirelessNetworkProperties & props )
{
    d->essid = props.essid;
    d->hwAddr.append( props.hwAddr );
    d->strength = props.strength;
    d->frequency = props.frequency;
    d->rate = props.rate;
    d->mode = props.mode;
    d->capabilities = props.capabilities;
    d->broadcast = props.broadcast;
}

int NMWirelessNetwork::signalStrength() const
{
    return d->strength;
}

int NMWirelessNetwork::bitrate() const
{
    return d->rate;
}

double NMWirelessNetwork::frequency() const
{
    return d->frequency;
}

Solid::WirelessNetwork::Capabilities NMWirelessNetwork::capabilities() const
{
    return d->capabilities;
}

QString NMWirelessNetwork::essid() const
{
    return d->essid;
}

Solid::WirelessNetwork::OperationMode NMWirelessNetwork::mode() const
{
    return d->mode;
}

bool NMWirelessNetwork::isAssociated() const
{
    kDebug() << "Fixme: implement NMWirelessNetwork::isAssociated()" << endl;
    return true;
}

bool NMWirelessNetwork::isEncrypted() const
{
    return d->authentication != 0;
}

bool NMWirelessNetwork::isHidden() const
{
    kDebug() << "Fixme: implement NMWirelessNetwork::isHidden()" << endl;
    return true;
}

MacAddressList NMWirelessNetwork::bssList() const
{
    return d->hwAddr;
}

Authentication * NMWirelessNetwork::authentication() const
{
    return d->authentication;
}

void NMWirelessNetwork::setAuthentication( Authentication * auth )
{
    d->authentication = auth;
}

void NMWirelessNetwork::setSignalStrength( int strength )
{
    d->strength = strength;
    emit signalStrengthChanged( strength );
}

void NMWirelessNetwork::setBitrate( int rate )
{
    d->rate = rate;
    emit bitrateChanged( rate );
}

#include "NetworkManager-wirelessnetwork.moc"

