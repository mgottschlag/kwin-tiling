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

#include "modemgsmcontactsinterface.h"
#include "modemgsmcontactsinterface_p.h"

#include "frontendobject_p.h"
#include "soliddefs_p.h"
#include "ifaces/modemgsmcontactsinterface.h"

Solid::Control::ModemGsmContactsInterface::ModemGsmContactsInterface(QObject *backendObject)
    : ModemInterface(*new ModemGsmContactsInterfacePrivate(this), backendObject)
{
    Q_D(ModemGsmContactsInterface);
    d->setBackendObject(backendObject);
    makeConnections( backendObject );
}

Solid::Control::ModemGsmContactsInterface::ModemGsmContactsInterface(const ModemGsmContactsInterface &contactsIface)
    : ModemInterface(*new ModemGsmContactsInterfacePrivate(this), contactsIface)
{
    Q_D(ModemGsmContactsInterface);
    d->setBackendObject(contactsIface.d_ptr->backendObject());
    makeConnections(contactsIface.d_ptr->backendObject());
}

Solid::Control::ModemGsmContactsInterface::~ModemGsmContactsInterface()
{
}

int Solid::Control::ModemGsmContactsInterface::addContact(const QString & name, const QString & number) const
{
    Q_D(const ModemGsmContactsInterface);
    return_SOLID_CALL(Ifaces::ModemGsmContactsInterface *, d->backendObject(), -1, addContact(name, number));
}

void Solid::Control::ModemGsmContactsInterface::deleteContact(const int index) const
{
    Q_D(const ModemGsmContactsInterface);
    SOLID_CALL(Ifaces::ModemGsmContactsInterface *, d->backendObject(), deleteContact(index));
}

Solid::Control::ModemGsmContactsInterface::ContactType Solid::Control::ModemGsmContactsInterface::get(const int index) const
{
    Q_D(const ModemGsmContactsInterface);
    return_SOLID_CALL(Ifaces::ModemGsmContactsInterface *, d->backendObject(), ContactType(), get(index));
}

QList<Solid::Control::ModemGsmContactsInterface::ContactType> Solid::Control::ModemGsmContactsInterface::list() const
{
    Q_D(const ModemGsmContactsInterface);
    return_SOLID_CALL(Ifaces::ModemGsmContactsInterface *, d->backendObject(), QList<ContactType>(), list());
}

QList<Solid::Control::ModemGsmContactsInterface::ContactType> Solid::Control::ModemGsmContactsInterface::find(const QString & pattern) const
{
    Q_D(const ModemGsmContactsInterface);
    return_SOLID_CALL(Ifaces::ModemGsmContactsInterface *, d->backendObject(), QList<ContactType>(), find(pattern));
}

int Solid::Control::ModemGsmContactsInterface::getCount() const
{
    Q_D(const ModemGsmContactsInterface);
    return_SOLID_CALL(Ifaces::ModemGsmContactsInterface *, d->backendObject(), 0, getCount());
}

void Solid::Control::ModemGsmContactsInterface::makeConnections(QObject * source)
{
    if (source) {
    }
}

void Solid::Control::ModemGsmContactsInterfacePrivate::setBackendObject(QObject *object)
{
    ModemInterfacePrivate::setBackendObject(object);
}

void Solid::Control::ModemGsmContactsInterface::_k_destroyed(QObject *object)
{
    Q_UNUSED(object);
}
