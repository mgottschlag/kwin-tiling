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

#ifndef SOLID_CONTROL_IFACES_MODEMINTERFACE_H
#define SOLID_CONTROL_IFACES_MODEMINTERFACE_H

#include "../solid_control_export.h"
#include "../modeminterface.h"
#include <QtCore/QObject>

namespace Solid
{
namespace Control
{
namespace Ifaces
{
    class SOLIDCONTROLIFACES_EXPORT ModemInterface
    {
    public:
        virtual ~ModemInterface();

        /**
         * Retrieves the Unique Device Identifier (UDI).
         *
         * @returns the Unique Device Identifier of the current device
         */
        virtual QString udi() const = 0;

        virtual void enable(const bool enable) = 0;

        virtual void connectModem(const QString & number) = 0;

        virtual void connectModem(const QVariantMap & properties) = 0;

        virtual void disconnectModem() = 0;

        virtual Solid::Control::ModemInterface::Ip4ConfigType getIp4Config() = 0;

        virtual Solid::Control::ModemInterface::InfoType getInfo() = 0;

        virtual QVariantMap getStatus() = 0;

        /*
         * Properties
         */
        virtual QString device() const = 0;

        virtual QString masterDevice() const = 0;

        virtual QString driver() const = 0;

        virtual Solid::Control::ModemInterface::Type type() const = 0;

        virtual bool enabled() const = 0;

        virtual QString unlockRequired() const = 0;

        virtual Solid::Control::ModemInterface::Method ipMethod() const = 0;

    Q_SIGNALS:
        void deviceChanged(const QString & device);

        void masterDeviceChanged(const QString & masterDevice);

        void driverChanged(const QString & driver);

        void typeChanged(const Solid::Control::ModemInterface::Type type);

        void enabledChanged(const bool enabled);

        void unlockRequiredChanged(const QString & codeRequired);

        void ipMethodChanged(const Solid::Control::ModemInterface::Method ipMethod);
    };

} // Ifaces
} // Control
} // Solid

Q_DECLARE_INTERFACE(Solid::Control::Ifaces::ModemInterface, "org.kde.Solid.Control.Ifaces.ModemInterface/0.1")

#endif
