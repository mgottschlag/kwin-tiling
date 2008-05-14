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

#include "NetworkManager-wirelessaccesspoint.h"

#include <QtDBus>
#include <kdebug.h>

#include "NetworkManager-wirelessnetwork.h"

class NMAccessPointPrivate
{
public:
    NMAccessPointPrivate(const QString  & networkPath) : netPath(networkPath) { }
    QString netPath;
    QList<QNetworkAddressEntry> addrList;
    QString route;
    QList<QHostAddress> dnsServers;
    bool active;
};

NMAccessPoint::NMAccessPoint(const QString  & netPath)
    : Solid::Control::Ifaces::WirelessAccessPoint(), d(new NMAccessPointPrivate(netPath))
{
}

NMAccessPoint::~NMAccessPoint()
{
    delete d;
}

QString NMAccessPoint::uni() const
{
    return d->netPath;
}

QList<QNetworkAddressEntry> NMAccessPoint::addressEntries() const
{
    return d->addrList;
}

QString NMAccessPoint::route() const
{
    return d->route;
}

QList<QHostAddress> NMAccessPoint::dnsServers() const
{
    return d->dnsServers;
}

bool NMAccessPoint::isActive() const
{
    return d->active;
}

void NMAccessPoint::setActivated(bool activated)
{
    // todo activate the device network here
    d->active = activated;
    QDBusInterface manager("org.freedesktop.NetworkManager",
            "/org/freedesktop/NetworkManager",
            "org.freedesktop.NetworkManager",
            QDBusConnection::systemBus());
    QString devicePath = d->netPath.left(d->netPath.indexOf("/Networks"));
    manager.call("setActiveDevice", qVariantFromValue(QDBusObjectPath(devicePath)));

    emit activationStateChanged(activated);
}

void NMAccessPoint::setProperties(const NMDBusNetworkProperties  & props)
{
    QNetworkAddressEntry addr;
    addr.setIp(QHostAddress(props.ipv4Address));
    addr.setNetmask(QHostAddress(props.subnetMask));
    addr.setBroadcast(QHostAddress(props.broadcast));
    d->addrList.append(addr);
    d->route = props.route;
    d->dnsServers.append(props.primaryDNS);
    d->dnsServers.append(props.secondaryDNS);
}

#include "NetworkManager-network.moc"
