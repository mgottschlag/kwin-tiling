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

#ifndef SOLID_CONTROL_MODEMGSMCARDINTERFACE_H
#define SOLID_CONTROL_MODEMGSMCARDINTERFACE_H

#include "modeminterface.h"

namespace Solid
{
namespace Control
{
    class ModemGsmCardInterfacePrivate;

    class SOLIDCONTROL_EXPORT ModemGsmCardInterface: public ModemInterface
    {
        Q_OBJECT
        Q_DECLARE_PRIVATE(ModemGsmCardInterface)

    public:

        ModemGsmCardInterface(QObject *backendObject = 0);

        ModemGsmCardInterface(const ModemGsmCardInterface &card);

        virtual ~ModemGsmCardInterface();

        QString getImei() const;

        QString getImsi() const;

        QDBusPendingReply<> sendPuk(const QString & puk, const QString & pin) const;

        QDBusPendingReply<> sendPin(const QString & pin) const;

        QDBusPendingReply<> enablePin(const QString & pin, const bool enabled) const;

        QDBusPendingReply<> changePin(const QString & oldPin, const QString & newPin) const;

        Solid::Control::ModemInterface::Band getSupportedBands() const;

        Solid::Control::ModemInterface::Mode getSupportedModes() const;

    Q_SIGNALS:
        void supportedBandsChanged(const Solid::Control::ModemInterface::Band band);

        void supportedModesChanged(const Solid::Control::ModemInterface::Mode modes);

    protected:
        /**
         * @internal
         */
        ModemGsmCardInterface(ModemGsmCardInterfacePrivate &dd, QObject *backendObject);

        /**
         * @internal
         */
        ModemGsmCardInterface(ModemGsmCardInterfacePrivate &dd, const ModemGsmCardInterface &card);

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
