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

#include "modemlocationinterface.h"
#include "modemlocationinterface_p.h"

#include "frontendobject_p.h"
#include "soliddefs_p.h"
#include "ifaces/modemlocationinterface.h"

Solid::Control::ModemLocationInterface::ModemLocationInterface(QObject *backendObject)
    : ModemInterface(*new ModemLocationInterfacePrivate(this), backendObject)
{
    Q_D(ModemLocationInterface);
    d->setBackendObject(backendObject);
    makeConnections( backendObject );
}

Solid::Control::ModemLocationInterface::ModemLocationInterface(const ModemLocationInterface &locationinterface)
    : ModemInterface(*new ModemLocationInterfacePrivate(this), locationinterface)
{
    Q_D(ModemLocationInterface);
    d->setBackendObject(locationinterface.d_ptr->backendObject());
    makeConnections( locationinterface.d_ptr->backendObject() );
}

Solid::Control::ModemLocationInterface::~ModemLocationInterface()
{
}

void Solid::Control::ModemLocationInterface::slotLocationChanged(const LocationInformationMap & location)
{
    emit locationChanged(location);
}

void Solid::Control::ModemLocationInterface::makeConnections(QObject * source)
{
    if (source) {
        QObject::connect(source, SIGNAL(capabilitiesChanged(Solid::Control::ModemLocationInterface::Capability)),
                this, SIGNAL(capabilitiesChanged(Solid::Control::ModemLocationInterface::Capability)));
        QObject::connect(source, SIGNAL(enabledChanged(bool)),
                this, SIGNAL(enabledChanged(bool)));
        QObject::connect(source, SIGNAL(signalsLocationChanged(bool)),
                this, SIGNAL(signalsLocationChanged(bool)));
        QObject::connect(source, SIGNAL(locationChanged(LocationInformationMap)),
                this, SIGNAL(slotLocationChanged(LocationInformationMap)));
    }
}

void Solid::Control::ModemLocationInterfacePrivate::setBackendObject(QObject *object)
{
    ModemInterfacePrivate::setBackendObject(object);
}

void Solid::Control::ModemLocationInterface::_k_destroyed(QObject *object)
{
    Q_UNUSED(object);
}
