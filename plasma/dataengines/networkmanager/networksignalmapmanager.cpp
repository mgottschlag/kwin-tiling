/*
 *   Copyright (C) 2007 Christopher Blauvelt <cblauvelt@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "networksignalmapmanager.h"

NetworkSignalMapManager::NetworkSignalMapManager(QObject *parent) : QObject(parent)
{
    user = parent;
}

NetworkSignalMapManager::~NetworkSignalMapManager()
{
}

void NetworkSignalMapManager::mapNetwork(Solid::Control::NetworkInterface *iface, const QString &uni)
{
    NetworkInterfaceControlSignalMapper *map=0;
    if(!signalmap.contains(NetworkInterfaceControl)) {
        signalmap[NetworkInterfaceControl] = new NetworkInterfaceControlSignalMapper();
    }
    map = (NetworkInterfaceControlSignalMapper*)signalmap[NetworkInterfaceControl];

    connect(iface, SIGNAL(ipDetailsChanged()), map, SLOT(ipDetailsChanged()));
    connect(iface, SIGNAL(linkUpChanged(bool)), map, SLOT(linkUpChanged(bool)));
    connect(iface, SIGNAL(connectionStateChanged(int)), map, SLOT(connectionStateChanged(int)));

    //for wired devices only
    if (iface->type() == Solid::Control::NetworkInterface::Ieee8023) {
        Solid::Control::WiredNetworkInterface *wiredIface = (Solid::Control::WiredNetworkInterface*)iface;
        connect(wirediface, SIGNAL(bitRateChanged(int)), map, SLOT(bitRateChanged(int)));
        connect(wiredIface, SIGNAL(carrierChanged(bool)), map, SLOT(carrierChanged(bool)));
    }

    //for wireless devices only
    if (iface->type() == Solid::Control::NetworkInterface::Ieee80211) {
        Solid::Control::WirelessNetworkInterface *wirelessIface = (Solid::Control::WirelessNetworkInterface*)iface;
        connect(wirelessIface, SIGNAL(bitRateChanged(int)), map, SLOT(bitRateChanged(int)));
        connect(wirelessIface, SIGNAL(activeAccessPointChanged(const QString&)), map, SLOT(activeAccessPointChanged(const QString&)));
        connect(wirelessIface, SIGNAL(modeChanged(Solid::Control::WirelessNetworkInterface::OperationMode)), map, SLOT(modeChanged(Solid::Control::WirelessNetworkInterface::OperationMode)));
        connect(wirelessIface, SIGNAL(accessPointAppeared(const QString&)), map, SLOT(accessPointAppeared(const QString&)));
        connect(wirelessIface, SIGNAL(accessPointDisappeared(const QString&)), map, SLOT(accessPointDisappeared(const QString&)));
    }
    
    connect(map, SIGNAL(networkChanged(const QString&, const QString &, const QVariant&)), user, SLOT(networkChanged(const QString&, const QString &, const QVariant&)));
    map->setMapping(iface, uni);
}

void NetworkSignalMapManager::mapNetwork(Solid::Control::AccessPoint *accessPoint, const QString &uni)
{
    NetworkControlSignalMapper *map=0;
    if(!signalmap.contains(AccessPointControl)) {
        signalmap[AccessPointControl] = new AccessPointControlSignalMapper();
    }
    map = (AccessPointControlSignalMapper*)signalmap[AccessPointControl];

    connect(accessPoint, SIGNAL(signalStrengthChanged(int)), map, SLOT(signalStrengthChanged(int)));
    connect(accessPoint, SIGNAL(bitRateChanged(int)), map, SLOT(bitRateChanged(int)));
    connect(accessPoint, SIGNAL(wpaFlagsChanged(Solid::Control::AccessPoint::WpaFlags)), map, SLOT(wpaFlagsChanged(Solid::Control::AccessPoint::WpaFlags)));
    connect(accessPoint, SIGNAL(rsnFlagsChanged(Solid::Control::AccessPoint::WpaFlags)), map, SLOT(rsnFlagsChanged(Solid::Control::AccessPoint::WpaFlags)));
    connect(accessPoint, SIGNAL(ssidChanged(const QString&)), map, SLOT(ssidChanged(const QString&)));
    connect(accessPoint, SIGNAL(frequencyChanged(double)), map, SLOT(frequencyChanged(double)));
    
    connect(map, SIGNAL(networkChanged(const QString&, const QString &, QVariant)), user, SLOT(networkChanged(const QString&, const QString &, QVariant)));
    map->setMapping(accessPoint, uni);
}

void NetworkSignalMapManager::unmapNetwork(Solid::Control::NetworkInterface *iface)
{
    if(!signalmap.contains(NetworkInterfaceControl)) {
        return;
    }
    NetworkInterfaceControlSignalMapper *map = (NetworkInterfaceControlSignalMapper*)signalmap[NetworkInterfaceControl];

    disconnect(iface, SIGNAL(ipDetailsChanged()), map, SLOT(ipDetailsChanged()));
    disconnect(iface, SIGNAL(linkUpChanged(bool)), map, SLOT(linkUpChanged(bool)));
    disconnect(iface, SIGNAL(connectionStateChanged(int)), map, SLOT(connectionStateChanged(int)));

    //for wired devices only
    if (iface->type() == Solid::Control::NetworkInterface::Ieee8023) {
        Solid::Control::WiredNetworkInterface *wiredIface = (Solid::Control::WiredNetworkInterface*)iface;
        disconnect(wirediface, SIGNAL(bitRateChanged(int)), map, SLOT(bitRateChanged(int)));
        disconnect(wiredIface, SIGNAL(carrierChanged(bool)), map, SLOT(carrierChanged(bool)));
    }

    //for wireless devices only
    if (iface->type() == Solid::Control::NetworkInterface::Ieee80211) {
        Solid::Control::WirelessNetworkInterface *wirelessIface = (Solid::Control::WirelessNetworkInterface*)iface;
        disconnect(wirelessIface, SIGNAL(bitRateChanged(int)), map, SLOT(bitRateChanged(int)));
        disconnect(wirelessIface, SIGNAL(activeAccessPointChanged(const QString&)), map, SLOT(activeAccessPointChanged(const QString&)));
        disconnect(wirelessIface, SIGNAL(modeChanged(Solid::Control::WirelessNetworkInterface::OperationMode)), map, SLOT(modeChanged(Solid::Control::WirelessNetworkInterface::OperationMode)));
        disconnect(wirelessIface, SIGNAL(accessPointAppeared(const QString&)), map, SLOT(accessPointAppeared(const QString&)));
        disconnect(wirelessIface, SIGNAL(accessPointDisappeared(const QString&)), map, SLOT(accessPointDisappeared(const QString&)));
    }
    disconnect(map, SIGNAL(networkChanged(const QString&, const QString &, QVariant)), user, SLOT(networkChanged(const QString&, const QString &, QVariant)));
    
}

void NetworkSignalMapManager::unmapNetwork(Solid::Control::Network *network)
{
    if(!signalmap.contains(NetworkControl)) {
        return;
    }
    NetworkControlSignalMapper *map = (NetworkControlSignalMapper*)signalmap[NetworkControl];

    disconnect(accessPoint, SIGNAL(signalStrengthChanged(int)), map, SLOT(signalStrengthChanged(int)));
    disconnect(accessPoint, SIGNAL(bitRateChanged(int)), map, SLOT(bitRateChanged(int)));
    disconnect(accessPoint, SIGNAL(wpaFlagsChanged(Solid::Control::AccessPoint::WpaFlags)), map, SLOT(wpaFlagsChanged(Solid::Control::AccessPoint::WpaFlags)));
    disconnect(accessPoint, SIGNAL(rsnFlagsChanged(Solid::Control::AccessPoint::WpaFlags)), map, SLOT(rsnFlagsChanged(Solid::Control::AccessPoint::WpaFlags)));
    disconnect(accessPoint, SIGNAL(ssidChanged(const QString&)), map, SLOT(ssidChanged(const QString&)));
    disconnect(accessPoint, SIGNAL(frequencyChanged(double)), map, SLOT(frequencyChanged(double)));
}

#include "networksignalmapmanager.moc"
