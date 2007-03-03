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

#ifndef NETWORKMANAGER_NETWORK_H
#define NETWORKMANAGER_NETWORK_H

#include <QDBusInterface>
#include <solid/ifaces/network.h>

struct NMDBusNetworkProperties {
	QString ipv4Address;
	QString subnetMask;
	QString broadcast;
	QString route;
	QString primaryDNS;
	QString secondaryDNS;
};

class NMNetworkPrivate;

/**
 * This interface represents a generic Internet Protocol (IP) network which we may be connected to.
 */
class KDE_EXPORT NMNetwork : public QObject, virtual public Solid::Ifaces::Network
{
Q_OBJECT
Q_INTERFACES(Solid::Ifaces::Network)
public:
    /**
     * Constructs a network and looks up its properties over DBus.
     * @param net contains the IP details of the network.
     */
    NMNetwork( const QString & networkPath );
    virtual ~NMNetwork();
    QString uni() const;
    QList<KNetwork::KIpAddress> ipV4Addresses() const;
    QList<KNetwork::KIpAddress> ipV6Addresses() const;
    QString subnetMask() const;
    QString broadcastAddress() const;
    QString route() const;
    QList<KNetwork::KIpAddress> dnsServers() const;
    bool isActive() const;
    void setActivated( bool activated );

    void setProperties( const NMDBusNetworkProperties & props );
Q_SIGNALS:
    void ipDetailsChanged();
    void activationStateChanged( bool );
private:
    NMNetworkPrivate * d;
};

#endif
