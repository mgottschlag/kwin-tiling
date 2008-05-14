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

#include "wirelessnetworkinterface.h"
#include "wirelessnetworkinterface_p.h"

#include "soliddefs_p.h"
#include <solid/control/wirelessaccesspoint.h>
#include <solid/control/ifaces/wirelessaccesspoint.h>
#include <solid/control/ifaces/wirelessnetworkinterface.h>

Solid::Control::WirelessNetworkInterface::WirelessNetworkInterface(QObject *backendObject)
    : NetworkInterface(*new WirelessNetworkInterfacePrivate(this), backendObject)
{
    Q_D(WirelessNetworkInterface);
    d->setBackendObject(backendObject);
    makeConnections( backendObject );
}

Solid::Control::WirelessNetworkInterface::WirelessNetworkInterface(const WirelessNetworkInterface &networkinterface)
    : NetworkInterface(*new WirelessNetworkInterfacePrivate(this), networkinterface)
{
    Q_D(WirelessNetworkInterface);
    d->setBackendObject(networkinterface.d_ptr->backendObject());
    makeConnections( networkinterface.d_ptr->backendObject() );
}

Solid::Control::WirelessNetworkInterface::WirelessNetworkInterface(WirelessNetworkInterfacePrivate &dd, QObject *backendObject)
    : NetworkInterface(dd, backendObject)
{
    makeConnections( backendObject );
}

Solid::Control::WirelessNetworkInterface::WirelessNetworkInterface(WirelessNetworkInterfacePrivate &dd, const WirelessNetworkInterface &networkinterface)
    : NetworkInterface(dd, networkinterface.d_ptr->backendObject())
{
    makeConnections( networkinterface.d_ptr->backendObject() );
}

Solid::Control::WirelessNetworkInterface::~WirelessNetworkInterface()
{

}

Solid::Control::NetworkInterface::Type Solid::Control::WirelessNetworkInterface::type() const
{
    return Ieee80211;
}

void Solid::Control::WirelessNetworkInterface::makeConnections(QObject * source)
{
    connect(source, SIGNAL(accessPointAppeared(const QString &)),
            this, SLOT(_k_accessPointAdded(const QString &)));
    connect(source, SIGNAL(accessPointDisappeared(const QString &)),
            this, SLOT(_k_accessPointRemoved(const QString &)));
}

QString Solid::Control::WirelessNetworkInterface::hardwareAddress() const
{
    Q_D(const WirelessNetworkInterface);
    return_SOLID_CALL(Ifaces::WirelessNetworkInterface *, d->backendObject(), QString(), hardwareAddress());
}

QString Solid::Control::WirelessNetworkInterface::activeAccessPoint() const
{
    Q_D(const WirelessNetworkInterface);
    return_SOLID_CALL(Ifaces::WirelessNetworkInterface *, d->backendObject(), QString(), activeAccessPoint());
}

int Solid::Control::WirelessNetworkInterface::bitRate() const
{
    Q_D(const WirelessNetworkInterface);
    return_SOLID_CALL(Ifaces::WirelessNetworkInterface *, d->backendObject(), 0, bitRate());
}

Solid::Control::WirelessNetworkInterface::OperationMode Solid::Control::WirelessNetworkInterface::mode() const
{
    Q_D(const WirelessNetworkInterface);
    return_SOLID_CALL(Ifaces::WirelessNetworkInterface *, d->backendObject(), (Solid::Control::WirelessNetworkInterface::OperationMode)0, mode());
}

Solid::Control::WirelessNetworkInterface::Capabilities Solid::Control::WirelessNetworkInterface::wirelessCapabilities() const
{
    Q_D(const WirelessNetworkInterface);
    return_SOLID_CALL(Ifaces::WirelessNetworkInterface *, d->backendObject(), (Solid::Control::WirelessNetworkInterface::Capabilities)0, wirelessCapabilities());
}

Solid::Control::AccessPoint * Solid::Control::WirelessNetworkInterface::findAccessPoint(const QString  & uni) const
{
    Q_D(const WirelessNetworkInterface);
    QPair<AccessPoint*, Ifaces::AccessPoint*> pair = findRegisteredAccessPoint(uni);
    return pair.first;
}

