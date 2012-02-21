/*
Copyright 2008 Will Stephenson <wstephenson@kde.org>
Copyright 2010 Lamarque Souza <lamarque@gmail.com>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of
the License or (at your option) version 3 or any later version
accepted by the membership of KDE e.V. (or its successor approved
by the membership of KDE e.V.), which shall act as a proxy
defined in Section 14 of version 3 of the license.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef MM04_MODEMGSMNETWORKINTERFACE_P_H
#define MM04_MODEMGSMNETWORKINTERFACE_P_H

#include "modeminterface_p.h"
#include "dbus/mm-modem-gsm-networkinterface.h"

class MMModemGsmNetworkInterfacePrivate: public MMModemInterfacePrivate
{
public:
    MMModemGsmNetworkInterfacePrivate(const QString &path, QObject *owner);
    OrgFreedesktopModemManagerModemGsmNetworkInterface modemGsmNetworkIface;
    uint signalQuality;
    Solid::Control::ModemGsmNetworkInterface::RegistrationInfoType registrationInfo;
    Solid::Control::ModemInterface::AccessTechnology accessTechnology;
    Solid::Control::ModemInterface::AllowedMode allowedMode;
};

#endif

