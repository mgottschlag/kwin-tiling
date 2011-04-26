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

#include "wirednetworkinterface.h"

#include "wicddbusinterface.h"
#include "wicd-defines.h"

#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>
#include <QtCore/QProcess>

#include <KDebug>

class WicdWiredNetworkInterface::Private
{
public:

    bool isActiveInterface;
    QString uni;
    int bitrate;
    QString driver;
    bool carrier;
    Solid::Control::NetworkInterface::ConnectionState connection_state;
};

WicdWiredNetworkInterface::WicdWiredNetworkInterface(const QString &name)
        : WicdNetworkInterface(name)
        , d(new Private())
{
    d->uni = uni();
    recacheInformation();
    QDBusConnection::systemBus().connect(WICD_DBUS_SERVICE, WICD_DAEMON_DBUS_PATH, WICD_DAEMON_DBUS_INTERFACE,
                                         "StatusChanged", this, SLOT(refreshStatus()));
}

WicdWiredNetworkInterface::~WicdWiredNetworkInterface()
{
    delete d;
}

void WicdWiredNetworkInterface::recacheInformation()
{
    //QDBusReply< QString > bitrater = WicdDbusInterface::instance()->wireless().call("GetCurrentBitrate", iwconfig);
    QDBusReply< QString > interfacer = WicdDbusInterface::instance()->daemon().call("GetWiredInterface");
    QDBusReply< bool > carrierr = WicdDbusInterface::instance()->wired().call("CheckPluggedIn");
    QDBusReply< QString > cstater = WicdDbusInterface::instance()->wired().call("CheckWiredConnectingMessage");

    if (interfacer.value() == d->uni) {
        kDebug() << "Active interface";

        if (d->carrier != carrierr.value()) {
            d->carrier = carrierr.value();
            emit carrierChanged(d->carrier);
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
            int old_state = d->connection_state;
            connection_state = d->connection_state;
            emit connectionStateChanged(d->connection_state, old_state, Solid::Control::NetworkInterface::NoReason);
        }
    } else {
        if (d->carrier != false) {
            d->carrier = false;
            emit carrierChanged(d->carrier);
        }
    }
}

Solid::Control::NetworkInterface::ConnectionState WicdWiredNetworkInterface::connectionState() const
{
    if (d->isActiveInterface) {
        return d->connection_state;
    } else {
        return Solid::Control::NetworkInterface::Unavailable;
    }
}

Solid::Control::NetworkInterface::Type WicdWiredNetworkInterface::type() const
{
    return Solid::Control::NetworkInterface::Ieee8023;
}

bool WicdWiredNetworkInterface::isActive() const
{
    return d->isActiveInterface;
}

Solid::Control::NetworkInterface::Capabilities WicdWiredNetworkInterface::capabilities() const
{
    Solid::Control::NetworkInterface::Capabilities cap;

    if (interfaceName() != "lo" || !interfaceName().contains("wmaster")) {
        cap |= Solid::Control::NetworkInterface::IsManageable;
    }

    cap |= Solid::Control::NetworkInterface::SupportsCarrierDetect;

    return cap;
}

QString WicdWiredNetworkInterface::driver() const
{
    return QString();
}

QString WicdWiredNetworkInterface::hardwareAddress() const
{
    // Let's parse ifconfig here

    QProcess ifconfig;
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    qputenv("PATH", QString("/bin:/usr/bin:/sbin:/usr/sbin:" + env.value("PATH")).toAscii());
    qputenv("LC_ALL", "C");
    ifconfig.start(QString("ifconfig %1").arg(uni()));

    // ifconfig is not installed or not found in PATH.
    if (!ifconfig.waitForStarted()) {
        return QString();
    }

    // ifconfig returned a error.
    if (!ifconfig.waitForFinished()) {
        return QString();
    }

    QString result = ifconfig.readAllStandardOutput();
    QStringList lines = result.split('\n');

    // ifconfig returned something.
    if (lines.count() > 0) {
        QStringList result = lines.at(0).split("HWaddr ");

        // MAC address not found.
        if (result.count() > 1)
            return result.at(1);
    }
    
    return QString();
}

int WicdWiredNetworkInterface::bitRate() const
{
    return d->bitrate;
}

bool WicdWiredNetworkInterface::carrier() const
{
    return d->carrier;
}

bool WicdWiredNetworkInterface::activateConnection(const QString & connectionUni, const QVariantMap & connectionParameters)
{
    Q_UNUSED(connectionUni)
    Q_UNUSED(connectionParameters)
    WicdDbusInterface::instance()->daemon().call("SetWiredInterface", interfaceName());
    WicdDbusInterface::instance()->wired().call("ConnectWired");
    return true;
}

bool WicdWiredNetworkInterface::deactivateConnection()
{
    WicdDbusInterface::instance()->wired().call("DisconnectWired");
    return true;
}

#include "wirednetworkinterface.moc"
