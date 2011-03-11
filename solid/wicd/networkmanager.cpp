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

#include "networkmanager.h"

#include "wicd-defines.h"
#include "wicdcustomtypes.h"
#include "wicddbusinterface.h"
#include "wirednetworkinterface.h"
#include "wirelessnetworkinterface.h"

#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusConnectionInterface>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusMetaType>
#include <QtDBus/QDBusReply>

#include <QProcess>

#include <kdebug.h>

class WicdNetworkManagerPrivate
{
public:
    WicdNetworkManagerPrivate();

    bool recacheState();

    Solid::Networking::Status cachedState;
    QHash<QString, WicdNetworkInterface *> interfaces;
};

WicdNetworkManagerPrivate::WicdNetworkManagerPrivate()
        : cachedState(Solid::Networking::Unknown)
{
    recacheState();
}

bool WicdNetworkManagerPrivate::recacheState()
{
    QDBusMessage message = WicdDbusInterface::instance()->daemon().call("GetConnectionStatus");

    if (message.arguments().count() == 0) {
        cachedState = Solid::Networking::Unknown;
        return false;
    }

    if (!message.arguments().at(0).isValid()) {
        cachedState = Solid::Networking::Unknown;
        return false;
    }

    WicdConnectionInfo s;
    message.arguments().at(0).value<QDBusArgument>() >> s;
    kDebug() << "State: " << s.status << " Info: " << s.info;
    Solid::Networking::Status state;

    switch (static_cast<Wicd::ConnectionStatus>(s.status)) {
    case Wicd::CONNECTING:
        state = Solid::Networking::Connecting;
        break;
    case Wicd::WIRED:
    case Wicd::WIRELESS:
        state = Solid::Networking::Connected;
        break;
    case Wicd::NOT_CONNECTED:
        state = Solid::Networking::Unconnected;
        break;
    default:
    case Wicd::SUSPENDED:
        state = Solid::Networking::Unknown;
        break;
    }

    if (state != cachedState) {
        cachedState = state;
        return true;
    } else {
        return false;
    }
}


WicdNetworkManager::WicdNetworkManager(QObject * parent, const QVariantList  & /*args */)
        : NetworkManager(parent), d(new WicdNetworkManagerPrivate())
{
    qDBusRegisterMetaType<WicdConnectionInfo>();
    QDBusConnection::systemBus().connect(WICD_DBUS_SERVICE, WICD_DAEMON_DBUS_PATH, WICD_DAEMON_DBUS_INTERFACE,
                                         "StatusChanged", this, SLOT(refreshStatus()));
}

WicdNetworkManager::~WicdNetworkManager()
{
    delete d;
}

void WicdNetworkManager::refreshStatus()
{
    if (d->recacheState()) {
        emit statusChanged(d->cachedState);
    }
}

Solid::Networking::Status WicdNetworkManager::status() const
{
    return d->cachedState;
}

QStringList WicdNetworkManager::networkInterfaces() const
{
    // Let's parse ifconfig here

    QProcess ifconfig;

    ifconfig.setEnvironment(QStringList() << QProcess::systemEnvironment() << "LANG=C");
    ifconfig.start(QString("ifconfig -a"));
    ifconfig.waitForFinished();

    QString result = ifconfig.readAllStandardOutput();

    QStringList lines = result.split('\n');

    QStringList ifaces;

    bool enterIface = true;

    foreach(const QString &line, lines) {
        if (enterIface) {
            if (!line.split(' ').at(0).isEmpty()) {
                if (line.split(' ').at(0) != "lo" && !line.split(' ').at(0).contains("wmaster")) {
                    ifaces.append(line.split(' ').at(0));
                }
            }
            enterIface = false;
        }

        if (line.isEmpty()) {
            enterIface = true;
        }
    }

    return ifaces;
}

QObject * WicdNetworkManager::createNetworkInterface(const QString  & uni)
{
    kDebug(1441) << uni;

    if (!networkInterfaces().contains(uni)) {
        kDebug() << "Interface not present in the available list, returning 0";
        return 0;
    }

    WicdNetworkInterface * netInterface = 0;
    QHash<QString, WicdNetworkInterface *>::Iterator it = d->interfaces.find(uni);
    if (it == d->interfaces.end()) {
        kDebug() << "unknown interface:" << uni << "creating it";
    } else {
        kDebug() << "Interface already created";
        return it.value();
    }

    // Let's parse iwconfig here

    QProcess iwconfig;

    iwconfig.setEnvironment(QStringList() << QProcess::systemEnvironment() << "LANG=C");
    iwconfig.start(QString("iwconfig"));
    iwconfig.waitForFinished();

    QString result = iwconfig.readAllStandardError();

    QStringList lines = result.split('\n');

    QStringList wired;

    foreach(const QString &line, lines) {
        if (!line.isEmpty()) {
            wired << line.split(' ').at(0);
        }
    }

    if (wired.contains(uni)) {
        kDebug() << "Wired interface";
        netInterface = new WicdWiredNetworkInterface(uni);
    } else {
        kDebug() << "Wireless interface";
        netInterface = new WicdWirelessNetworkInterface(uni);
    }

    if (netInterface) {
        kDebug() << "Interface created successfully";
        //netInterface->setManagerInterface(&WicdDbusInterface::instance()->daemon());
        d->interfaces[uni] = netInterface;
    }

    return netInterface;
}

