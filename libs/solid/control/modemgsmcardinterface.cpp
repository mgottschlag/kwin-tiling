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
#include <KLocale>

#include "modemgsmcardinterface.h"
#include "modemgsmcardinterface_p.h"

#include "frontendobject_p.h"
#include "soliddefs_p.h"
#include "ifaces/modemgsmcardinterface.h"

Solid::Control::ModemGsmCardInterface::ModemGsmCardInterface(QObject *backendObject)
    : ModemInterface(*new ModemGsmCardInterfacePrivate(this), backendObject)
{
    Q_D(ModemGsmCardInterface);
    d->setBackendObject(backendObject);
    makeConnections( backendObject );
}

Solid::Control::ModemGsmCardInterface::ModemGsmCardInterface(const ModemGsmCardInterface &cardinterface)
    : ModemInterface(*new ModemGsmCardInterfacePrivate(this), cardinterface)
{
    Q_D(ModemGsmCardInterface);
    d->setBackendObject(cardinterface.d_ptr->backendObject());
    makeConnections( cardinterface.d_ptr->backendObject() );
}

Solid::Control::ModemGsmCardInterface::~ModemGsmCardInterface()
{
}

QString Solid::Control::ModemGsmCardInterface::getImei() const
{
    Q_D(const ModemGsmCardInterface);
    return_SOLID_CALL(Ifaces::ModemGsmCardInterface *, d->backendObject(), QString(), getImei());
}

QString Solid::Control::ModemGsmCardInterface::getImsi() const
{
    Q_D(const ModemGsmCardInterface);
    return_SOLID_CALL(Ifaces::ModemGsmCardInterface *, d->backendObject(), QString(), getImsi());
}

QDBusPendingReply<> Solid::Control::ModemGsmCardInterface::sendPuk(const QString & puk, const QString & pin) const
{
    Q_D(const ModemGsmCardInterface);
    return_SOLID_CALL(Ifaces::ModemGsmCardInterface *, d->backendObject(), QDBusPendingReply<>(), sendPuk(puk, pin));
}

QDBusPendingReply<> Solid::Control::ModemGsmCardInterface::sendPin(const QString & pin) const
{
    Q_D(const ModemGsmCardInterface);
    return_SOLID_CALL(Ifaces::ModemGsmCardInterface *, d->backendObject(), QDBusPendingReply<>(), sendPin(pin));
}

QDBusPendingReply<> Solid::Control::ModemGsmCardInterface::enablePin(const QString & pin, const bool enabled) const
{
    Q_D(const ModemGsmCardInterface);
    return_SOLID_CALL(Ifaces::ModemGsmCardInterface *, d->backendObject(), QDBusPendingReply<>(), enablePin(pin, enabled));
}

QDBusPendingReply<> Solid::Control::ModemGsmCardInterface::changePin(const QString & oldPin, const QString & newPin) const
{
    Q_D(const ModemGsmCardInterface);
    return_SOLID_CALL(Ifaces::ModemGsmCardInterface *, d->backendObject(), QDBusPendingReply<>(), changePin(oldPin, newPin));
}

Solid::Control::ModemInterface::Band Solid::Control::ModemGsmCardInterface::getSupportedBands() const
{
    Q_D(const ModemGsmCardInterface);
    return_SOLID_CALL(Ifaces::ModemGsmCardInterface *, d->backendObject(), Solid::Control::ModemInterface::UnknownBand, getSupportedBands());
}

Solid::Control::ModemInterface::Mode Solid::Control::ModemGsmCardInterface::getSupportedModes() const
{
    Q_D(const ModemGsmCardInterface);
    return_SOLID_CALL(Ifaces::ModemGsmCardInterface *, d->backendObject(), Solid::Control::ModemInterface::UnknownMode, getSupportedModes());
}

void Solid::Control::ModemGsmCardInterface::makeConnections(QObject * source)
{
    if (source) {
        QObject::connect(source, SIGNAL(supportedBandsChanged(const Solid::Control::ModemInterface::Band)),
                this, SIGNAL(supportedBandsChanged(const Solid::Control::ModemInterface::Band)));
        QObject::connect(source, SIGNAL(supportedModesChanged(const Solid::Control::ModemInterface::Mode)),
                this, SIGNAL(supportedModesChanged(const Solid::Control::ModemInterface::Mode)));
    }
}

void Solid::Control::ModemGsmCardInterfacePrivate::setBackendObject(QObject *object)
{
    ModemInterfacePrivate::setBackendObject(object);
}

void Solid::Control::ModemGsmCardInterface::_k_destroyed(QObject *object)
{
    Q_UNUSED(object);
}
