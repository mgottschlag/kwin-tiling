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

#include "modemgsmsmsinterface.h"
#include "modemgsmsmsinterface_p.h"

#include "frontendobject_p.h"
#include "soliddefs_p.h"
#include "ifaces/modemgsmsmsinterface.h"

Solid::Control::ModemGsmSmsInterface::ModemGsmSmsInterface(QObject *backendObject)
    : ModemInterface(*new ModemGsmSmsInterfacePrivate(this), backendObject)
{
    Q_D(ModemGsmSmsInterface);
    d->setBackendObject(backendObject);
    makeConnections( backendObject );
}

Solid::Control::ModemGsmSmsInterface::ModemGsmSmsInterface(const ModemGsmSmsInterface &smsinterface)
    : ModemInterface(*new ModemGsmSmsInterfacePrivate(this), smsinterface)
{
    Q_D(ModemGsmSmsInterface);
    d->setBackendObject(smsinterface.d_ptr->backendObject());
    makeConnections( smsinterface.d_ptr->backendObject() );
}

Solid::Control::ModemGsmSmsInterface::~ModemGsmSmsInterface()
{
}

void Solid::Control::ModemGsmSmsInterface::deleteSms(const int index) const
{
    Q_D(const ModemGsmSmsInterface);
    SOLID_CALL(Ifaces::ModemGsmSmsInterface *, d->backendObject(), deleteSms(index));
}

QVariantMap Solid::Control::ModemGsmSmsInterface::get(const int index) const
{
    Q_D(const ModemGsmSmsInterface);
    return_SOLID_CALL(Ifaces::ModemGsmSmsInterface *, d->backendObject(), QVariantMap(), get(index));
}

int Solid::Control::ModemGsmSmsInterface::getFormat() const
{
    Q_D(const ModemGsmSmsInterface);
    return_SOLID_CALL(Ifaces::ModemGsmSmsInterface *, d->backendObject(), -1, getFormat());
}

void Solid::Control::ModemGsmSmsInterface::setFormat(const int format) const
{
    Q_D(const ModemGsmSmsInterface);
    SOLID_CALL(Ifaces::ModemGsmSmsInterface *, d->backendObject(), setFormat(format));
}

QString Solid::Control::ModemGsmSmsInterface::getSmsc() const
{
    Q_D(const ModemGsmSmsInterface);
    return_SOLID_CALL(Ifaces::ModemGsmSmsInterface *, d->backendObject(), QString(), getSmsc());
}

QList<QVariantMap> Solid::Control::ModemGsmSmsInterface::list() const
{
    Q_D(const ModemGsmSmsInterface);
    return_SOLID_CALL(Ifaces::ModemGsmSmsInterface *, d->backendObject(), QList<QVariantMap>(), list());
}

void Solid::Control::ModemGsmSmsInterface::save(const QVariantMap & properties) const
{
    Q_D(const ModemGsmSmsInterface);
    SOLID_CALL(Ifaces::ModemGsmSmsInterface *, d->backendObject(), save(properties));
}

void Solid::Control::ModemGsmSmsInterface::send(const QVariantMap & properties) const
{
    Q_D(const ModemGsmSmsInterface);
    SOLID_CALL(Ifaces::ModemGsmSmsInterface *, d->backendObject(), send(properties));
}

void Solid::Control::ModemGsmSmsInterface::sendFromStorage(const int index) const
{
    Q_D(const ModemGsmSmsInterface);
    SOLID_CALL(Ifaces::ModemGsmSmsInterface *, d->backendObject(), sendFromStorage(index));
}

void Solid::Control::ModemGsmSmsInterface::setIndication(const int mode, const int mt, const int bm, const int ds, const int brf) const
{
    Q_D(const ModemGsmSmsInterface);
    SOLID_CALL(Ifaces::ModemGsmSmsInterface *, d->backendObject(), setIndication(mode, mt, bm, ds, brf));
}

void Solid::Control::ModemGsmSmsInterface::makeConnections(QObject * source)
{
    if (source) {
        QObject::connect(source, SIGNAL(smsReceived(int,bool)),
                this, SIGNAL(smsReceived(int,bool)));
        QObject::connect(source, SIGNAL(completed(int,bool)),
                this, SIGNAL(completed(int,bool)));
    }
}

void Solid::Control::ModemGsmSmsInterfacePrivate::setBackendObject(QObject *object)
{
    ModemInterfacePrivate::setBackendObject(object);
}

void Solid::Control::ModemGsmSmsInterface::_k_destroyed(QObject *object)
{
    Q_UNUSED(object);
}