bool WicdNetworkManager::isNetworkingEnabled() const
{
    return Solid::Networking::Connecting == d->cachedState ||
           Solid::Networking::Connected == d->cachedState ||
           Solid::Networking::Unconnected == d->cachedState;
}

bool WicdNetworkManager::isWirelessEnabled() const
{
    QDBusReply< bool > state = WicdDbusInterface::instance()->wireless().call("IsWirelessUp");
    if (state.isValid()) {
        return state.value();
    }

    return false;
}

bool WicdNetworkManager::isWirelessHardwareEnabled() const
{
    QDBusReply< bool > state = WicdDbusInterface::instance()->wireless().call("GetKillSwitchEnabled");

    if (state.isValid()) {
        return !state.value();
    }

    return false;
}

/* Wicd 1.x does not support Wwan (Mobile Broadand). It is schedule for 2.x.
   http://wicd.sourceforge.net/moinmoin/FAQ */
bool WicdNetworkManager::isWwanEnabled() const
{
    return false;
}

/* Wicd 1.x does not support Wwan (Mobile Broadand). It is schedule for 2.x.
   http://wicd.sourceforge.net/moinmoin/FAQ */
void WicdNetworkManager::setWwanEnabled(bool)
{
    return;
}

/* Wicd 1.x does not support Wwan (Mobile Broadand). It is schedule for 2.x.
   http://wicd.sourceforge.net/moinmoin/FAQ */
bool WicdNetworkManager::isWwanHardwareEnabled() const
{
    return false;
}

QStringList WicdNetworkManager::activeConnections() const
{
    QStringList activeConnections;
    QHash<QString, WicdNetworkInterface *>::ConstIterator it = d->interfaces.constBegin(), itEnd = d->interfaces.constEnd();
    for (; it != itEnd; ++it) {
        WicdNetworkInterface * interface = it.value();
        if (interface && interface->isActive()) {
            activeConnections << it.key();
        }
    }
    return activeConnections;
}

Solid::Control::NetworkInterface::Types WicdNetworkManager::supportedInterfaceTypes() const
{
    return (Solid::Control::NetworkInterface::Types) (
           Solid::Control::NetworkInterface::Ieee80211 |
           Solid::Control::NetworkInterface::Ieee8023
           );
}

void WicdNetworkManager::activateConnection(const QString & interfaceUni, const QString & connectionUni,
        const QVariantMap & connectionParameters)
{
    kDebug(1441) << interfaceUni << connectionUni << connectionParameters;
    const QHash<QString, WicdNetworkInterface *>::Iterator it = d->interfaces.find(interfaceUni);
    if (it != d->interfaces.end()) {
        WicdNetworkInterface * interface = it.value();

        if (!interface) {
            interface = qobject_cast<WicdNetworkInterface *>(createNetworkInterface(interfaceUni));
        }

        if (interface) {
            bool activated = interface->activateConnection(connectionUni, connectionParameters);
            Q_UNUSED(activated)
        }
    }
}

void WicdNetworkManager::deactivateConnection(const QString & activeConnection)
{
    kDebug(1441) << activeConnection;
    const QHash<QString, WicdNetworkInterface *>::Iterator it = d->interfaces.find(activeConnection);
    if (it != d->interfaces.end() && it.value()) {
        WicdNetworkInterface * const interface = it.value();
        bool deactivated = interface->deactivateConnection();
        Q_UNUSED(deactivated)
    }
}

void WicdNetworkManager::setNetworkingEnabled(bool enabled)
{
    WicdDbusInterface::instance()->daemon().call("SetSuspend", !enabled);
}

void WicdNetworkManager::setWirelessEnabled(bool enabled)
{
    if (enabled) {
        WicdDbusInterface::instance()->wireless().call("EnableWirelessInterface");
    } else {
        WicdDbusInterface::instance()->wireless().call("DisableWirelessInterface");
    }
}

#include "networkmanager.moc"
