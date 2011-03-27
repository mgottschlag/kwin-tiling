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

#ifndef SOLID_IFACES_MODEMLOCATIONINTERFACE_H
#define SOLID_IFACES_MODEMLOCATIONINTERFACE_H

#include "../solid_control_export.h"
#include "../modemlocationinterface.h"
#include "modemmanagerinterface.h"

namespace Solid
{
namespace Control
{
namespace Ifaces
{
    class SOLIDCONTROLIFACES_EXPORT ModemLocationInterface: virtual public ModemManagerInterface
    {
    public:
        virtual ~ModemLocationInterface();

        void enableLocation(const bool enable, const bool signalLocation) const;

        Solid::Control::ModemLocationInterface::LocationInformationMap getLocation() const;

        Solid::Control::ModemLocationInterface::Capability getCapability() const;

        bool enabled() const;

        bool signalsLocation();

    protected:
    //Q_SIGNALS:
        void capabilitiesChanged(const Solid::Control::ModemLocationInterface::Capability capability);

        void enabledChanged(const bool enabled);

        void signalsLocationChanged(const bool signalsLocation);

        void locationChanged(const Solid::Control::ModemLocationInterface::LocationInformationMap & location);
    };
} // Ifaces
} // Control
} // Solid

Q_DECLARE_INTERFACE(Solid::Control::Ifaces::ModemLocationInterface, "org.kde.Solid.Control.Ifaces.ModemLocationInterface/0.1")
#endif
