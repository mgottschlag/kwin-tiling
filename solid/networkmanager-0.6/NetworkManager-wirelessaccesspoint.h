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

#ifndef NETWORKMANAGER_WIRELESSACCESSPOINT_H
#define NETWORKMANAGER_WIRELESSACCESSPOINT_H

#include <solid/control/ifaces/wirelessaccesspoint.h>

struct NMDBusAccessPointProperties {
    QString ipv4Address;
    QString subnetMask;
    QString broadcast;
    QString route;
    QHostAddress primaryDNS;
    QHostAddress secondaryDNS;
};

class NMAccessPointPrivate;

/**
 * This interface represents a generic Internet Protocol (IP) network which we may be connected to.
 */
class KDE_EXPORT NMAccessPoint : public Solid::Control::Ifaces::WirelessAccessPoint
{
Q_OBJECT
Q_INTERFACES(Solid::Control::Ifaces::WirelessAccessPoint)
public:
    /**
     * Constructs a network and looks up its properties over DBus.
     * @param net contains the IP details of the network.
     */
    NMAccessPoint(const QString  & networkPath);
    virtual ~NMAccessPoint();
    QString uni() const;
    QList<QNetworkAddressEntry> addressEntries() const;
    QString route() const;
    QList<QHostAddress> dnsServers() const;
    bool isActive() const;
    virtual void setActivated(bool activated);

    void setProperties(const NMDBusNetworkProperties  & props);
Q_SIGNALS:
    void ipDetailsChanged();
    void activationStateChanged(bool);
private:
    NMAccessPointPrivate * d;
};

#endif
