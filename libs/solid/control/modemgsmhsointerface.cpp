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

#include "modemgsmhsointerface.h"
#include "modemgsmhsointerface_p.h"

#include "frontendobject_p.h"
#include "soliddefs_p.h"
#include "ifaces/modemgsmhsointerface.h"

Solid::Control::ModemGsmHsoInterface::ModemGsmHsoInterface(QObject *backendObject)
    : ModemInterface(*new ModemGsmHsoInterfacePrivate(this), backendObject)
{
    Q_D(ModemGsmHsoInterface);
    d->setBackendObject(backendObject);
    makeConnections( backendObject );
}

Solid::Control::ModemGsmHsoInterface::ModemGsmHsoInterface(const ModemGsmHsoInterface &hsointerface)
    : ModemInterface(*new ModemGsmHsoInterfacePrivate(this), hsointerface)
{
    Q_D(ModemGsmHsoInterface);
    d->setBackendObject(hsointerface.d_ptr->backendObject());
    makeConnections( hsointerface.d_ptr->backendObject() );
}

Solid::Control::ModemGsmHsoInterface::~ModemGsmHsoInterface()
{
}

void Solid::Control::ModemGsmHsoInterface::authenticate(const QString & username, const QString & password) const
{
    Q_D(const ModemGsmHsoInterface);
    SOLID_CALL(Ifaces::ModemGsmHsoInterface *, d->backendObject(), authenticate(username, password));
}

void Solid::Control::ModemGsmHsoInterface::makeConnections(QObject * source)
{
    if (source) {
    }
}

void Solid::Control::ModemGsmHsoInterfacePrivate::setBackendObject(QObject *object)
{
    ModemInterfacePrivate::setBackendObject(object);
}

void Solid::Control::ModemGsmHsoInterface::_k_destroyed(QObject *object)
{
    Q_UNUSED(object);
}
