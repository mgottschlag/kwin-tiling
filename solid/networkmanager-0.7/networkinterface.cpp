/*
Copyright 2008 Will Stephenson <wstephenson@kde.org>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of
the License or (at your option) version 3 or any later version
accepted by the membership of KDE e.V. (or its successor approved
by the membership of KDE e.V.), which shall act as a proxy 
defined in Section 14 of version 3 of the license.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "networkinterface.h"
#include "networkinterface_p.h"
#include <KDebug>

#include "manager.h"
#include "networkmanagerdefinitions.h"

// lifted from libnm-glib/nm-device.h, remove when our NM packages have this version
#define NM_DEVICE_UDI "udi"
#define NM_DEVICE_INTERFACE "interface"
#define NM_DEVICE_DRIVER "driver"
#define NM_DEVICE_CAPABILITIES "capabilities"
#define NM_DEVICE_MANAGED "managed"
#define NM_DEVICE_IP4_CONFIG "ip4-config"
#define NM_DEVICE_STATE "state"
#define NM_DEVICE_VENDOR "vendor"
#define NM_DEVICE_PRODUCT "product"

NMNetworkInterfacePrivate::NMNetworkInterfacePrivate( const QString & path, QObject * owner ) : deviceIface(NMNetworkManager::DBUS_SERVICE, path, QDBusConnection::systemBus()), uni(path), designSpeed(0), manager(0), propHelper(owner)
{
    //isLinkUp = deviceIface.isLinkUp();
    driver = deviceIface.driver();
    interfaceName = deviceIface.interface();
    ipV4Address = deviceIface.ip4Address();    
    managed = deviceIface.managed();

    //TODO set active connections based on active connection list on the manager; find out if
    //signal needed
    //activeConnection = deviceIface.activeConnection();
    propHelper.registerProperty(NM_DEVICE_UDI, PropertySignalPair("uni",0));
}

NMNetworkInterface::NMNetworkInterface(const QString & path, NMNetworkManager * manager, QObject * parent) : QObject(parent), d_ptr(new NMNetworkInterfacePrivate(path, this))
{
    Q_D(NMNetworkInterface);
    init();
    d->manager = manager;
}

NMNetworkInterface::NMNetworkInterface(NMNetworkInterfacePrivate & dd, NMNetworkManager * manager, QObject * parent) : QObject(parent), d_ptr(&dd)
{
    Q_D(NMNetworkInterface);
    init();
    d->manager = manager;
}

void NMNetworkInterface::init()
{
    Q_D(NMNetworkInterface);
    d->capabilities = convertCapabilities(d->deviceIface.capabilities());
    d->connectionState = convertState(d->deviceIface.state());

    connect(&d->deviceIface, SIGNAL(StateChanged(uint)), this, SLOT(stateChanged(uint)));
}

NMNetworkInterface::~NMNetworkInterface()
{

}

QString NMNetworkInterface::uni() const
{
    Q_D(const NMNetworkInterface);
    return d->uni;
}

void NMNetworkInterface::setUni(const QVariant & uni)
{
    Q_D(NMNetworkInterface);
    d->uni = uni.toString();
}

QString NMNetworkInterface::interfaceName() const
{
    Q_D(const NMNetworkInterface);
    return d->interfaceName;
}

void NMNetworkInterface::setInterfaceName(const QVariant & name)
{
    Q_D(NMNetworkInterface);
    d->interfaceName = name.toString();
}

QString NMNetworkInterface::driver() const
{
    Q_D(const NMNetworkInterface);
    return d->driver;
}

void NMNetworkInterface::setDriver(const QVariant & driver)
{
    Q_D(NMNetworkInterface);
    d->driver = driver.toString();
}

int NMNetworkInterface::ipV4Address() const
{
    Q_D(const NMNetworkInterface);
    return d->ipV4Address;
}

Solid::Control::IPv4Config NMNetworkInterface::ipV4Config() const
{
#warning TODO NMNetworkInterface::ipV4Config()
    return Solid::Control::IPv4Config();
}

// TODO remove
void NMNetworkInterface::setIpV4Config(const QVariant & ipConfigObjPath)
{
#warning TODO NMNetworkInterface::setIpV4Config demarshal QVariant here
    Q_D(NMNetworkInterface);
}


bool NMNetworkInterface::isActive() const
{
    Q_D(const NMNetworkInterface);
    return !(d->connectionState == Solid::Control::NetworkInterface::Unavailable
            || d->connectionState == Solid::Control::NetworkInterface::Disconnected
            || d->connectionState == Solid::Control::NetworkInterface::Failed );
}

bool NMNetworkInterface::managed() const
{
    Q_D(const NMNetworkInterface);
    return d->managed;
}

void NMNetworkInterface::setManaged(const QVariant & driver)
{
    Q_D(NMNetworkInterface);
    d->driver = driver.toBool();
}

Solid::Control::NetworkInterface::ConnectionState NMNetworkInterface::connectionState() const
{
    Q_D(const NMNetworkInterface);
    return d->connectionState;
}

void NMNetworkInterface::setConnectionState(const QVariant & state)
{
    Q_D(NMNetworkInterface);
    d->connectionState = convertState(state.toUInt());
}

int NMNetworkInterface::designSpeed() const
{
    Q_D(const NMNetworkInterface);
    return d->designSpeed;
}
/*
bool NMNetworkInterface::isLinkUp() const
{
    Q_D(const NMNetworkInterface);
    return d->isLinkUp;
}
*/
Solid::Control::NetworkInterface::Capabilities NMNetworkInterface::capabilities() const
{
    Q_D(const NMNetworkInterface);
    return d->capabilities;
}

QVariant NMNetworkInterface::capabilitiesV() const
{
    Q_D(const NMNetworkInterface);
    return QVariant(d->capabilities);
}

void NMNetworkInterface::setCapabilitiesV(const QVariant & caps)
{
    Q_D(NMNetworkInterface);
    d->capabilities = convertCapabilities(caps.toUInt());
}

QStringList NMNetworkInterface::activeConnections() const
{
    Q_D(const NMNetworkInterface);
    return d->activeConnections;
}

Solid::Control::NetworkInterface::Capabilities NMNetworkInterface::convertCapabilities(uint theirCaps)
{
    Solid::Control::NetworkInterface::Capabilities ourCaps
        = (Solid::Control::NetworkInterface::Capabilities) theirCaps;
    return ourCaps;
}

Solid::Control::NetworkInterface::ConnectionState NMNetworkInterface::convertState(uint theirState)
{
    Solid::Control::NetworkInterface::ConnectionState ourState = (Solid::Control::NetworkInterface::ConnectionState)theirState;
    return ourState;
}

void NMNetworkInterface::stateChanged(uint state)
{
    Q_D(NMNetworkInterface);
    d->connectionState = convertState(state);
}
#include "networkinterface.moc"

