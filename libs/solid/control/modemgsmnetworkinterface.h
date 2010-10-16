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

#ifndef SOLID_CONTROL_MODEMGSMNETWORKINTERFACE_H
#define SOLID_CONTROL_MODEMGSMNETWORKINTERFACE_H

#include "modeminterface.h"

namespace Solid
{
namespace Control
{
    class ModemGsmNetworkInterfacePrivate;

    class SOLIDCONTROL_EXPORT ModemGsmNetworkInterface: public ModemInterface
    {
        Q_OBJECT
        Q_DECLARE_PRIVATE(ModemGsmNetworkInterface)

    public:

        ModemGsmNetworkInterface(QObject *backendObject = 0);

        ModemGsmNetworkInterface(const ModemGsmNetworkInterface &network);

        virtual ~ModemGsmNetworkInterface();

        /*
         * Each item list may include one or more of the following keys:
         *
         * "status": a number representing network availability status as defined in 3GPP TS 27.007
         *           section 7.3. e.g. "0" (unknown), "1" (available), "2" (current), or "3" (forbidden).
         *           This key will always be present.
         * "operator-long": long-format name of operator. If the name is unknown, this field should not be present.
         * "operator-short": short-format name of operator. If the name is unknown, this field should not be present.
         * "operator-num": mobile code of the operator. Returned in the format "MCCMNC",
         *                 where MCC is the three-digit ITU E.212 Mobile Country Code and MNC is the two- or
         *                 three-digit GSM Mobile Network Code. e.g. "31026" or "310260".
         * "access-tech": a number representing the access technology used by this mobile network as described
         *                in 3GPP TS 27.007 section 7.3. e.g. "0" (GSM), "1" (GSM Compact), "2" (UTRAN/UMTS), "3" (EDGE), etc.
         */
        typedef QList< QMap<QString,QString> > ScanResultsType;

        /* GSM registration code as defined in 3GPP TS 27.007 section 10.1.19. */
        enum RegistrationStatus {
            StatusIdle = 0x0, /* Not registered, not searching for new operator to register. */
            StatusHome, /* Registered on home network. */
            StatusSearching, /* Not registered, searching for new operator to register with. */
            StatusDenied, /* Registration denied. */
            StatusUnknown, /* Unknown registration status. */
            StatusRoaming /* Registered on a roaming network. */
        };

        class RegistrationInfoType
        {
        public:
            RegistrationStatus status; /* Mobile registration status as defined in 3GPP TS 27.007 section 10.1.19. */
            QString operatorCode, /* Current operator code of the operator to which the mobile is currently registered.
                                     Returned in the format "MCCMNC", where MCC is the three-digit ITU E.212 Mobile Country Code
                                     and MNC is the two- or three-digit GSM Mobile Network Code. If the MCC and MNC are not known
                                     or the mobile is not registered to a mobile network, this value should be a zero-length (blank)
                                     string. e.g. "31026" or "310260". */
                    operatorName /* Current operator name of the operator to which the mobile is currently registered.
                                    If the operator name is not knowon or the mobile is not registered to a mobile network,
                                    this value should be a zero-length (blank) string. */;
        };

        ModemInterface::Type type() const;

        /**
         * Register the device to network.
         *
         * @param networkId the network ID to register. An empty string can be used to register to the home network.
         */
        void registerToNetwork(const QString & networkId) const;

        /**
         * Scan for available networks.
         *
         * @return Found networks. It's an array of dictionaries (strings for both keys and values) with each
         * array element describing a mobile network found in the scan.
         */
        ScanResultsType scan() const;

        /**
         * Sets the Access Point Name (APN).
         *
         * @param apn the APN to set to.
         */
        void setApn(const QString & apn) const;

        /**
         * Retrieves the current signal quality of the gsm connection.
         *
         * @return the signal quality as a percentage
         */
        uint getSignalQuality() const;

        /**
         * Sets the band the device is allowed to use when connecting to a mobile network.
         *
         * @param band the desired band. Only one band may be specified, and may not be UNKNOWN.
         */
        void setBand(const Band band) const;

        /**
         * Returns the current band the device is using. (Note for plugin writers: returned value must not be ANY)
         *
         * @return the current band.
         */
        Band getBand() const;

        /**
         * Retrieves the current registration info.
         *
         * @return the registration info.
         */
        RegistrationInfoType getRegistrationInfo() const;

        /**
         * Set the access technologies a device is allowed to use when connecting to a mobile network.
         *
         * @param mode The allowed mode. The device may not support all modes;
         * see the org.freedesktop.ModemManager.Gsm.Card.SupportedModes property for allowed modes for each device.
         * All devices support the "ANY" flag.
         */
        void setAllowedMode(const ModemInterface::AllowedMode mode) const;

        /**
         * Retrieves the current allowed mode.
         *
         * @return the allowed mode.
         */
        ModemInterface::AllowedMode getAllowedMode() const;

        /**
         * Retrieves the current used access technology.
         *
         * @return the access technology
         */
        ModemInterface::AccessTechnology getAccessTechnology() const;

    Q_SIGNALS:
        /**
         * This signal is emitted when the registration info this network changes
         *
         * @param registrationInfo the new registration info (status, operatorCode, operatorName)
         */
        void registrationInfoChanged(const Solid::Control::ModemGsmNetworkInterface::RegistrationInfoType & registrationInfo) const;

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
        void allowedModeChanged(const Solid::Control::ModemInterface::AllowedMode mode);

        /**
         * This signal is emitted when the AccessTechnology property changes.
         *
         * @param mode the new access technology used by the modem.
         */
        void accessTechnologyChanged(const Solid::Control::ModemInterface::AccessTechnology tech);

    protected:
        /**
         * @internal
         */
        ModemGsmNetworkInterface(ModemGsmNetworkInterfacePrivate &dd, QObject *backendObject);

        /**
         * @internal
         */
        ModemGsmNetworkInterface(ModemGsmNetworkInterfacePrivate &dd, const ModemGsmNetworkInterface &network);

        void makeConnections(QObject * source);
    private Q_SLOTS:
        void _k_destroyed(QObject *object);
    private:
        friend class ModemManagerInterface;
        friend class ModemManagerInterfacePrivate;
    };
} // Control
} // Solid

#endif
