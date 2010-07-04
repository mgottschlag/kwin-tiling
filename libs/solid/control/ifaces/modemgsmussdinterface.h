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

#ifndef SOLID_IFACES_MODEMGSMUSSDINTERFACE_H
#define SOLID_IFACES_MODEMGSMUSSDINTERFACE_H

#include "../solid_control_export.h"
#include "../modemgsmussdinterface.h"
#include "modemmanagerinterface.h"

namespace Solid
{
namespace Control
{
namespace Ifaces
{
    class SOLIDCONTROLIFACES_EXPORT ModemGsmUssdInterface: virtual public ModemManagerInterface
    {
    public:
        virtual ~ModemGsmUssdInterface();

        virtual QString initiate(const QString & command) = 0;

        virtual void respond(const QString response) = 0;

        virtual void cancel() = 0;

        virtual QString getState() = 0;

        virtual QString getNetworkNotification() = 0;

        virtual QString getNetworkRequest() = 0;

    protected:
    //Q_SIGNALS:
        void stateChanged(const QString & state);

        void networkNotificationChanged(const QString & networkNotification);

        void networkRequestChanged(const QString & networkRequest);
    };
} // Ifaces
} // Control
} // Solid

Q_DECLARE_INTERFACE(Solid::Control::Ifaces::ModemGsmUssdInterface, "org.kde.Solid.Control.Ifaces.ModemGsmUssdInterface/0.1")
#endif
