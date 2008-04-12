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

#include "NetworkManager-networkinterface.h"

#include <NetworkManager/NetworkManager.h>
#include <QtDBus>

#include <kdebug.h>

#include <solid/control/networkinterface.h>

#include "NetworkManager-network.h"
#include "NetworkManager-wirelessnetwork.h"

void dump(const NMDBusDeviceProperties &device)
{
    kDebug(1441) << "dump(const NMDBusDeviceProperties &device):\n    Object path: " << device.path.path() << "\n    Interface: " << device.interface
        << "\n    Type: " << device.type << "\n    Udi: " << device.udi << "\n    Active: "<< device.active
        << "\n    Activation stage: " << device.activationStage 
        << "\n    Hardware address: " << device.hardwareAddress << "\n    mode: " << device.mode
        << "\n    Strength: " << device.strength << "\n    Link active: " << device.linkActive
        << "\n    Speed: " << device.speed << "\n    Capabilities: " << device.capabilities
        << "\n    Capabilities type: " << device.capabilitiesType << "\n    active net path: "
        << device.activeNetPath << "\n    Networks:" << device.networks << endl;
}

void dump(const NMDBusNetworkProperties  & network)
{
    kDebug(1441) << "dump(const NMDBusNetworkProperties &)\n    IPV4 address: " << network.ipv4Address
        << "\n    subnet mask: " << network.subnetMask << "\n    Broadcast: " << network.broadcast
        << "\n    route: " << network.route << "\n    primary dns: " << network.primaryDNS
        << "\n    secondary dns: " << network.secondaryDNS << endl;
}

void deserialize(const QDBusMessage &message, NMDBusDeviceProperties  & device, NMDBusNetworkProperties  & network)
{
    //kDebug(1441) << /*"deserialize args: " << message.arguments() << */"signature: " << message.signature();
    QList<QVariant> args = message.arguments();
    device.path.setPath((args.size() != 0) ? args.takeFirst().toString() : QString());
    device.interface = (args.size() != 0) ? args.takeFirst().toString() : QString();
    device.type = (args.size() != 0) ? args.takeFirst().toUInt() : 0;
    device.udi = (args.size() != 0) ? args.takeFirst().toString() : QString();
    device.active = (args.size() != 0) ? args.takeFirst().toBool() : false;
    device.activationStage = (args.size() != 0) ? args.takeFirst().toUInt() : 0;
    network.ipv4Address = (args.size() != 0) ? args.takeFirst().toString() : QString();
    network.subnetMask = (args.size() != 0) ? args.takeFirst().toString() : QString();
    network.broadcast = (args.size() != 0) ? args.takeFirst().toString() : QString();
    device.hardwareAddress = (args.size() != 0) ? args.takeFirst().toString() : QString();
    network.route = (args.size() != 0) ? args.takeFirst().toString() : QString();
    network.primaryDNS = (args.size() != 0) ? args.takeFirst().toString() : QString();
    network.secondaryDNS = (args.size() != 0) ? args.takeFirst().toString() : QString();
    device.mode = (args.size() != 0) ? args.takeFirst().toInt() : 0;
    device.strength = (args.size() != 0) ? args.takeFirst().toInt() : 0;
    device.linkActive = (args.size() != 0) ? args.takeFirst().toBool() : false;
    device.speed = (args.size() != 0) ? args.takeFirst().toInt() : 0;
    device.capabilities = (args.size() != 0) ? args.takeFirst().toUInt() : 0;
    device.capabilitiesType = (args.size() != 0) ? args.takeFirst().toUInt() : 0;
    device.activeNetPath = (args.size() != 0) ? args.takeFirst().toString() : QString();
    device.networks = (args.size() != 0) ? args.takeFirst().toStringList() : QStringList();
}

class NMNetworkInterfacePrivate
{
public:
    NMNetworkInterfacePrivate(const QString  & objPath)
     : iface("org.freedesktop.NetworkManager",
              objPath,
              "org.freedesktop.NetworkManager.Devices",
              QDBusConnection::systemBus()),
       objectPath(objPath) { }
    QDBusInterface iface;
    QString objectPath;
    bool active;
    Solid::Control::NetworkInterface::Type type;
    int activationStage;
    bool carrier;
    int signalStrength;
    int designSpeed;
    QMap<QString,NMNetwork *> networks;
    QPair<QString, NMDBusNetworkProperties> cachedNetworkProps;
    Solid::Control::NetworkInterface::Capabilities capabilities;
    QString activeNetPath;
};

