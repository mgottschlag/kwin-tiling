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

#include <kdebug.h>

#include "NetworkManager-wirelessnetwork.h"

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
    kDebug() << reply.arguments() << endl;
}

NMWirelessNetwork::~NMWirelessNetwork()
{
    delete d;
}

int NMWirelessNetwork::signalStrength() const
{
    return d->strength;
}

int NMWirelessNetwork::bitRate() const
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

#include "NetworkManager-wirelessnetwork.moc"

