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
#include <ksocketaddress.h>

#include "NetworkManager-network.h"

class NMNetworkPrivate
{
public:
    NMNetworkPrivate( const QString & networkPath ) : netPath( networkPath ) { }
    QString netPath;
    QList<KNetwork::KIpAddress> ipv4List;
    QString subnetMask;
    QString broadcast;
    QString route;
    QList<KNetwork::KIpAddress> dnsServers;
    bool active;
};

NMNetwork::NMNetwork( const QString & netPath )
 : QObject(), d( new NMNetworkPrivate( netPath ) )
{
}

NMNetwork::~NMNetwork()
{
    delete d;
}

QString NMNetwork::uni() const
{
    return d->netPath;
}

QList<KNetwork::KIpAddress> NMNetwork::ipV4Addresses() const
{
    return d->ipv4List;
}

QList<KNetwork::KIpAddress> NMNetwork::ipV6Addresses() const
{
    return QList<KNetwork::KIpAddress>();
}

QString NMNetwork::subnetMask() const
{
    return d->subnetMask;
}

QString NMNetwork::broadcastAddress() const
{
    return d->broadcast;
}

QString NMNetwork::route() const
{
    return d->route;
}

QList<KNetwork::KIpAddress> NMNetwork::dnsServers() const
{
    return d->dnsServers;
}

bool NMNetwork::isActive() const
{
    return d->active;
}

void NMNetwork::setActivated( bool activated )
{
    // todo activate the device network here
    d->active = activated;
    emit activationStateChanged( activated );
}

void NMNetwork::setProperties( const NMDBusNetworkProperties & props )
{
    d->ipv4List.append( KNetwork::KIpAddress( props.ipv4Address ) );
    d->subnetMask = props.subnetMask;
    d->broadcast = props.broadcast;
    d->route = props.route;
    d->dnsServers.append( props.primaryDNS );
    d->dnsServers.append( props.secondaryDNS );
}

#include "NetworkManager-network.moc"