NMNetworkInterface::NMNetworkInterface(const QString  & objectPath)
: NetworkInterface(0), d(new NMNetworkInterfacePrivate(objectPath))
{
    QDBusMessage reply = d->iface.call("getProperties");
    NMDBusDeviceProperties dev;
    NMDBusNetworkProperties net;
    deserialize(reply, dev, net);
    //dump(dev);
    //dump(net);
    setProperties(dev);
    // insert empty networks in our map.  These will be expanded on demand
    foreach (QString netPath, dev.networks)
        d->networks.insert(netPath, 0);

    if (d->type == Solid::Control::NetworkInterface::Ieee8023)
    {
        QString fakeNetPath = objectPath + "/Networks/ethernet";
        d->networks.insert(fakeNetPath, 0);
        d->cachedNetworkProps.first = fakeNetPath;
        d->cachedNetworkProps.second = net;
    }
    else if (d->type == Solid::Control::NetworkInterface::Ieee80211)
    {
        d->cachedNetworkProps.first = dev.activeNetPath;
        d->cachedNetworkProps.second = net;
    }
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

Solid::Control::NetworkInterface::Type NMNetworkInterface::type() const
{
    return d->type;
}

Solid::Control::NetworkInterface::ConnectionState NMNetworkInterface::connectionState() const
{
    return (Solid::Control::NetworkInterface::ConnectionState)d->activationStage;
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

Solid::Control::NetworkInterface::Capabilities NMNetworkInterface::capabilities() const
{
    return d->capabilities;
}

QObject * NMNetworkInterface::createNetwork(const QString  & uni)
{
    kDebug(1441) << "NMNetworkInterface::createNetwork() - " << uni;
    NMNetwork * net = 0;
    if (d->networks.contains(uni) && d->networks[uni] != 0)
        net = d->networks[uni];
    else
    {
        if (d->type == Solid::Control::NetworkInterface::Ieee8023)
        {
            net = new NMNetwork(uni);
            //net->setActivated(true);
        }
        else if (d->type == Solid::Control::NetworkInterface::Ieee80211)
        {
            net = new NMWirelessNetwork(uni);
        }
        if (d->cachedNetworkProps.first == uni)
            net->setProperties(d->cachedNetworkProps.second);
        d->networks.insert(uni, net);
    }
    return net;
}

QStringList NMNetworkInterface::networks() const
{
    return d->networks.keys();
}

QString NMNetworkInterface::activeNetwork() const
{
    return d->activeNetPath;
}

void NMNetworkInterface::setProperties(const NMDBusDeviceProperties  & props)
{
    switch (props.type)
    {
    case DEVICE_TYPE_UNKNOWN:
        d->type = Solid::Control::NetworkInterface::UnknownType;
        break;
    case DEVICE_TYPE_802_3_ETHERNET:
        d->type = Solid::Control::NetworkInterface::Ieee8023;
        break;
    case DEVICE_TYPE_802_11_WIRELESS:
        d->type = Solid::Control::NetworkInterface::Ieee80211;
        break;
    default:
        d->type = Solid::Control::NetworkInterface::UnknownType;
        break;
    }
    d->active = props.active;
    d->activationStage = props.activationStage;
    d->carrier = props.linkActive;
    d->signalStrength = props.strength;
    d->designSpeed = props.speed;
    //d->networks
    d->capabilities = 0;
    if (props.capabilities  & NM_DEVICE_CAP_NM_SUPPORTED)
        d->capabilities |= Solid::Control::NetworkInterface::IsManageable;
    if (props.capabilities  & NM_DEVICE_CAP_CARRIER_DETECT)
        d->capabilities |= Solid::Control::NetworkInterface::SupportsCarrierDetect;
    if (props.capabilities  & NM_DEVICE_CAP_WIRELESS_SCAN)
        d->capabilities |= Solid::Control::NetworkInterface::SupportsWirelessScan;
    d->activeNetPath = props.activeNetPath;
}

void NMNetworkInterface::setSignalStrength(int strength)
{
    d->signalStrength = strength;
    // update the network object
    if (d->networks.contains(d->activeNetPath))
    {
        NMWirelessNetwork * net = qobject_cast<NMWirelessNetwork *>(d->networks[d->activeNetPath]);
        if (net != 0)
        {
            net->setSignalStrength(strength);
        }
    }
    emit signalStrengthChanged(strength);
}

void NMNetworkInterface::setCarrierOn(bool on)
{
    d->carrier = on;
    emit linkUpChanged(on);
}

void NMNetworkInterface::setActive(bool active)
{
    d->active = active;
    emit activeChanged(active);
}

void NMNetworkInterface::setActivationStage(int activationStage)
{
    d->activationStage = activationStage;
    emit connectionStateChanged(activationStage);
}

void NMNetworkInterface::addNetwork(const QDBusObjectPath  & netPath)
{
    // check that it's not already present, as NetworkManager may
    // detect networks that aren't really new.
    if (!d->networks.contains(netPath.path()))
        d->networks.insert(netPath.path(), 0);
}

void NMNetworkInterface::removeNetwork(const QDBusObjectPath  & netPath)
{
    d->networks.remove(netPath.path());
}

void NMNetworkInterface::updateNetworkStrength(const QDBusObjectPath  & netPath, int strength)
{
    // check that it's not already present, as NetworkManager may
    // detect networks that aren't really new.
    if (d->networks.contains(netPath.path()))
    {
        NMNetwork * net = d->networks[netPath.path()];
        if (net != 0)
        {
            NMWirelessNetwork * wlan =  qobject_cast<NMWirelessNetwork *>(net);
            if (wlan != 0)
                wlan->setSignalStrength(strength);
        }
    }
}
#include "NetworkManager-networkinterface.moc"
