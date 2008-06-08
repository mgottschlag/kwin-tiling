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

// Copied from wireless.h
/* Modes of operation */
#define IW_MODE_AUTO    0   /* Let the driver decides */
#define IW_MODE_ADHOC   1   /* Single cell network */
#define IW_MODE_INFRA   2   /* Multi cell network, roaming, ... */
#define IW_MODE_MASTER  3   /* Synchronization master or Access Point */
#define IW_MODE_REPEAT  4   /* Wireless Repeater (forwarder) */
#define IW_MODE_SECOND  5   /* Secondary master/repeater (backup) */
#define IW_MODE_MONITOR 6   /* Passive monitor (listen only) */

#include "wirelessnetworkinterface.h"
#include "wirelessnetworkinterface_p.h"

#include <KDebug>

#include "accesspoint.h"
#include "manager.h"

NMWirelessNetworkInterfacePrivate::NMWirelessNetworkInterfacePrivate(const QString & path, QObject * owner)
    : NMNetworkInterfacePrivate(path, owner), wirelessIface(NMNetworkManager::DBUS_SERVICE, path, QDBusConnection::systemBus())
      , bitRate(0)
{

}

NMWirelessNetworkInterface::NMWirelessNetworkInterface(const QString & path, NMNetworkManager * manager, QObject * parent)
    : NMNetworkInterface(*new NMWirelessNetworkInterfacePrivate(path, this), manager, parent)
{
    Q_D(NMWirelessNetworkInterface);
    d->hardwareAddress = d->wirelessIface.hwAddress();
    d->mode = convertOperationMode(d->wirelessIface.mode());
    d->bitRate = d->wirelessIface.bitrate();
    d->wirelessCapabilities = convertCapabilities(d->wirelessIface.wirelessCapabilities());

    connect( &d->wirelessIface, SIGNAL(PropertiesChanged(const QVariantMap &)),
                this, SLOT(wirelessPropertiesChanged(const QVariantMap &)));

    qDBusRegisterMetaType<QList<QDBusObjectPath> >();
    QDBusReply< QList <QDBusObjectPath> > apPathList = d->wirelessIface.GetAccessPoints();
    if (apPathList.isValid())
    {
        kDebug(1441) << "Got device list";
        QList <QDBusObjectPath> aps = apPathList.value();
        foreach (QDBusObjectPath op, aps)
        {
            d->accessPoints.append(op.path());
            kDebug(1441) << "  " << op.path();
        }
    }
    else
        kDebug(1441) << "Error getting access point list: " << apPathList.error().name() << ": " << apPathList.error().message();
}

NMWirelessNetworkInterface::~NMWirelessNetworkInterface()
{

}

MacAddressList NMWirelessNetworkInterface::accessPoints() const
{
    Q_D(const NMWirelessNetworkInterface);
    return d->accessPoints;
}

QString NMWirelessNetworkInterface::activeAccessPoint() const
{
    Q_D(const NMWirelessNetworkInterface);
    return d->activeAccessPoint;
}

QString NMWirelessNetworkInterface::hardwareAddress() const
{
    Q_D(const NMWirelessNetworkInterface);
    return d->hardwareAddress;
}

Solid::Control::WirelessNetworkInterface::OperationMode NMWirelessNetworkInterface::mode() const
{
    Q_D(const NMWirelessNetworkInterface);
    return d->mode;
}

int NMWirelessNetworkInterface::bitRate() const
{
    Q_D(const NMWirelessNetworkInterface);
    return d->bitRate;
}

Solid::Control::WirelessNetworkInterface::Capabilities NMWirelessNetworkInterface::wirelessCapabilities() const
{
    Q_D(const NMWirelessNetworkInterface);
    return d->wirelessCapabilities;
}

QObject * NMWirelessNetworkInterface::createAccessPoint(const QString & uni)
{
    return new NMAccessPoint(uni, 0);
}

void NMWirelessNetworkInterface::wirelessPropertiesChanged(const QVariantMap & changedProperties)
{
    kDebug(1441) << changedProperties.keys();
    QStringList propKeys = changedProperties.keys();
    Q_D(NMWirelessNetworkInterface);
    QLatin1String activeApKey("ActiveAccessPoint"), 
                  hwAddrKey("HwAddress"), 
                  bitRateKey("Bitrate"), 
                  modeKey("Mode"), 
                  wirelessCapsKey("WirelessCapabilities");
    QVariantMap::const_iterator it = changedProperties.find(activeApKey);
    if (it != changedProperties.end()) {
        d->activeAccessPoint = qdbus_cast<QDBusObjectPath>(*it).path();
        emit activeAccessPointChanged(d->activeAccessPoint);
        propKeys.removeOne(activeApKey);
    }
    it = changedProperties.find(hwAddrKey);
    if (it != changedProperties.end()) {
        d->hardwareAddress = it->toString();
        propKeys.removeOne(hwAddrKey);
    }
    it = changedProperties.find(bitRateKey);
    if (it != changedProperties.end()) {
        d->bitRate = it->toUInt();
        emit bitRateChanged(d->bitRate);
        propKeys.removeOne(bitRateKey);
    }
    it = changedProperties.find(modeKey);
    if (it != changedProperties.end()) {
        d->mode = convertOperationMode(it->toUInt());
        emit modeChanged(d->mode);
        propKeys.removeOne(modeKey);
    }
    it = changedProperties.find(wirelessCapsKey);
    if (it != changedProperties.end()) {
        d->wirelessCapabilities = convertCapabilities(it->toUInt());
        propKeys.removeOne(wirelessCapsKey);
    }
    if (propKeys.count()) {
        kDebug(1441) << "Unhandled properties: " << propKeys;
    }
}

void NMWirelessNetworkInterface::accessPointAdded(const QDBusObjectPath &apPath)
{
    kDebug(1441) << apPath.path();
    Q_D(NMWirelessNetworkInterface);
    d->accessPoints.append(apPath.path());
    emit accessPointAppeared(apPath.path());
}

void NMWirelessNetworkInterface::accessPointRemoved(const QDBusObjectPath &apPath)
{
    kDebug(1441) << apPath.path();
    Q_D(NMWirelessNetworkInterface);
    if (!d->accessPoints.contains(apPath.path())) {
        kDebug(1441) << "Access point list lookup failed for " << apPath.path();
    }
    d->accessPoints.removeAll(apPath.path());
    emit accessPointDisappeared(apPath.path());
}

Solid::Control::WirelessNetworkInterface::OperationMode NMWirelessNetworkInterface::convertOperationMode(uint theirMode)
{
    Solid::Control::WirelessNetworkInterface::OperationMode ourMode;
    switch ( theirMode ) {
        case IW_MODE_AUTO:
            ourMode = Solid::Control::WirelessNetworkInterface::Managed;
            break;
        case IW_MODE_ADHOC:
            ourMode = Solid::Control::WirelessNetworkInterface::Adhoc;
            break;
        case IW_MODE_INFRA:
        case IW_MODE_MASTER:
            ourMode = Solid::Control::WirelessNetworkInterface::Master;
            break;
        case IW_MODE_REPEAT:
            ourMode = Solid::Control::WirelessNetworkInterface::Repeater;
            break;
        case IW_MODE_SECOND:
        case IW_MODE_MONITOR:
            ourMode = (Solid::Control::WirelessNetworkInterface::OperationMode)0;
            break;
    }
    return ourMode;
}

Solid::Control::WirelessNetworkInterface::Capabilities NMWirelessNetworkInterface::convertCapabilities(uint caps)
{
    return (Solid::Control::WirelessNetworkInterface::Capabilities)caps;
}

