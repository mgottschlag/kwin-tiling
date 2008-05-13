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

#include "accesspoint.h"

#include <KDebug>
#include "dbus/nm-access-pointinterface.h"
#include "manager.h"
#include "wirelessnetworkinterface.h"


class NMAccessPoint::Private
{
public:
    Private( const QString & path ) : iface( NMNetworkManager::DBUS_SERVICE, path, QDBusConnection::systemBus()), capabilities(0), wpaFlags(0), rsnFlags(0), frequency(0), hardwareAddress(0), maxBitRate(0), mode((Solid::Control::WirelessNetworkInterface::OperationMode)0), signalStrength(0)
    {
    }
    OrgFreedesktopNetworkManagerAccessPointInterface iface;
    QString uni;
    Solid::Control::AccessPoint::Capabilities capabilities;
    Solid::Control::AccessPoint::WpaFlags wpaFlags;
    Solid::Control::AccessPoint::WpaFlags rsnFlags;
    QString ssid;
    uint frequency;
    QString hardwareAddress;
    uint maxBitRate;
    Solid::Control::WirelessNetworkInterface::OperationMode mode;
    int signalStrength;
};

NMAccessPoint::NMAccessPoint( const QString& path, QObject * parent ) : Solid::Control::Ifaces::AccessPoint(parent), d(new Private( path ))
{
    d->uni = path;
    d->capabilities = convertCapabilities( d->iface.flags() );
    d->wpaFlags = convertWpaFlags( d->iface.wpaFlags() );
    d->rsnFlags = convertWpaFlags( d->iface.rsnFlags() );
    d->signalStrength = d->iface.strength();
    d->ssid = d->iface.ssid();
    d->frequency = d->iface.frequency();
    d->hardwareAddress = d->iface.hwAddress();
    d->maxBitRate = d->iface.maxBitrate();
    // make this a static on WirelessNetworkInterface
    d->mode = NMWirelessNetworkInterface::convertOperationMode(d->iface.mode());
}

NMAccessPoint::~NMAccessPoint()
{
    delete d;
}

QString NMAccessPoint::uni() const
{
    return d->uni;
}

QString NMAccessPoint::hardwareAddress() const
{
    return d->hardwareAddress;
}

Solid::Control::AccessPoint::Capabilities NMAccessPoint::capabilities() const
{
    return d->capabilities;
}

Solid::Control::AccessPoint::WpaFlags NMAccessPoint::wpaFlags() const
{
    return d->wpaFlags;
}

Solid::Control::AccessPoint::WpaFlags NMAccessPoint::rsnFlags() const
{
    return d->rsnFlags;
}

QString NMAccessPoint::ssid() const
{
    return d->ssid;
}

uint NMAccessPoint::frequency() const
{
    return d->frequency;
}

uint NMAccessPoint::maxBitRate() const
{
    return d->maxBitRate;
}

Solid::Control::WirelessNetworkInterface::OperationMode NMAccessPoint::mode() const
{
    return d->mode;
}

int NMAccessPoint::signalStrength() const
{
    return d->signalStrength;
}

void NMAccessPoint::propertiesChanged(const QVariantMap &properties)
{
#warning TODO NMAccessPoint::propertiesChanged() implement
    kDebug() << properties.keys();
}

Solid::Control::AccessPoint::Capabilities NMAccessPoint::convertCapabilities(int caps)
{
    if ( 1 == caps ) {
        return Solid::Control::AccessPoint::Privacy;
    } else {
        return 0;
    }
}
// Copied from wireless.h
// /* Modes of operation */
#define IW_MODE_AUTO    0   /* Let the driver decides */
#define IW_MODE_ADHOC   1   /* Single cell network */
#define IW_MODE_INFRA   2   /* Multi cell network, roaming, ... */
#define IW_MODE_MASTER  3   /* Synchronization master or Access Point */
#define IW_MODE_REPEAT  4   /* Wireless Repeater (forwarder) */
#define IW_MODE_SECOND  5   /* Secondary master/repeater (backup) */
#define IW_MODE_MONITOR 6   /* Passive monitor (listen only) */

Solid::Control::AccessPoint::WpaFlags NMAccessPoint::convertWpaFlags(uint theirFlags)
{
    return (Solid::Control::AccessPoint::WpaFlags)theirFlags;
}

#include "accesspoint.moc"

