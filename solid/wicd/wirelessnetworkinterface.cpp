/*  This file is part of the KDE project
    Copyright (C) 2008 Dario Freddi <drf54321@gmail.com>

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

#include "wirelessnetworkinterface.h"

#include "wirelessaccesspoint.h"

#include "wicddbusinterface.h"
#include "wicd-defines.h"

#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>
#include <QtCore/QProcess>

#include <KDebug>

class WicdWirelessNetworkInterface::Private
{
public:
    Private() {};

    QMap<int, QString> getAccessPointsWithId();

    Solid::Control::WirelessNetworkInterface::OperationMode parseOpMode(const QString &m);

    bool isActiveInterface;
    QString uni;
    int bitrate;
    int current_network;
    QString driver;
    QString mode;
    QString auth_methods;
    Solid::Control::NetworkInterface::ConnectionState connection_state;
};

Solid::Control::WirelessNetworkInterface::OperationMode WicdWirelessNetworkInterface::Private::parseOpMode(const QString &m)
{
    if (m == "Master") {
        return Solid::Control::WirelessNetworkInterface::Master;
    } else if (m == "Managed") {
        return Solid::Control::WirelessNetworkInterface::Managed;
    } else if (m == "Adhoc") {
        return Solid::Control::WirelessNetworkInterface::Adhoc;
    }

    return Solid::Control::WirelessNetworkInterface::Master;
}

void WicdWirelessNetworkInterface::recacheInformation()
{
    QProcess process;
    process.start("iwconfig " + d->uni);
    process.waitForFinished();
    QString iwconfig = process.readAll();
    process.close();
    process.start("iwlist " + d->uni + " auth");
    process.waitForFinished();
    QString iwlist = process.readAll();

    QDBusReply< QString > bitrater = WicdDbusInterface::instance()->wireless().call("GetCurrentBitrate", iwconfig);
    QDBusReply< QString > authmr = WicdDbusInterface::instance()->wireless().call("GetAvailableAuthMethods", iwlist);
    QDBusReply< QString > moder = WicdDbusInterface::instance()->wireless().call("GetOperationalMode", iwconfig);
    QDBusReply< int > networkr = WicdDbusInterface::instance()->wireless().call("GetCurrentNetworkID");
    QDBusReply< QString > driverr = WicdDbusInterface::instance()->daemon().call("GetWPADriver");
    QDBusReply< QString > interfacer = WicdDbusInterface::instance()->daemon().call("GetWirelessInterface");
    QDBusReply< QString > cstater = WicdDbusInterface::instance()->wireless().call("CheckWirelessConnectingMessage");

    if (d->bitrate != bitrater.value().split(' ').at(0).toInt() * 1000) {
        d->bitrate = bitrater.value().split(' ').at(0).toInt() * 1000;
        emit bitRateChanged(d->bitrate);
    }
    d->driver = driverr.value();

    if (d->mode != moder.value()) {
        d->mode = moder.value();
        emit modeChanged(d->parseOpMode(d->mode));
    }

    d->auth_methods = authmr.value();

    if (interfacer.value() == d->uni) {
        kDebug() << "Active interface";
        if (d->current_network != networkr.value()) {
            d->current_network = networkr.value();
            emit activeAccessPointChanged(d->getAccessPointsWithId()[d->current_network]);
        }

        Solid::Control::NetworkInterface::ConnectionState connection_state;

        if (cstater.value() == "configuring_interface") {
            connection_state = Solid::Control::NetworkInterface::Configuring;
        } else if (cstater.value() == "validating_authentication") {
            connection_state = Solid::Control::NetworkInterface::NeedAuth;
        } else if (cstater.value() == "done") {
            connection_state = Solid::Control::NetworkInterface::Activated;
        } else if (cstater.value() == "interface_down") {
            connection_state = Solid::Control::NetworkInterface::Disconnected;
        } else if (cstater.value() == "running_dhcp" ||
                   cstater.value() == "setting_static_ip" ||
                   cstater.value() == "setting_broadcast_address") {
            connection_state = Solid::Control::NetworkInterface::IPConfig;
        } else if (cstater.value() == "interface_up") {
            connection_state = Solid::Control::NetworkInterface::Preparing;
        } else {
            connection_state = Solid::Control::NetworkInterface::UnknownState;
        }

        if (connection_state != d->connection_state) {
            connection_state = d->connection_state;
            emit connectionStateChanged(d->connection_state);
        }
    }
}

QMap<int, QString> WicdWirelessNetworkInterface::Private::getAccessPointsWithId()
{
    QDBusReply< int > networks = WicdDbusInterface::instance()->wireless().call("GetNumberOfNetworks");
    QMap<int, QString> retlist;

    for (int i = 0; i < networks.value(); ++i) {
        QDBusReply< QString > r = WicdDbusInterface::instance()->wireless().call("GetWirelessProperty", i, "bssid");
        retlist[i] = r;
    }

    return retlist;
}

WicdWirelessNetworkInterface::WicdWirelessNetworkInterface(const QString &objectPath)
        : WicdNetworkInterface(objectPath)
        , d(new Private())
{
    d->uni = uni();
    recacheInformation();
    QDBusConnection::systemBus().connect(WICD_DBUS_SERVICE, WICD_DAEMON_DBUS_PATH, WICD_DAEMON_DBUS_INTERFACE,
                                         "StatusChanged", this, SLOT(refreshStatus()));
}

WicdWirelessNetworkInterface::~WicdWirelessNetworkInterface()
{
    delete d;
}

Solid::Control::NetworkInterface::Type WicdWirelessNetworkInterface::type() const
{
    return Solid::Control::NetworkInterface::Ieee80211;
}

bool WicdWirelessNetworkInterface::isActive() const
{
    return d->isActiveInterface;
}

Solid::Control::NetworkInterface::Capabilities WicdWirelessNetworkInterface::capabilities() const
{
    Solid::Control::NetworkInterface::Capabilities cap;

    if (interfaceName() != "lo" || !interfaceName().contains("wmaster")) {
        cap |= Solid::Control::NetworkInterface::IsManageable;
    }

    return cap;
}

QString WicdWirelessNetworkInterface::driver() const
{
    return d->driver;
}

int WicdWirelessNetworkInterface::bitRate() const
{
    return d->bitrate;
}

Solid::Control::NetworkInterface::ConnectionState WicdWirelessNetworkInterface::connectionState() const
{
    if (d->isActiveInterface) {
        return d->connection_state;
    } else {
        return Solid::Control::NetworkInterface::Unavailable;
    }
}

Solid::Control::WirelessNetworkInterface::Capabilities WicdWirelessNetworkInterface::wirelessCapabilities() const
{
    Solid::Control::WirelessNetworkInterface::Capabilities cap;

    if (d->auth_methods.contains("WPA")) {
        cap |= Solid::Control::WirelessNetworkInterface::Wpa;
    }
    if (d->auth_methods.contains("CIPHER-TKIP")) {
        cap |= Solid::Control::WirelessNetworkInterface::Tkip;
    }
    if (d->auth_methods.contains("CIPHER-CCMP")) {
        cap |= Solid::Control::WirelessNetworkInterface::Ccmp;
    }

    cap |= Solid::Control::WirelessNetworkInterface::Wep104;
    cap |= Solid::Control::WirelessNetworkInterface::Wep40;

    return cap;
}

Solid::Control::WirelessNetworkInterface::OperationMode WicdWirelessNetworkInterface::mode() const
{
    return d->parseOpMode(d->mode);
}

MacAddressList WicdWirelessNetworkInterface::accessPoints() const
{
    return d->getAccessPointsWithId().values();
}

QString WicdWirelessNetworkInterface::activeAccessPoint() const
{
    if (d->isActiveInterface) {
        return d->getAccessPointsWithId()[d->current_network];
    } else {
        return QString();
    }
}

QString WicdWirelessNetworkInterface::hardwareAddress() const
{
    // Let's parse ifconfig here

    QProcess ifconfig;

    ifconfig.start(QString("ifconfig %1").arg(uni()));
    ifconfig.waitForFinished();

    QString result = ifconfig.readAllStandardOutput();

    QStringList lines = result.split('\n');

    return lines.at(0).split("HWaddr ").at(1);
}

QObject * WicdWirelessNetworkInterface::createAccessPoint(const QString & uni)
{
    QMap<int, QString> aps = d->getAccessPointsWithId();

    if (!aps.values().contains(uni)) {
        kDebug() << "Requested a non existent AP";
    }

    int network = aps.key(uni);

    return new WicdAccessPoint(network);
}

#include "wirelessnetworkinterface.moc"
