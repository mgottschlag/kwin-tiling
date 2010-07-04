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

#ifndef SOLID_CONTROL_MODEMGSMUSSDINTERFACE_H
#define SOLID_CONTROL_MODEMGSMUSSDINTERFACE_H

#include "modeminterface.h"

namespace Solid
{
namespace Control
{
    class ModemGsmUssdInterfacePrivate;

    class SOLIDCONTROL_EXPORT ModemGsmUssdInterface: public ModemInterface
    {
        Q_OBJECT
        Q_DECLARE_PRIVATE(ModemGsmUssdInterface)

    public:

        ModemGsmUssdInterface(QObject *backendObject = 0);

        ModemGsmUssdInterface(const ModemGsmUssdInterface &ussd);

        virtual ~ModemGsmUssdInterface();

        QString initiate(const QString & command) const;

        void respond(const QString response) const;

        void cancel() const;

        QString getState() const;

        QString getNetworkNotification() const;

        QString getNetworkRequest() const;

    Q_SIGNALS:
        void stateChanged(const QString & state);

        void networkNotificationChanged(const QString & networkNotification);

        void networkRequestChanged(const QString & networkRequest);

    protected Q_SLOTS:
        void slotStateChanged(const QString & state);

        void slotNetworkNotificationChanged(const QString & networkNotification);

        void slotNetworkRequestChanged(const QString & networkRequest);

    protected:
        /**
         * @internal
         */
        ModemGsmUssdInterface(ModemGsmUssdInterfacePrivate &dd, QObject *backendObject);

        /**
         * @internal
         */
        ModemGsmUssdInterface(ModemGsmUssdInterfacePrivate &dd, const ModemGsmUssdInterface &ussd);

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
