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

#ifndef SOLID_IFACES_MODEMGSMCONTACTSINTERFACE_H
#define SOLID_IFACES_MODEMGSMCONTACTSINTERFACE_H

#include "../solid_control_export.h"
#include "../modemgsmcontactsinterface.h"
#include "modemmanagerinterface.h"

namespace Solid
{
namespace Control
{
namespace Ifaces
{
    class SOLIDCONTROLIFACES_EXPORT ModemGsmContactsInterface: virtual public ModemManagerInterface
    {
    public:
        virtual ~ModemGsmContactsInterface();

        virtual int addContact(const QString & name, const QString & number) = 0;

        virtual void deleteContact(const int index) = 0;

        virtual Solid::Control::ModemGsmContactsInterface::ContactType get(const int index) = 0;

        virtual QList<Solid::Control::ModemGsmContactsInterface::ContactType> list() = 0;

        virtual QList<Solid::Control::ModemGsmContactsInterface::ContactType> find(const QString & pattern) = 0;

        virtual int getCount() = 0;

    protected:
    //Q_SIGNALS:
    };
} // Ifaces
} // Control
} // Solid

Q_DECLARE_INTERFACE(Solid::Control::Ifaces::ModemGsmContactsInterface, "org.kde.Solid.Control.Ifaces.ModemGsmContactsInterface/0.1")
#endif
