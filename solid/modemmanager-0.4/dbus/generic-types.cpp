/* This file is part of the KDE project
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

#include "generic-types.h"

// Marshall the Solid::Control::Ip4ConfigType data into a D-BUS argument
QDBusArgument &operator << (QDBusArgument &arg,
    const Solid::Control::ModemInterface::Ip4ConfigType &config)
{
    arg.beginStructure();
    arg << config.ip4Address << config.dns1 << config.dns2 << config.dns3;
    arg.endStructure();
    return arg;
}

// Retrieve the Solid::Control::Ip4ConfigType data from the D-BUS argument
const QDBusArgument &operator >> (const QDBusArgument &arg,
    Solid::Control::ModemInterface::Ip4ConfigType &config)
{
    arg.beginStructure();
    arg >> config.ip4Address >> config.dns1 >> config.dns2 >> config.dns3;
    arg.endStructure();

    return arg;
}

// Marshall the Solid::Control::ModemManager::Modem::InfoType data into a D-BUS argument
QDBusArgument &operator << (QDBusArgument &arg,
    const Solid::Control::ModemInterface::InfoType &info)
{
    arg.beginStructure();
    arg << info.manufacturer << info.model << info.version;
    arg.endStructure();
    return arg;
}

// Retrieve the Solid::Control::ModemManager::Modem::InfoType data from the D-BUS argument
const QDBusArgument &operator >> (const QDBusArgument &arg,
    Solid::Control::ModemInterface::InfoType &info)
{
    arg.beginStructure();
    arg >> info.manufacturer >> info.model >> info.version;
    arg.endStructure();
    return arg;
}

// Marshall the Solid::Control::ModemCdmaInterface::ServingSystemType data into a D-BUS argument
QDBusArgument &operator << (QDBusArgument &arg,
    const Solid::Control::ModemCdmaInterface::ServingSystemType &servingSystem)
{
    arg.beginStructure();
    arg << servingSystem.bandClass << servingSystem.band << servingSystem.systemId;
    arg.endStructure();
    return arg;
}

// Retrieve the Solid::Control::ModemCdmaInterface::ServingSystemType data from the D-BUS argument
const QDBusArgument &operator >> (const QDBusArgument &arg,
    Solid::Control::ModemCdmaInterface::ServingSystemType &servingSystem)
{
    uint temp;
    arg.beginStructure();
    arg >> temp >> servingSystem.band >> servingSystem.systemId;
    servingSystem.bandClass = (Solid::Control::ModemCdmaInterface::BandClass) temp;
    arg.endStructure();
    return arg;
}

// Marshall the Solid::Control::ModemGsmContactsInterface::ContactType data into a D-BUS argument
QDBusArgument &operator << (QDBusArgument &arg,
    const Solid::Control::ModemGsmContactsInterface::ContactType &contact)
{
    arg.beginStructure();
    arg << contact.index << contact.name << contact.number;
    arg.endStructure();
    return arg;
}

// Retrieve the Solid::Control::ModemGsmContactsInterface::ContactType data from the D-BUS argument
const QDBusArgument &operator >> (const QDBusArgument &arg,
    Solid::Control::ModemGsmContactsInterface::ContactType &contact)
{
    arg.beginStructure();
    arg >> contact.index >> contact.name >> contact.number;
    arg.endStructure();
    return arg;
}

// Marshall the RegistrationInfoType data into a D-BUS argument
QDBusArgument &operator << (QDBusArgument &arg,
    const Solid::Control::ModemGsmNetworkInterface::RegistrationInfoType &info)
{
    uint temp;

    temp = (uint) info.status;
    arg.beginStructure();
    arg << temp << info.operatorCode << info.operatorName;
    arg.endStructure();
    return arg;
}

// Retrieve the RegistrationInfoType data from the D-BUS argument
const QDBusArgument &operator >> (const QDBusArgument &arg,
    Solid::Control::ModemGsmNetworkInterface::RegistrationInfoType &info)
{
    uint temp;

    arg.beginStructure();
    arg >> temp >> info.operatorCode >> info.operatorName;
    info.status = (Solid::Control::ModemGsmNetworkInterface::RegistrationStatus) temp;
    arg.endStructure();
    return arg;
}
