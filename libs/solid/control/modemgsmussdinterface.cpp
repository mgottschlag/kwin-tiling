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

#include "modemgsmussdinterface.h"
#include "modemgsmussdinterface_p.h"

#include "frontendobject_p.h"
#include "soliddefs_p.h"
#include "ifaces/modemgsmussdinterface.h"

Solid::Control::ModemGsmUssdInterface::ModemGsmUssdInterface(QObject *backendObject)
    : ModemInterface(*new ModemGsmUssdInterfacePrivate(this), backendObject)
{
    Q_D(ModemGsmUssdInterface);
    d->setBackendObject(backendObject);
    makeConnections( backendObject );
}

Solid::Control::ModemGsmUssdInterface::ModemGsmUssdInterface(const ModemGsmUssdInterface &ussdinterface)
    : ModemInterface(*new ModemGsmUssdInterfacePrivate(this), ussdinterface)
{
    Q_D(ModemGsmUssdInterface);
    d->setBackendObject(ussdinterface.d_ptr->backendObject());
    makeConnections(ussdinterface.d_ptr->backendObject());
}

Solid::Control::ModemGsmUssdInterface::~ModemGsmUssdInterface()
{
}

QString Solid::Control::ModemGsmUssdInterface::initiate(const QString & command) const
{
    Q_D(const ModemGsmUssdInterface);
    return_SOLID_CALL(Ifaces::ModemGsmUssdInterface *, d->backendObject(), QString(), initiate(command));
}

void Solid::Control::ModemGsmUssdInterface::respond(const QString response) const
{
    Q_D(const ModemGsmUssdInterface);
    SOLID_CALL(Ifaces::ModemGsmUssdInterface *, d->backendObject(), respond(response));
}

void Solid::Control::ModemGsmUssdInterface::cancel() const
{
    Q_D(const ModemGsmUssdInterface);
    SOLID_CALL(Ifaces::ModemGsmUssdInterface *, d->backendObject(), cancel());
}

QString Solid::Control::ModemGsmUssdInterface::getState() const
{
    Q_D(const ModemGsmUssdInterface);
    return_SOLID_CALL(Ifaces::ModemGsmUssdInterface *, d->backendObject(), QString(), getState());
}

QString Solid::Control::ModemGsmUssdInterface::getNetworkNotification() const
{
    Q_D(const ModemGsmUssdInterface);
    return_SOLID_CALL(Ifaces::ModemGsmUssdInterface *, d->backendObject(), QString(), getNetworkNotification());
}

QString Solid::Control::ModemGsmUssdInterface::getNetworkRequest() const
{
    Q_D(const ModemGsmUssdInterface);
    return_SOLID_CALL(Ifaces::ModemGsmUssdInterface *, d->backendObject(), QString(), getNetworkRequest());
}

void Solid::Control::ModemGsmUssdInterface::slotStateChanged(const QString & state)
{
    emit stateChanged(state);
}

void Solid::Control::ModemGsmUssdInterface::slotNetworkNotificationChanged(const QString & networkNotification)
{
    emit networkNotificationChanged(networkNotification);
}

void Solid::Control::ModemGsmUssdInterface::slotNetworkRequestChanged(const QString & networkRequest)
{
    emit networkRequestChanged(networkRequest);
}

void Solid::Control::ModemGsmUssdInterface::makeConnections(QObject * source)
{
    if (source) {
        QObject::connect(source, SIGNAL(stateChanged(QString)),
                this, SLOT(slotStateChanged(QString)));
        QObject::connect(source, SIGNAL(networkNotificationChanged(QString)),
                this, SLOT(slotNetworkNotificationChanged(QString)));
        QObject::connect(source, SIGNAL(networkRequestChanged(QString)),
                this, SLOT(slotNetworkRequestChanged(QString)));
    }
}

void Solid::Control::ModemGsmUssdInterfacePrivate::setBackendObject(QObject *object)
{
    ModemInterfacePrivate::setBackendObject(object);
}

void Solid::Control::ModemGsmUssdInterface::_k_destroyed(QObject *object)
{
    Q_UNUSED(object);
}
