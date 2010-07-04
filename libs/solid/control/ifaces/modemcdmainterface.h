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

#ifndef SOLID_IFACES_MODEMCDMAINTERFACE_H
#define SOLID_IFACES_MODEMCDMAINTERFACE_H

#include "../solid_control_export.h"
#include "../modemcdmainterface.h"
#include "modemmanagerinterface.h"

namespace Solid
{
namespace Control
{
namespace Ifaces
{
    class SOLIDCONTROLIFACES_EXPORT ModemCdmaInterface: virtual public ModemManagerInterface
    {
    public:
        virtual ~ModemCdmaInterface();

        virtual uint getSignalQuality() = 0;

        virtual QString getEsn() = 0;

        virtual Solid::Control::ModemCdmaInterface::ServingSystemType getServingSystem() = 0;

        virtual Solid::Control::ModemCdmaInterface::RegistrationStateResult getRegistrationState() = 0;

    protected:
    //Q_SIGNALS:
        /**
         * This signal is emitted when the signal quality of this network changes.
         *
         * @param signalQuality the new signal quality value for this network.
         */
        void signalQualityChanged(uint signalQuality);

        /**
         * This signal is emitted when the registration info this network changes
         *
         * @param registrationInfo the new registration info (status, operatorCode, operatorName)
         */
        void registrationStateChanged(const Solid::Control::ModemCdmaInterface::RegistrationState cdma_1x_state, Solid::Control::ModemCdmaInterface::RegistrationState evdo_state);
    };
} // Ifaces
} // Control
} // Solid

Q_DECLARE_INTERFACE(Solid::Control::Ifaces::ModemCdmaInterface, "org.kde.Solid.Control.Ifaces.ModemCdmaInterface/0.1")
#endif
