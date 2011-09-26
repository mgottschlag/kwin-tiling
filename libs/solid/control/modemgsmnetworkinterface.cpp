/*  This file is part of the KDE project
    Copyright (C) 2010 Lamarque Souza <lamarque@gmail.com>

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

#include <KDebug>

#include "modemgsmnetworkinterface.h"
#include "modemgsmnetworkinterface_p.h"

#include "frontendobject_p.h"
#include "soliddefs_p.h"
#include "ifaces/modemgsmnetworkinterface.h"

Solid::Control::ModemGsmNetworkInterface::ModemGsmNetworkInterface(QObject *backendObject)
    : ModemInterface(*new ModemGsmNetworkInterfacePrivate(this), backendObject)
{
    Q_D(ModemGsmNetworkInterface);
    d->setBackendObject(backendObject);
    makeConnections( backendObject );
}

Solid::Control::ModemGsmNetworkInterface::ModemGsmNetworkInterface(const ModemGsmNetworkInterface &networkinterface)
    : ModemInterface(*new ModemGsmNetworkInterfacePrivate(this), networkinterface)
{
    Q_D(ModemGsmNetworkInterface);
    d->setBackendObject(networkinterface.d_ptr->backendObject());
    makeConnections( networkinterface.d_ptr->backendObject() );
}

Solid::Control::ModemGsmNetworkInterface::~ModemGsmNetworkInterface()
{
}

Solid::Control::ModemInterface::Type Solid::Control::ModemGsmNetworkInterface::type() const
{
    return GsmType;
}

void Solid::Control::ModemGsmNetworkInterface::registerToNetwork(const QString & networkId) const
{
    Q_D(const ModemGsmNetworkInterface);
    SOLID_CALL(Ifaces::ModemGsmNetworkInterface *, d->backendObject(), registerToNetwork(networkId));
}

Solid::Control::ModemGsmNetworkInterface::ScanResultsType Solid::Control::ModemGsmNetworkInterface::scan() const
{
    Q_D(const ModemGsmNetworkInterface);
    return_SOLID_CALL(Ifaces::ModemGsmNetworkInterface *, d->backendObject(), ScanResultsType(), scan());
}

void Solid::Control::ModemGsmNetworkInterface::setApn(const QString & apn) const
{
    Q_D(const ModemGsmNetworkInterface);
    SOLID_CALL(Ifaces::ModemGsmNetworkInterface *, d->backendObject(), setApn(apn));
}

uint Solid::Control::ModemGsmNetworkInterface::getSignalQuality() const
{
    Q_D(const ModemGsmNetworkInterface);
    return_SOLID_CALL(Ifaces::ModemGsmNetworkInterface *, d->backendObject(), 0, getSignalQuality());
}

void Solid::Control::ModemGsmNetworkInterface::setBand(const Band band) const
{
    Q_D(const ModemGsmNetworkInterface);
    SOLID_CALL(Ifaces::ModemGsmNetworkInterface *, d->backendObject(), setBand(band));
}

Solid::Control::ModemInterface::Band Solid::Control::ModemGsmNetworkInterface::getBand() const
{
    Q_D(const ModemGsmNetworkInterface);
    return_SOLID_CALL(Ifaces::ModemGsmNetworkInterface *, d->backendObject(), UnknownBand, getBand());
}

Solid::Control::ModemGsmNetworkInterface::RegistrationInfoType Solid::Control::ModemGsmNetworkInterface::getRegistrationInfo() const
{
    Q_D(const ModemGsmNetworkInterface);
    return_SOLID_CALL(Ifaces::ModemGsmNetworkInterface *, d->backendObject(), RegistrationInfoType(), getRegistrationInfo());
}

void Solid::Control::ModemGsmNetworkInterface::setAllowedMode(const AllowedMode mode) const
{
    Q_D(const ModemGsmNetworkInterface);
    SOLID_CALL(Ifaces::ModemGsmNetworkInterface *, d->backendObject(), setAllowedMode(mode));
}

Solid::Control::ModemInterface::AllowedMode Solid::Control::ModemGsmNetworkInterface::getAllowedMode() const
{
    Q_D(const ModemGsmNetworkInterface);
    return_SOLID_CALL(Ifaces::ModemGsmNetworkInterface *, d->backendObject(), AnyModeAllowed, getAllowedMode());
}

Solid::Control::ModemInterface::AccessTechnology Solid::Control::ModemGsmNetworkInterface::getAccessTechnology() const
{
    Q_D(const ModemGsmNetworkInterface);
    return_SOLID_CALL(Ifaces::ModemGsmNetworkInterface *, d->backendObject(), UnknownTechnology, getAccessTechnology());
}

void Solid::Control::ModemGsmNetworkInterface::makeConnections(QObject * source)
{
    if (source) {
        QObject::connect(source, SIGNAL(registrationInfoChanged(Solid::Control::ModemGsmNetworkInterface::RegistrationInfoType)),
                this, SIGNAL(registrationInfoChanged(Solid::Control::ModemGsmNetworkInterface::RegistrationInfoType)));
        QObject::connect(source, SIGNAL(signalQualityChanged(uint)),
                this, SIGNAL(signalQualityChanged(uint)));
        QObject::connect(source, SIGNAL(allowedModeChanged(Solid::Control::ModemInterface::AllowedMode)),
                this, SIGNAL(allowedModeChanged(Solid::Control::ModemInterface::AllowedMode)));
        QObject::connect(source, SIGNAL(accessTechnologyChanged(Solid::Control::ModemInterface::AccessTechnology)),
                this, SIGNAL(accessTechnologyChanged(Solid::Control::ModemInterface::AccessTechnology)));
    }
}

void Solid::Control::ModemGsmNetworkInterfacePrivate::setBackendObject(QObject *object)
{
    ModemInterfacePrivate::setBackendObject(object);
}

void Solid::Control::ModemGsmNetworkInterface::_k_destroyed(QObject *object)
{
    Q_UNUSED(object);
}


