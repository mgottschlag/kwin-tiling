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

#ifndef SOLID_CONTROL_MODEMLOCATIONINTERFACE_H
#define SOLID_CONTROL_MODEMLOCATIONINTERFACE_H

#include "modeminterface.h"

namespace Solid
{
namespace Control
{
    class ModemLocationInterfacePrivate;

    class SOLIDCONTROL_EXPORT ModemLocationInterface: public ModemInterface
    {
        Q_OBJECT
        Q_ENUMS(Capability)
        Q_DECLARE_PRIVATE(ModemLocationInterface)

    public:

        enum Capability { Unknown = 0x0, GpsNmea = 0x1, GpsLacCi = 0x2 };

        typedef QList<QMap<Capability, QVariant> > LocationInformationMap;

        ModemLocationInterface(QObject *backendObject = 0);

        ModemLocationInterface(const ModemLocationInterface &locationIface);

        virtual ~ModemLocationInterface();

        void enableLocation(const bool enable, const bool signalLocation) const;

        LocationInformationMap getLocation() const;

        Solid::Control::ModemLocationInterface::Capability getCapability() const;

        bool enabled() const;

        bool signalsLocation() const;

    Q_SIGNALS:
        void capabilitiesChanged(const Solid::Control::ModemLocationInterface::Capability capability);

        void enabledChanged(const bool enabled);

        void signalsLocationChanged(const bool signalsLocation);

        void locationChanged(const LocationInformationMap & location);

    protected Q_SLOTS:
       void slotLocationChanged(const LocationInformationMap & location);

    protected:
        /**
         * @internal
         */
        ModemLocationInterface(ModemLocationInterfacePrivate &dd, QObject *backendObject);

        /**
         * @internal
         */
        ModemLocationInterface(ModemLocationInterfacePrivate &dd, const ModemLocationInterface &locationinterface);

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
