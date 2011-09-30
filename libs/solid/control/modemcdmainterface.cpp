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

#include "modemcdmainterface.h"
#include "modemcdmainterface_p.h"

#include "frontendobject_p.h"
#include "soliddefs_p.h"
#include "ifaces/modemcdmainterface.h"

Solid::Control::ModemCdmaInterface::ModemCdmaInterface(QObject *backendObject)
    : ModemInterface(*new ModemCdmaInterfacePrivate(this), backendObject)
{
    Q_D(ModemCdmaInterface);
    d->setBackendObject(backendObject);
    makeConnections( backendObject );
}

Solid::Control::ModemCdmaInterface::ModemCdmaInterface(const ModemCdmaInterface &networkinterface)
    : ModemInterface(*new ModemCdmaInterfacePrivate(this), networkinterface)
{
    Q_D(ModemCdmaInterface);
    d->setBackendObject(networkinterface.d_ptr->backendObject());
    makeConnections( networkinterface.d_ptr->backendObject() );
}

Solid::Control::ModemCdmaInterface::~ModemCdmaInterface()
{
}

Solid::Control::ModemInterface::Type Solid::Control::ModemCdmaInterface::type() const
{
    return CdmaType;
}

uint Solid::Control::ModemCdmaInterface::getSignalQuality() const
{
    Q_D(const ModemCdmaInterface);
    return_SOLID_CALL(Ifaces::ModemCdmaInterface *, d->backendObject(), 0, getSignalQuality());
}

QString Solid::Control::ModemCdmaInterface::getEsn() const
{
    Q_D(const ModemCdmaInterface);
    return_SOLID_CALL(Ifaces::ModemCdmaInterface *, d->backendObject(), 0, getEsn());
}

Solid::Control::ModemCdmaInterface::ServingSystemType Solid::Control::ModemCdmaInterface::getServingSystem() const
{
    Q_D(const ModemCdmaInterface);
    return_SOLID_CALL(Ifaces::ModemCdmaInterface *, d->backendObject(), ServingSystemType(), getServingSystem());
}

Solid::Control::ModemCdmaInterface::RegistrationStateResult Solid::Control::ModemCdmaInterface::getRegistrationState() const
{
    Q_D(const ModemCdmaInterface);
    return_SOLID_CALL(Ifaces::ModemCdmaInterface *, d->backendObject(), RegistrationStateResult(), getRegistrationState());
}

void Solid::Control::ModemCdmaInterface::makeConnections(QObject * source)
{
    if (source) {
        QObject::connect(source, SIGNAL(signalQualityChanged(uint)),
                this, SIGNAL(signalQualityChanged(uint)));
        QObject::connect(source, SIGNAL(registrationStateChanged(Solid::Control::ModemCdmaInterface::RegistrationState,Solid::Control::ModemCdmaInterface::RegistrationState)),
                this, SIGNAL(registrationStateChanged(Solid::Control::ModemCdmaInterface::RegistrationState,Solid::Control::ModemCdmaInterface::RegistrationState)));
    }
}

void Solid::Control::ModemCdmaInterfacePrivate::setBackendObject(QObject *object)
{
    ModemInterfacePrivate::setBackendObject(object);
}

void Solid::Control::ModemCdmaInterface::_k_destroyed(QObject *object)
{
    Q_UNUSED(object);
}