void Solid::Control::WirelessNetworkInterfacePrivate::setBackendObject(QObject *object)
{
    NetworkInterfacePrivate::setBackendObject(object);

    if (object) {
        QObject::connect(object, SIGNAL(bitRateChanged(int)),
                         parent(), SIGNAL(bitRateChanged(int)));
        QObject::connect(object, SIGNAL(activeAccessPointChanged(const QString&)),
                         parent(), SIGNAL(activeAccessPointChanged(const QString&)));
        QObject::connect(object, SIGNAL(modeChanged(Solid::Control::WirelessNetworkInterface::OperationMode)),
                         parent(), SIGNAL(modeChanged(Solid::Control::WirelessNetworkInterface::OperationMode)));
        QObject::connect(object, SIGNAL(accessPointAppeared(const QString&)),
                         parent(), SIGNAL(accessPointAppeared(const QString&)));
        QObject::connect(object, SIGNAL(accessPointDisappeared(const QString&)),
                         parent(), SIGNAL(accessPointDisappeared(const QString&)));
    }
}

Solid::Control::AccessPointList Solid::Control::WirelessNetworkInterface::accessPoints() const
{
    Q_D(const WirelessNetworkInterface);
    return d->apMap.keys();
}

void Solid::Control::WirelessNetworkInterface::_k_accessPointAdded(const QString & uni)
{
    Q_D(WirelessNetworkInterface);
    QPair<AccessPoint *, Ifaces::AccessPoint *> pair = d->apMap.take(uni);

    if (pair.first!= 0)
    {
        // Oops, I'm not sure it should happen...
        // But well in this case we'd better kill the old device we got, it's probably outdated

        delete pair.first;
        delete pair.second;
    }

    emit accessPointAppeared(uni);
}

void Solid::Control::WirelessNetworkInterface::_k_accessPointRemoved(const QString & uni)
{
    Q_D(WirelessNetworkInterface);
    QPair<AccessPoint *, Ifaces::AccessPoint *> pair = d->apMap.take(uni);

    if (pair.first!= 0)
    {
        delete pair.first;
        delete pair.second;
    }

    emit accessPointDisappeared(uni);
}

void Solid::Control::WirelessNetworkInterface::_k_destroyed(QObject *object)
{
    Q_D(WirelessNetworkInterface);
    Ifaces::AccessPoint *ap = qobject_cast<Ifaces::AccessPoint *>(object);

    if (ap!=0)
    {
        QString uni = ap->uni();
        QPair<AccessPoint *, Ifaces::AccessPoint *> pair = d->apMap.take(uni);
        delete pair.first;
    }
}

QPair<Solid::Control::AccessPoint *, Solid::Control::Ifaces::AccessPoint *>
Solid::Control::WirelessNetworkInterface::findRegisteredAccessPoint(const QString &uni) const
{
    Q_D(const WirelessNetworkInterface);

    if (d->apMap.contains(uni)) {
        return d->apMap[uni];
    } else {
        Ifaces::WirelessNetworkInterface *device = qobject_cast<Ifaces::WirelessNetworkInterface *>(d->backendObject());
        AccessPoint *ap = 0;

        if (device!=0) {
            Ifaces::AccessPoint *iface = qobject_cast<Ifaces::AccessPoint *>( device->createAccessPoint(uni) );

            if (qobject_cast<Ifaces::AccessPoint *>(iface)!=0) {
                ap = new AccessPoint(iface);
            }

            if (ap != 0) {
                QPair<AccessPoint *, Ifaces::AccessPoint *> pair(ap, iface);
                QObject::connect(iface, SIGNAL(destroyed(QObject *)),
                        this, SLOT(_k_destroyed(QObject *)));

                d->apMap[uni] = pair;
                return pair;
            }
        }
    }

    return QPair<AccessPoint *, Ifaces::AccessPoint *>(0, 0);
}

#include "wirelessnetworkinterface.moc"
