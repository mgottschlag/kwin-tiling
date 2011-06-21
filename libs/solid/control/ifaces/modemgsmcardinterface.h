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

#ifndef SOLID_IFACES_MODEMGSMCARDINTERFACE_H
#define SOLID_IFACES_MODEMGSMCARDINTERFACE_H

#include "../solid_control_export.h"
#include "../modemgsmcardinterface.h"
#include "modemmanagerinterface.h"

namespace Solid
{
namespace Control
{
namespace Ifaces
{
    class SOLIDCONTROLIFACES_EXPORT ModemGsmCardInterface: virtual public ModemManagerInterface
    {
    public:
        virtual ~ModemGsmCardInterface();

        virtual QString getImei() = 0;

        virtual QString getImsi() = 0;

        virtual QDBusPendingReply<> sendPuk(const QString & puk, const QString & pin) = 0;

        virtual QDBusPendingReply<> sendPin(const QString & pin) = 0;

        virtual QDBusPendingReply<> enablePin(const QString & pin, const bool enabled) = 0;

        virtual QDBusPendingReply<> changePin(const QString & oldPin, const QString & newPin) = 0;

        virtual Solid::Control::ModemInterface::Band getSupportedBands() const = 0;

        virtual Solid::Control::ModemInterface::Mode getSupportedModes() const = 0;

    protected:
    //Q_SIGNALS:
        void supportedBandsChanged(const Solid::Control::ModemInterface::Band band);

        void supportedModesChanged(const Solid::Control::ModemInterface::Mode modes);
    };
} // Ifaces
} // Control
} // Solid

Q_DECLARE_INTERFACE(Solid::Control::Ifaces::ModemGsmCardInterface, "org.kde.Solid.Control.Ifaces.ModemGsmCardInterface/0.1")
#endif
