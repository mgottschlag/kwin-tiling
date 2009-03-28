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

    Wicd::ConnectionStatus cachedState;
    QHash<QString, WicdNetworkInterface *> interfaces;
};

WicdNetworkManagerPrivate::WicdNetworkManagerPrivate()
        : cachedState(Wicd::Unknown)
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
    if (d->cachedState == Wicd::Unknown) {
        QDBusReply< uint > state = WicdDbusInterface::instance()->daemon().call("GetConnectionStatus");
        if (state.isValid()) {
            kDebug(1441) << "  got state: " << state.value();
            d->cachedState = static_cast<Wicd::ConnectionStatus>(state.value());
        } else {
          kDebug() << "Invalid reply from DBus";
        }
    }
    switch (d->cachedState) {
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

    foreach(const QString &line, lines) {
        if (enterIface) {
            if (!line.split(' ').at(0).isEmpty()) {
                ifaces.append(line.split(' ').at(0));
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
    if (d->cachedState == Wicd::Unknown) {
        kDebug() << "First run";
        QDBusReply< uint > state = WicdDbusInterface::instance()->daemon().call("GetConnectionStatus");
        if (state.isValid()) {
            kDebug(1441) << "  got state: " << state.value();
            d->cachedState = static_cast<Wicd::ConnectionStatus>(state.value());
        } else {
            kDebug() << "Invalid reply!!";
        }
    }

    return Wicd::CONNECTING == d->cachedState || Wicd::WIRED == d->cachedState ||
           Wicd::WIRELESS == d->cachedState || Wicd::NOT_CONNECTED == d->cachedState;
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

void WicdNetworkManager::activateConnection(const QString & interfaceUni, const QString & connectionUni,
        const QVariantMap & connectionParameters)
{
    kDebug(1441) << interfaceUni << connectionUni << connectionParameters;
    QHash<QString, WicdNetworkInterface *>::ConstIterator it = d->interfaces.find(interfaceUni);
    if (it != d->interfaces.end()) {
        WicdNetworkInterface * interface = it.value();
        if (!interface)
            interface = qobject_cast<WicdNetworkInterface *>(createNetworkInterface(interfaceUni));
        if (interface) {
            bool activated = interface->activateConnection(connectionUni, connectionParameters);
            Q_UNUSED(activated)
        }
    }
}

void WicdNetworkManager::deactivateConnection(const QString & activeConnection)
{
    kDebug(1441) << activeConnection;
    QHash<QString, WicdNetworkInterface *>::ConstIterator it = d->interfaces.find(activeConnection);
    if (it != d->interfaces.end() && it.value()) {
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
