/*  This file is part of the KDE project
    Copyright (C) 2006 Will Stephenson <wstephenson@kde.org>
    Copyright (C) 2007 Kevin Ottens <ervin@kde.org>

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

#include <QMap>
#include <QStringList>

#include <solid/control/ifaces/network.h>
#include <solid/control/ifaces/networkinterface.h>
#include <solid/control/ifaces/wirelessnetwork.h>

#include "frontendobject_p.h"

#include "soliddefs_p.h"
#include "networkmanager.h"
#include "network.h"
#include "network_p.h"
#include "networkinterface.h"
#include "wirelessnetwork.h"

namespace Solid
{
namespace Control
{
    class NetworkInterfacePrivate : public FrontendObjectPrivate
    {
    public:
        NetworkInterfacePrivate(QObject *parent)
            : FrontendObjectPrivate(parent) { }

        void setBackendObject(QObject *object);
        void _k_destroyed(QObject *object);

        Solid::Control::Network *findRegisteredNetwork(const QString &uni) const;


        mutable QMap<QString, Network *> networkMap;
        mutable Network invalidNetwork;
    };
}
}


Solid::Control::NetworkInterface::NetworkInterface()
    : QObject(), d(new NetworkInterfacePrivate(this))
{
}

Solid::Control::NetworkInterface::NetworkInterface(const QString &uni)
    : QObject(), d(new NetworkInterfacePrivate(this))
{
    const NetworkInterface &device = NetworkManager::findNetworkInterface(uni);
    d->setBackendObject(device.d->backendObject());
}

Solid::Control::NetworkInterface::NetworkInterface(QObject *backendObject)
    : QObject(), d(new NetworkInterfacePrivate(this))
{
    d->setBackendObject(backendObject);
}

Solid::Control::NetworkInterface::NetworkInterface(const NetworkInterface &device)
    : QObject(), d(new NetworkInterfacePrivate(this))
{
    d->setBackendObject(device.d->backendObject());
}

Solid::Control::NetworkInterface::~NetworkInterface()
{
    foreach (QObject *network, d->networkMap)
    {
        delete network;
    }
}

Solid::Control::NetworkInterface &Solid::Control::NetworkInterface::operator=(const Solid::Control::NetworkInterface  & dev)
{
    d->setBackendObject(dev.d->backendObject());
    return *this;
}

bool Solid::Control::NetworkInterface::isValid() const
{
    return d->backendObject()!=0;
}

QString Solid::Control::NetworkInterface::uni() const
{
    return_SOLID_CALL(Ifaces::NetworkInterface *, d->backendObject(), QString(), uni());
}

bool Solid::Control::NetworkInterface::isActive() const
{
    return_SOLID_CALL(Ifaces::NetworkInterface *, d->backendObject(), false, isActive());
}

Solid::Control::NetworkInterface::Type Solid::Control::NetworkInterface::type() const
{
    return_SOLID_CALL(Ifaces::NetworkInterface *, d->backendObject(), UnknownType, type());
}
Solid::Control::NetworkInterface::ConnectionState Solid::Control::NetworkInterface::connectionState() const
{
    return_SOLID_CALL(Ifaces::NetworkInterface *, d->backendObject(), UnknownState, connectionState());
}

int Solid::Control::NetworkInterface::signalStrength() const
{
    return_SOLID_CALL(Ifaces::NetworkInterface *, d->backendObject(), 0, signalStrength());
}

int Solid::Control::NetworkInterface::designSpeed() const
{
    return_SOLID_CALL(Ifaces::NetworkInterface *, d->backendObject(), 0, designSpeed());
}

bool Solid::Control::NetworkInterface::isLinkUp() const
{
    return_SOLID_CALL(Ifaces::NetworkInterface *, d->backendObject(), false, isLinkUp());
}

Solid::Control::NetworkInterface::Capabilities Solid::Control::NetworkInterface::capabilities() const
{
    return_SOLID_CALL(Ifaces::NetworkInterface *, d->backendObject(), Capabilities(), capabilities());
}

Solid::Control::Network * Solid::Control::NetworkInterface::findNetwork(const QString  & uni) const
{
    if (!isValid()) return 0;

    Network *network = d->findRegisteredNetwork(uni);

    if (network!=0)
    {
        return network;
    }
    else
    {
        return &(d->invalidNetwork);
    }
}

Solid::Control::NetworkList Solid::Control::NetworkInterface::networks() const
{
    NetworkList list;
    Ifaces::NetworkInterface *device = qobject_cast<Ifaces::NetworkInterface *>(d->backendObject());

    if (device==0) return list;

    QStringList uniList = device->networks();

    foreach (const QString &uni, uniList)
    {
        Network *network = d->findRegisteredNetwork(uni);
        if (network!=0)
        {
            list.append(network);
        }
    }

    return list;
}

QString Solid::Control::NetworkInterface::activeNetwork() const
{
    return_SOLID_CALL(Ifaces::NetworkInterface *, d->backendObject(), QString(), activeNetwork());
}

void Solid::Control::NetworkInterfacePrivate::_k_destroyed(QObject *object)
{
    if (object == backendObject())
    {
        FrontendObjectPrivate::_k_destroyed(object);

        foreach (Network *network, networkMap)
        {
            delete network->d_func()->backendObject();
            delete network;
        }

        networkMap.clear();
    }
}

void Solid::Control::NetworkInterfacePrivate::setBackendObject(QObject *object)
{
    foreach (QObject *network, networkMap) {
        delete network;
    }

    networkMap.clear();


    FrontendObjectPrivate::setBackendObject(object);

    if (object) {
        QObject::connect(object, SIGNAL(activeChanged(bool)),
                         parent(), SIGNAL(activeChanged(bool)));
        QObject::connect(object, SIGNAL(linkUpChanged(bool)),
                         parent(), SIGNAL(linkUpChanged(bool)));
        QObject::connect(object, SIGNAL(signalStrengthChanged(int)),
                         parent(), SIGNAL(signalStrengthChanged(int)));
        QObject::connect(object, SIGNAL(connectionStateChanged(int)),
                         parent(), SIGNAL(connectionStateChanged(int)));
        QObject::connect(object, SIGNAL(networkAppeared(const QString &)),
                         parent(), SIGNAL(networkAppeared(const QString &)));
        QObject::connect(object, SIGNAL(networkDisappeared(const QString &)),
                         parent(), SIGNAL(networkDisappeared(const QString &)));
    }
}

Solid::Control::Network *Solid::Control::NetworkInterfacePrivate::findRegisteredNetwork(const QString &uni) const
{
    Network *network = 0;

    if (networkMap.contains(uni)) {
        network = networkMap[uni];
    } else {
        Ifaces::NetworkInterface *device = qobject_cast<Ifaces::NetworkInterface *>(backendObject());

        if (device!=0)
        {
            QObject *iface = device->createNetwork(uni);

            if (qobject_cast<Ifaces::WirelessNetwork *>(iface)!=0)
            {
                network = new WirelessNetwork(iface);
            }
            else if (qobject_cast<Ifaces::Network *>(iface)!=0)
            {
                network = new Network(iface);
            }

            if (network != 0)
            {
                networkMap[uni] = network;
            }
        }
    }

    return network;
}

#include "networkinterface.moc"
