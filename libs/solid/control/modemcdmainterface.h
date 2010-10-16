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

#ifndef SOLID_CONTROL_MODEMCDMAINTERFACE_H
#define SOLID_CONTROL_MODEMCDMAINTERFACE_H

#include "modeminterface.h"

namespace Solid
{
namespace Control
{
    class ModemCdmaInterfacePrivate;

    class SOLIDCONTROL_EXPORT ModemCdmaInterface: public ModemInterface
    {
        Q_OBJECT
        Q_DECLARE_PRIVATE(ModemCdmaInterface)

    public:

        ModemCdmaInterface(QObject *backendObject = 0);

        ModemCdmaInterface(const ModemCdmaInterface &network);

        virtual ~ModemCdmaInterface();

        enum BandClass { Unknown = 0x0, B800 = 0x01, B1900 = 0x2 };

        class ServingSystemType
        {
        public:
            BandClass bandClass;
            QString band;
            uint systemId;
        };

        enum RegistrationState { UnknownState = 0x0, Registered = 0x1, Home = 0x2, Roaming = 0x3 };

        class RegistrationStateResult
        {
        public:
            RegistrationState cdma_1x_state, evdo_state;
        };

        ModemInterface::Type type() const;

        uint getSignalQuality() const;

        QString getEsn() const;

        ServingSystemType getServingSystem() const;

        RegistrationStateResult getRegistrationState() const;

    Q_SIGNALS:
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
        void registrationStateChanged(const Solid::Control::ModemCdmaInterface::RegistrationState cdma_1x_state,
                                      const Solid::Control::ModemCdmaInterface::RegistrationState evdo_state);

    protected:
        /**
         * @internal
         */
        ModemCdmaInterface(ModemCdmaInterfacePrivate &dd, QObject *backendObject);

        /**
         * @internal
         */
        ModemCdmaInterface(ModemCdmaInterfacePrivate &dd, const ModemCdmaInterface &network);

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
