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

#include "modemmanagerinterface.h"
#include "modemmanagerinterface_p.h"

#include "soliddefs_p.h"
#include "ifaces/modemmanagerinterface.h"

Solid::Control::ModemManagerInterface::ModemManagerInterface(QObject *backendObject)
    : QObject(), d_ptr(new ModemManagerInterfacePrivate(this))
{
    Q_D(ModemManagerInterface); d->setBackendObject(backendObject);
}

Solid::Control::ModemManagerInterface::ModemManagerInterface(const ModemManagerInterface &other)
    : QObject(), d_ptr(new ModemManagerInterfacePrivate(this))
{
    Q_D(ModemManagerInterface);
    d->setBackendObject(other.d_ptr->backendObject());
}

Solid::Control::ModemManagerInterface::ModemManagerInterface(ModemManagerInterfacePrivate &dd, QObject *backendObject)
    : QObject(), d_ptr(&dd)
{
    Q_UNUSED(backendObject);
}

Solid::Control::ModemManagerInterface::ModemManagerInterface(ModemManagerInterfacePrivate &dd, const ModemManagerInterface &other)
    : d_ptr(&dd)
{
    Q_UNUSED(other);
}

Solid::Control::ModemManagerInterface::~ModemManagerInterface()
{
    delete d_ptr;
}

void Solid::Control::ModemManagerInterfacePrivate::setBackendObject(QObject *object)
{
    FrontendObjectPrivate::setBackendObject(object);

    if (object) {
        QObject::connect(object, SIGNAL(connectionStateChanged(int)),
                         parent(), SIGNAL(connectionStateChanged(int)));
        QObject::connect(object, SIGNAL(connectionStateChanged(int,int,int)),
                         parent(), SIGNAL(connectionStateChanged(int,int,int)));
    }
}


#include "modemmanagerinterface.moc"
