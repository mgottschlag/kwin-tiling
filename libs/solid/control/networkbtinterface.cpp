/*
Copyright 2011 Lamarque Souza <lamarque@gmail.com>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) version 3, or any
later version accepted by the membership of KDE e.V. (or its
successor approved by the membership of KDE e.V.), which shall
act as a proxy defined in Section 6 of version 3 of the license.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <KLocale>

#include "networkbtinterface.h"
#include "networkbtinterface_p.h"

#include "frontendobject_p.h"
#include "soliddefs_p.h"
#include "ifaces/networkbtinterface.h"

Solid::Control::BtNetworkInterface::BtNetworkInterface(QObject *backendObject)
    : GsmNetworkInterface(*new BtNetworkInterfacePrivate(this), backendObject)
{
    Q_D(BtNetworkInterface);
    d->setBackendObject(backendObject);
    makeConnections( backendObject );
}

Solid::Control::BtNetworkInterface::BtNetworkInterface(const BtNetworkInterface &networkinterface)
    : GsmNetworkInterface(*new BtNetworkInterfacePrivate(this), networkinterface)
{
    Q_D(BtNetworkInterface);
    d->setBackendObject(networkinterface.d_ptr->backendObject());
    makeConnections( networkinterface.d_ptr->backendObject() );
}

Solid::Control::BtNetworkInterface::BtNetworkInterface(BtNetworkInterfacePrivate &dd, QObject *backendObject)
    : GsmNetworkInterface(dd, backendObject)
{
    makeConnections( backendObject );
}

Solid::Control::BtNetworkInterface::BtNetworkInterface(BtNetworkInterfacePrivate &dd, const BtNetworkInterface &networkinterface)
    : GsmNetworkInterface(dd, networkinterface.d_ptr->backendObject())
{
    makeConnections( networkinterface.d_ptr->backendObject() );
}

Solid::Control::BtNetworkInterface::~BtNetworkInterface()
{
}

Solid::Control::NetworkInterface::Type Solid::Control::BtNetworkInterface::type() const
{
    return Bluetooth;
}

void Solid::Control::BtNetworkInterface::makeConnections(QObject * source)
{
    connect(source, SIGNAL(networkNameChanged(QString)),
            this, SIGNAL(networkNameChanged(QString)));
}

Solid::Control::BtNetworkInterface::Capabilities Solid::Control::BtNetworkInterface::btCapabilities() const
{
    Q_D(const BtNetworkInterface);
    return_SOLID_CALL(Ifaces::BtNetworkInterface *, d->backendObject(), (Solid::Control::BtNetworkInterface::Capabilities)0, btCapabilities());
}

QString Solid::Control::BtNetworkInterface::hardwareAddress() const
{
    Q_D(const BtNetworkInterface);
    return_SOLID_CALL(Ifaces::BtNetworkInterface *, d->backendObject(), QString(), hardwareAddress());
}

QString Solid::Control::BtNetworkInterface::name() const
{
    Q_D(const BtNetworkInterface);
    return_SOLID_CALL(Ifaces::BtNetworkInterface *, d->backendObject(), QString(), name());
}

void Solid::Control::BtNetworkInterfacePrivate::setBackendObject(QObject *object)
{
    GsmNetworkInterfacePrivate::setBackendObject(object);
}

void Solid::Control::BtNetworkInterface::_k_destroyed(QObject *object)
{
    Q_UNUSED(object);
}
// vim: sw=4 sts=4 et tw=100
