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

#ifndef SOLID_IFACES_MODEMMANAGERINTERFACE_H
#define SOLID_IFACES_MODEMMANAGERINTERFACE_H

#include "../solid_control_export.h"
#include "../modemmanagerinterface.h"
#include <QtCore/QObject>
#include <QtCore/QList>


namespace Solid
{
namespace Control
{
namespace Ifaces
{
    class SOLIDCONTROLIFACES_EXPORT ModemManagerInterface
    {
    public:
        /**
         * Destroys a ModemManagerInterface object.
         */
        virtual ~ModemManagerInterface();

    protected:
    //Q_SIGNALS:

    };
} // Ifaces
} // Control
} // Solid

Q_DECLARE_INTERFACE(Solid::Control::Ifaces::ModemManagerInterface, "org.kde.Solid.Control.Ifaces.ModemManagerInterface/0.1")
#endif
