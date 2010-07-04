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

#ifndef SOLID_IFACES_MODEMGSMNETWORKINTERFACE_H
#define SOLID_IFACES_MODEMGSMNETWORKINTERFACE_H

#include "../solid_control_export.h"
#include "../modemgsmnetworkinterface.h"
#include "modemmanagerinterface.h"

namespace Solid
{
namespace Control
{
namespace Ifaces
{
    class SOLIDCONTROLIFACES_EXPORT ModemGsmNetworkInterface: virtual public ModemManagerInterface
    {
    public:
        virtual ~ModemGsmNetworkInterface();

        /**
         * Register the device to network.
         *
         * @param networkId the network ID to register. An empty string can be used to register to the home network.
         */
        virtual void registerToNetwork(const QString & networkId) = 0;

        /**
         * Scan for available networks.
         *
         * @return Found networks. It's an array of dictionaries (strings for both keys and values) with each
         * array element describing a mobile network found in the scan.
         */
        virtual Solid::Control::ModemGsmNetworkInterface::ScanResultsType scan() = 0;

        /**
         * Sets the Access Point Name (APN).
         *
         * @param apn the APN to set to.
         */
        virtual void setApn(const QString & apn) = 0;

        /**
         * Retrieves the current signal quality of the gsm connection.
         *
         * @return the signal quality as a percentage
         */
        virtual uint getSignalQuality() = 0;

        /**
         * Sets the band the device is allowed to use when connecting to a mobile network.
         *
         * @param band the desired band. Only one band may be specified, and may not be UNKNOWN.
         */
        virtual void setBand(const Solid::Control::ModemGsmNetworkInterface::Band band) = 0;

        /**
         * Returns the current band the device is using. (Note for plugin writers: returned value must not be ANY)
         *
         * @return the current band.
         */
        virtual Solid::Control::ModemGsmNetworkInterface::Band getBand() = 0;

        /**
         * Retrieves the current registration info.
         *
         * @return the registration info.
         */
        virtual Solid::Control::ModemGsmNetworkInterface::RegistrationInfoType getRegistrationInfo() = 0;

        /**
         * Set the access technologies a device is allowed to use when connecting to a mobile network.
         *
         * @param mode The allowed mode. The device may not support all modes;
         * see the org.freedesktop.ModemManager.Gsm.Card.SupportedModes property for allowed modes for each device.
         * All devices support the "ANY" flag.
         */
        virtual void setAllowedMode(const Solid::Control::ModemInterface::AllowedMode mode) = 0;

        /**
         * Retrieves the current allowed mode.
         *
         * @return the allowed mode.
         */
        virtual Solid::Control::ModemInterface::AllowedMode getAllowedMode() const = 0;

        /**
         * Retrieves the current used access technology.
         *
         * @return the access technology
         */
        virtual Solid::Control::ModemInterface::AccessTechnology getAccessTechnology() const = 0;

    protected:
    //Q_SIGNALS:
        /**
         * This signal is emitted when the registration info this network changes
         *
         * @param registrationInfo the new registration info (status, operatorCode, operatorName)
         */
        void registrationInfoChanged(const Solid::Control::ModemGsmNetworkInterface::RegistrationInfoType & registrationInfo);

        /**
         * This signal is emitted when the signal quality of this network changes.
         *
         * @param signalQuality the new signal quality value for this network.
         */
        void signalQualityChanged(uint signalQuality);

        /**
         * This signal is emitted when the AllowedMode property changes.
         *
         * @param mode the new allowed mode.
         */
        void gsmNetworkAllowedModeChanged(const Solid::Control::ModemGsmNetworkInterface::AllowedMode mode);

        /**
         * This signal is emitted when the AccessTechnology property changes.
         *
         * @param mode the new access technology used by the modem.
         */
        void gsmNetworkAccessTechnologyChanged(const Solid::Control::ModemGsmNetworkInterface::AccessTechnology tech);
    };
} // Ifaces
} // Control
} // Solid

Q_DECLARE_INTERFACE(Solid::Control::Ifaces::ModemGsmNetworkInterface, "org.kde.Solid.Control.Ifaces.ModemGsmNetworkInterface/0.1")
#endif
