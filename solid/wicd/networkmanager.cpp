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

    QDBusInterface manager;
    QDBusInterface wireless;
    Wicd::ConnectionStatus cachedState;
    QHash<QString, WicdNetworkInterface *> interfaces;
};

WicdNetworkManagerPrivate::WicdNetworkManagerPrivate()
 : manager(WICD_DBUS_SERVICE, WICD_DBUS_PATH, WICD_DAEMON_DBUS_INTERFACE, QDBusConnection::systemBus())
 , wireless(WICD_DBUS_SERVICE, WICD_DBUS_PATH, WICD_WIRELESS_DBUS_INTERFACE, QDBusConnection::systemBus())
 , cachedState(Wicd::Unknown)
{
}


WicdNetworkManager::WicdNetworkManager(QObject * parent, const QVariantList  & /*args */)
 : NetworkManager(parent), d(new WicdNetworkManagerPrivate())
{

}

WicdNetworkManager::~WicdNetworkManager()
{
    delete d;
}

Solid::Networking::Status WicdNetworkManager::status() const
{
    if (d->cachedState == Wicd::Unknown)
    {
        QDBusReply< uint > state = d->manager.call("GetConnectionStatus");
        if (state.isValid())
        {
            kDebug(1441) << "  got state: " << state.value();
            d->cachedState = static_cast<Wicd::ConnectionStatus>(state.value());
        }
    }
    switch ( d->cachedState ) {
        case Wicd::CONNECTING:
            return Solid::Networking::Connecting;
            break;
        case Wicd::WIRED:
        case Wicd::WIRELESS:
            return Solid::Networking::Connected;
            break;
        case Wicd::NOT_CONNECTED:
            return Solid::Networking::Unconnected;
            break;
        default:
        case Wicd::SUSPENDED:
            return Solid::Networking::Unknown;
            break;
    }
}

QStringList WicdNetworkManager::networkInterfaces() const
{
    // Let's parse ifconfig here

    QProcess ifconfig;

    ifconfig.start(QString("ifconfig"));
    ifconfig.waitForFinished();

    QString result = ifconfig.readAllStandardOutput();

    QStringList lines = result.split('\n');

    QStringList ifaces;

    bool enterIface = true;

    foreach (const QString &line, lines) {
        if (enterIface) {
            ifaces.append(line.split(' ').at(0));
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
    WicdNetworkInterface * netInterface = 0;
    QHash<QString, WicdNetworkInterface *>::Iterator it = d->interfaces.find(uni);
    if (it == d->interfaces.end())
    {
        kDebug() << "unknown interface:" << uni;
        return 0;
    }
    if (it.value())
    {
        netInterface = it.value();
    }
    else
    {
        QDBusInterface wired_iface(WICD_DBUS_SERVICE,
                WICD_DBUS_PATH,
                WICD_WIRED_DBUS_INTERFACE,
                QDBusConnection::systemBus());
        QDBusInterface wireless_iface(WICD_DBUS_SERVICE,
                WICD_DBUS_PATH,
                WICD_WIRELESS_DBUS_INTERFACE,
                QDBusConnection::systemBus());
        QDBusReply<QStringList> wired_reply = wired_iface.call("DetectWiredInterface");
        QDBusReply<QStringList> wireless_reply = wireless_iface.call("DetectWirelessInterface");
        if (!wired_reply.isValid() && !wireless_reply.isValid())
        {
            kDebug(1441) << "Invalid reply, most probably the specified device does not exists.";
            return 0;
        }

        if (wired_reply.value().contains(uni)) {
            netInterface = new WicdWiredNetworkInterface(uni);
        } else if (wireless_reply.value().contains(uni)) {
            netInterface = new WicdWirelessNetworkInterface(uni);
        }

        if (netInterface)
        {
            //netInterface->setManagerInterface(&d->manager);
            it.value() = netInterface;
        }
    }
    return netInterface;
}

bool WicdNetworkManager::isNetworkingEnabled() const
{
    if (d->cachedState == Wicd::Unknown)
    {
        QDBusReply< uint > state = d->manager.call("GetConnectionStatus");
        if (state.isValid())
        {
            kDebug(1441) << "  got state: " << state.value();
            d->cachedState = static_cast<Wicd::ConnectionStatus>(state.value());
        }
    }

    return Wicd::CONNECTING == d->cachedState || Wicd::WIRED == d->cachedState ||
           Wicd::WIRELESS == d->cachedState || Wicd::NOT_CONNECTED == d->cachedState;
}

bool WicdNetworkManager::isWirelessEnabled() const
{
    QDBusReply< bool > state = d->wireless.call("IsWirelessUp");
    if (state.isValid()) {
        return state.value();
    }

    return false;
}

bool WicdNetworkManager::isWirelessHardwareEnabled() const
{
    QDBusReply< bool > state = d->manager.call("GetKillSwitchEnabled");

    if (state.isValid()) {
        return !state.value();
    }

    return false;
}

QStringList WicdNetworkManager::activeConnections() const
{
    QStringList activeConnections;
    QHash<QString, WicdNetworkInterface *>::ConstIterator it = d->interfaces.constBegin(), itEnd = d->interfaces.constEnd();
    for ( ; it != itEnd; ++it)
    {
        WicdNetworkInterface * interface = it.value();
        if (interface && interface->isActive())
        {
            activeConnections << it.key();
        }
    }
    return activeConnections;
}

void WicdNetworkManager::activateConnection(const QString & interfaceUni, const QString & connectionUni,
        const QVariantMap & connectionParameters)
{
    kDebug(1441) << interfaceUni << connectionUni << connectionParameters;
    QHash<QString, WicdNetworkInterface *>::ConstIterator it = d->interfaces.find(interfaceUni);
    if (it != d->interfaces.end())
    {
        WicdNetworkInterface * interface = it.value();
        if (!interface)
            interface = qobject_cast<WicdNetworkInterface *>(createNetworkInterface(interfaceUni));
            if (interface)
            {
                bool activated = interface->activateConnection(connectionUni, connectionParameters);
                Q_UNUSED(activated)
            }
    }
}

void WicdNetworkManager::deactivateConnection(const QString & activeConnection)
{
    kDebug(1441) << activeConnection;
    QHash<QString, WicdNetworkInterface *>::ConstIterator it = d->interfaces.find(activeConnection);
    if (it != d->interfaces.end() && it.value())
    {
        WicdNetworkInterface * interface = it.value();
        bool deactivated = interface->deactivateConnection();
        Q_UNUSED(deactivated)
    }
}

void WicdNetworkManager::setNetworkingEnabled(bool enabled)
{
    //TODO
}

void WicdNetworkManager::setWirelessEnabled(bool enabled)
{
    //TODO
}

#include "networkmanager.moc"
