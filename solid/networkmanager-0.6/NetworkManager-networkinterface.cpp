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
#include "NetworkManager-networkinterface_p.h"

#include <NetworkManager/NetworkManager.h>
#include <QtDBus>

#include <kdebug.h>

#include <solid/control/networkinterface.h>

#include "NetworkManager-wirelessaccesspoint.h"
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

void dump(const NMDBusAccessPointProperties  & network)
{
    kDebug(1441) << "dump(const NMDBusNetworkProperties &)\n    IPV4 address: " << network.ipv4Address
        << "\n    subnet mask: " << network.subnetMask << "\n    Broadcast: " << network.broadcast
        << "\n    route: " << network.route << "\n    primary dns: " << network.primaryDNS
        << "\n    secondary dns: " << network.secondaryDNS << endl;
}

void deserialize(const QDBusMessage &message, NMDBusDeviceProperties  & device, NMDBusAccessPointProperties  & network)
{
    //kDebug(1441) << /*"deserialize args: " << message.arguments() << */"signature: " << message.signature();
    QList<QVariant> args = message.arguments();
    device.path = ((args.size() != 0) ? args.takeFirst().value<QDBusObjectPath>() : QDBusObjectPath());
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


NMNetworkInterfacePrivate::NMNetworkInterfacePrivate(const QString  & objPath)
     : iface("org.freedesktop.NetworkManager",
              objPath,
              "org.freedesktop.NetworkManager.Devices",
              QDBusConnection::systemBus()),
       objectPath(objPath) { }

NMNetworkInterfacePrivate::~NMNetworkInterfacePrivate()
{
}


NMNetworkInterface::NMNetworkInterface(const QString  & objectPath)
: NetworkInterface(), QObject(), d_ptr(new NMNetworkInterfacePrivate(objectPath))
{
    Q_D(NMNetworkInterface);
    d->q_ptr = this;
    d->initGeneric();
}

NMNetworkInterface::NMNetworkInterface(NMNetworkInterfacePrivate &dd)
    : NetworkInterface(), QObject(), d_ptr(&dd)
{
    Q_D(NMNetworkInterface);
    d->q_ptr = this;
    d->initGeneric();
}


void NMNetworkInterfacePrivate::initGeneric()
{
    Q_Q(NMNetworkInterface);
    QDBusMessage reply = iface.call("getProperties");
    NMDBusDeviceProperties dev;
    NMDBusAccessPointProperties net;
    deserialize(reply, dev, net);
    //dump(dev);
    //dump(net);
    q->setProperties(dev);
    // insert empty networks in our map.  These will be expanded on demand
    foreach (QString netPath, dev.networks)
        networks.insert(netPath, 0);

    if (type == Solid::Control::NetworkInterface::Ieee8023)
    {
        QString fakeNetPath = objectPath + "/Networks/ethernet";
        networks.insert(fakeNetPath, 0);
        cachedNetworkProps.first = fakeNetPath;
        cachedNetworkProps.second = net;
    }
    else if (type == Solid::Control::NetworkInterface::Ieee80211)
    {
        cachedNetworkProps.first = dev.activeNetPath;
        cachedNetworkProps.second = net;
    }
}

NMNetworkInterface::~NMNetworkInterface()
{
    delete d_ptr;
}

QString NMNetworkInterface::uni() const
{
    Q_D(const NMNetworkInterface);
    return d->objectPath;
}

bool NMNetworkInterface::isActive() const
{
    Q_D(const NMNetworkInterface);
    return d->active;
}

Solid::Control::NetworkInterface::Type NMNetworkInterface::type() const
{
    Q_D(const NMNetworkInterface);
    return d->type;
}

Solid::Control::NetworkInterface::ConnectionState NMNetworkInterface::connectionState() const
{
    Q_D(const NMNetworkInterface);
    return (Solid::Control::NetworkInterface::ConnectionState)d->activationStage;
}

int NMNetworkInterface::signalStrength() const
{
    Q_D(const NMNetworkInterface);
    return d->signalStrength;
}

int NMNetworkInterface::designSpeed() const
{
    Q_D(const NMNetworkInterface);
    return d->designSpeed;
}

bool NMNetworkInterface::isLinkUp() const
{
    Q_D(const NMNetworkInterface);
    return d->carrier;
}

Solid::Control::NetworkInterface::Capabilities NMNetworkInterface::capabilities() const
{
    Q_D(const NMNetworkInterface);
    return d->capabilities;
}

#if 0
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
#endif

QStringList NMNetworkInterface::networks() const
{
    Q_D(const NMNetworkInterface);
    return d->networks.keys();
}

QString NMNetworkInterface::activeNetwork() const
{
    Q_D(const NMNetworkInterface);
    return d->activeNetPath;
}

void NMNetworkInterface::setProperties(const NMDBusDeviceProperties  & props)
{
    Q_D(NMNetworkInterface);
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
#if 0
    if (props.capabilities  & NM_DEVICE_CAP_WIRELESS_SCAN)
        d->capabilities |= Solid::Control::NetworkInterface::SupportsWirelessScan;
#endif
    d->activeNetPath = props.activeNetPath;
    d->interface = props.interface;
}

void NMNetworkInterface::setSignalStrength(int strength)
{
    Q_D(NMNetworkInterface);
    d->signalStrength = strength;
#if 0
    // update the network object
    if (d->networks.contains(d->activeNetPath))
    {
        NMWirelessNetwork * net = qobject_cast<NMWirelessNetwork *>(d->networks[d->activeNetPath]);
        if (net != 0)
        {
            net->setSignalStrength(strength);
        }
    }
#endif
#if 0
    emit signalStrengthChanged(strength);
#endif
}

void NMNetworkInterface::setCarrierOn(bool on)
{
    Q_D(NMNetworkInterface);
    d->carrier = on;
#if 0
    emit linkUpChanged(on);
#endif
}

void NMNetworkInterface::setActive(bool active)
{
    Q_D(NMNetworkInterface);
    d->active = active;
#if 0
    emit activeChanged(active);
#endif
}

void NMNetworkInterface::setActivationStage(int activationStage)
{
    Q_D(NMNetworkInterface);
    d->activationStage = activationStage;
#if 0
    emit connectionStateChanged(activationStage);
#endif
}

void NMNetworkInterface::addNetwork(const QDBusObjectPath  & netPath)
{
    Q_D(NMNetworkInterface);
    // check that it's not already present, as NetworkManager may
    // detect networks that aren't really new.
    if (!d->networks.contains(netPath.path()))
        d->networks.insert(netPath.path(), 0);
}

void NMNetworkInterface::removeNetwork(const QDBusObjectPath  & netPath)
{
    Q_D(NMNetworkInterface);
    d->networks.remove(netPath.path());
}

void NMNetworkInterface::updateNetworkStrength(const QDBusObjectPath  & netPath, int strength)
{
    Q_D(const NMNetworkInterface);
    // check that it's not already present, as NetworkManager may
    // detect networks that aren't really new.
    if (d->networks.contains(netPath.path()))
    {
#if 0
        NMNetwork * net = d->networks[netPath.path()];
        if (net != 0)
        {
            NMWirelessNetwork * wlan =  qobject_cast<NMWirelessNetwork *>(net);
            if (wlan != 0)
                wlan->setSignalStrength(strength);
        }
#endif
    }
}

QString NMNetworkInterface::interfaceName() const
{
    Q_D(const NMNetworkInterface);
    return d->interface;
}

QString NMNetworkInterface::driver() const
{
#warning implement me!
    kDebug();
    return QString();
}

Solid::Control::IPv4Config NMNetworkInterface::ipV4Config() const
{
#warning implement me!
    kDebug();
    return Solid::Control::IPv4Config();
}

QString NMNetworkInterface::activeConnection() const
{
#warning implement me!
    kDebug();
    return QString();
}

#include "NetworkManager-networkinterface.moc"
