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

#ifndef SOLID_IFACES_MODEMGSMHSOINTERFACE_H
#define SOLID_IFACES_MODEMGSMHSOINTERFACE_H

#include "../solid_control_export.h"
#include "../modemgsmhsointerface.h"
#include "modemmanagerinterface.h"

namespace Solid
{
namespace Control
{
namespace Ifaces
{
    class SOLIDCONTROLIFACES_EXPORT ModemGsmHsoInterface: virtual public ModemManagerInterface
    {
    public:
        virtual ~ModemGsmHsoInterface();

        virtual void authenticate(const QString & username, const QString & password) = 0;

    protected:
    //Q_SIGNALS:
        void hsoReceived(int index, bool complete);

        void completed(int index, bool completed);
    };
} // Ifaces
} // Control
} // Solid

Q_DECLARE_INTERFACE(Solid::Control::Ifaces::ModemGsmHsoInterface, "org.kde.Solid.Control.Ifaces.ModemGsmHsoInterface/0.1")
#endif
