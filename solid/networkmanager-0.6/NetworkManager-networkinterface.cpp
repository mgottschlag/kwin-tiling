/*  This file is part of the KDE project
    Copyright (C) 2007 Will Stephenson <wstephenson@kde.org>
    Copyright (C) 2008 Pino Toscano <pino@kde.org>

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

#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusReply>

#include <kdebug.h>

#include <solid/control/networkinterface.h>

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

void deserialize(const QDBusMessage &message, NMDBusDeviceProperties & device)
{
    //kDebug(1441) << /*"deserialize args: " << message.arguments() << */"signature: " << message.signature();
    QList<QVariant> args = message.arguments();
    device.path = ((args.size() != 0) ? args.takeFirst().value<QDBusObjectPath>() : QDBusObjectPath());
    device.interface = (args.size() != 0) ? args.takeFirst().toString() : QString();
    device.type = (args.size() != 0) ? args.takeFirst().toUInt() : 0;
    device.udi = (args.size() != 0) ? args.takeFirst().toString() : QString();
    device.active = (args.size() != 0) ? args.takeFirst().toBool() : false;
    device.activationStage = (args.size() != 0) ? args.takeFirst().toUInt() : 0;
    device.ipv4Address = (args.size() != 0) ? args.takeFirst().toString() : QString();
    device.subnetMask = (args.size() != 0) ? args.takeFirst().toString() : QString();
    device.broadcast = (args.size() != 0) ? args.takeFirst().toString() : QString();
    device.hardwareAddress = (args.size() != 0) ? args.takeFirst().toString() : QString();
    device.route = (args.size() != 0) ? args.takeFirst().toString() : QString();
    device.primaryDNS = (args.size() != 0) ? args.takeFirst().toString() : QString();
    device.secondaryDNS = (args.size() != 0) ? args.takeFirst().toString() : QString();
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
       objectPath(objPath)
    , manager(0)
    , active(false)
    , type(Solid::Control::NetworkInterface::UnknownType)
    , activationStage(Solid::Control::NetworkInterface::UnknownState)
    , designSpeed(0)
{
}

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
    QDBusMessage reply = iface.call("getProperties");
    NMDBusDeviceProperties dev;
    deserialize(reply, dev);
    //dump(dev);
    applyProperties(dev);
    QDBusReply<QString> dbusdriver = iface.call("getDriver");
    if (dbusdriver.isValid())
        driver = dbusdriver.value();
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
    Solid::Control::NetworkInterface::ConnectionState state = Solid::Control::NetworkInterface::UnknownState;
    switch (d->activationStage)
    {
    default:
    case NM_ACT_STAGE_UNKNOWN:
        state = Solid::Control::NetworkInterface::UnknownState;
        break;
    case NM_ACT_STAGE_DEVICE_PREPARE:
        state = Solid::Control::NetworkInterface::Preparing;
        break;
    case NM_ACT_STAGE_DEVICE_CONFIG:
        state = Solid::Control::NetworkInterface::Configuring;
        break;
    case NM_ACT_STAGE_NEED_USER_KEY:
        state = Solid::Control::NetworkInterface::NeedAuth;
        break;
    case NM_ACT_STAGE_IP_CONFIG_START:
        state = Solid::Control::NetworkInterface::IPConfig;
        break;
    case NM_ACT_STAGE_IP_CONFIG_GET:
        state = Solid::Control::NetworkInterface::IPConfig;
        break;
    case NM_ACT_STAGE_IP_CONFIG_COMMIT:
        state = Solid::Control::NetworkInterface::IPConfig;
        break;
    case NM_ACT_STAGE_ACTIVATED:
        state = Solid::Control::NetworkInterface::Activated;
        break;
    case NM_ACT_STAGE_FAILED:
        state = Solid::Control::NetworkInterface::Failed;
        break;
    case NM_ACT_STAGE_CANCELLED:
        state = Solid::Control::NetworkInterface::Disconnected;
        break;
    }
    return state;
}

int NMNetworkInterface::designSpeed() const
{
    Q_D(const NMNetworkInterface);
    return d->designSpeed;
}

Solid::Control::NetworkInterface::Capabilities NMNetworkInterface::capabilities() const
{
    Q_D(const NMNetworkInterface);
    return d->capabilities;
}

QString NMNetworkInterface::activeNetwork() const
{
    Q_D(const NMNetworkInterface);
    return d->activeNetPath;
}

void NMNetworkInterfacePrivate::applyProperties(const NMDBusDeviceProperties & props)
{
    switch (props.type)
    {
    case DEVICE_TYPE_UNKNOWN:
        type = Solid::Control::NetworkInterface::UnknownType;
        break;
    case DEVICE_TYPE_802_3_ETHERNET:
        type = Solid::Control::NetworkInterface::Ieee8023;
        break;
    case DEVICE_TYPE_802_11_WIRELESS:
        type = Solid::Control::NetworkInterface::Ieee80211;
        break;
    default:
        type = Solid::Control::NetworkInterface::UnknownType;
        break;
    }
    active = props.active;
    activationStage = props.activationStage;
    designSpeed = props.speed;
    if (props.capabilities  & NM_DEVICE_CAP_NM_SUPPORTED)
        capabilities |= Solid::Control::NetworkInterface::IsManageable;
    if (props.capabilities  & NM_DEVICE_CAP_CARRIER_DETECT)
        capabilities |= Solid::Control::NetworkInterface::SupportsCarrierDetect;
#if 0
    if (props.capabilities  & NM_DEVICE_CAP_WIRELESS_SCAN)
        capabilities |= Solid::Control::NetworkInterface::SupportsWirelessScan;
#endif
    activeNetPath = props.activeNetPath;
    interface = props.interface;
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
    emit connectionStateChanged(connectionState());
}

void NMNetworkInterface::addNetwork(const QDBusObjectPath  & netPath)
{
    Q_D(NMNetworkInterface);
    d->notifyNewNetwork(netPath);
}

void NMNetworkInterface::removeNetwork(const QDBusObjectPath  & netPath)
{
    Q_D(NMNetworkInterface);
    d->notifyRemoveNetwork(netPath);
}

void NMNetworkInterface::setManagerInterface(QDBusInterface * manager)
{
    Q_D(NMNetworkInterface);
    d->manager = manager;
}

QString NMNetworkInterface::interfaceName() const
{
    Q_D(const NMNetworkInterface);
    return d->interface;
}

QString NMNetworkInterface::driver() const
{
    Q_D(const NMNetworkInterface);
    return d->driver;
}

Solid::Control::IPv4Config NMNetworkInterface::ipV4Config() const
{
#warning NMNetworkInterface::ipV4Config() is unimplemented
    kDebug();
    return Solid::Control::IPv4Config();
}

bool NMNetworkInterface::activateConnection(const QString & connectionUni, const QVariantMap & connectionParameters)
{
    Q_UNUSED(connectionUni)
    Q_UNUSED(connectionParameters)
    return false;
}

bool NMNetworkInterface::deactivateConnection()
{
    return false;
}

#include "NetworkManager-networkinterface.moc"
