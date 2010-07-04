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

#ifndef MODEMMANAGER_TYPES_H
#define MODEMMANAGER_TYPES_H

#include <QMetaType>
#include <QDBusArgument>
#include <QtDBus/QtDBus>

#include <solid/control/modemmanager.h>
#include <solid/control/modemmanagerinterface.h>
#include <solid/control/modemlocationinterface.h>
#include <solid/control/modemcdmainterface.h>
#include <solid/control/modemgsmcontactsinterface.h>
#include <solid/control/modemgsmnetworkinterface.h>
#include <solid/control/networkipv4config.h>

typedef Solid::Control::ModemLocationInterface::LocationInformationMap LocationInformationMap;
typedef Solid::Control::ModemCdmaInterface::ServingSystemType ServingSystemType;
typedef Solid::Control::ModemCdmaInterface::RegistrationStateResult RegistrationStateResult;
typedef Solid::Control::ModemInterface::Ip4ConfigType Ip4ConfigType;
typedef Solid::Control::ModemInterface::InfoType InfoType;
typedef Solid::Control::ModemGsmContactsInterface::ContactType ContactType;
typedef Solid::Control::ModemGsmContactsInterface::ContactTypeList ContactTypeList;
typedef Solid::Control::ModemGsmNetworkInterface::ScanResultsType ScanResultsType;
typedef Solid::Control::ModemGsmNetworkInterface::RegistrationInfoType RegistrationInfoType;
typedef QList<QVariantMap> QVariantMapList;

Q_DECLARE_METATYPE(Solid::Control::ModemInterface::Ip4ConfigType)
Q_DECLARE_METATYPE(Solid::Control::ModemInterface::InfoType)
Q_DECLARE_METATYPE(Solid::Control::ModemLocationInterface::LocationInformationMap)
Q_DECLARE_METATYPE(Solid::Control::ModemCdmaInterface::ServingSystemType)
Q_DECLARE_METATYPE(Solid::Control::ModemCdmaInterface::RegistrationStateResult)
Q_DECLARE_METATYPE(Solid::Control::ModemGsmContactsInterface::ContactType)
Q_DECLARE_METATYPE(Solid::Control::ModemGsmContactsInterface::ContactTypeList)
Q_DECLARE_METATYPE(Solid::Control::ModemGsmNetworkInterface::ScanResultsType)
Q_DECLARE_METATYPE(Solid::Control::ModemGsmNetworkInterface::RegistrationInfoType)
Q_DECLARE_METATYPE(QList<QVariantMap>)

// Solid::Control::ModemManager::Modem::Ip4Configtype
QDBusArgument &operator << (QDBusArgument &arg,
    const Solid::Control::ModemInterface::Ip4ConfigType &config);

const QDBusArgument &operator >> (const QDBusArgument &arg,
    Solid::Control::ModemInterface::Ip4ConfigType &config);

// Solid::Control::ModemManager::Modem::InfoType
QDBusArgument &operator << (QDBusArgument &arg,
    const Solid::Control::ModemInterface::InfoType &info);

const QDBusArgument &operator >> (const QDBusArgument &arg,
    Solid::Control::ModemInterface::InfoType &info);

// Solid::Control::ModemCdmaInterface::ServingSystemType
QDBusArgument &operator << (QDBusArgument &arg,
    const Solid::Control::ModemCdmaInterface::ServingSystemType &servingSystem);

const QDBusArgument &operator >> (const QDBusArgument &arg,
    Solid::Control::ModemCdmaInterface::ServingSystemType &servingSystem);

// Solid::Control::ModemGsmContactsInterface::ContactType
QDBusArgument &operator << (QDBusArgument &arg,
    const Solid::Control::ModemGsmContactsInterface::ContactType &contact);

const QDBusArgument &operator >> (const QDBusArgument &arg,
    Solid::Control::ModemGsmContactsInterface::ContactType &contact);

// Solid::Control::ModemManager::Modem::Gsm::Network::RegistrationInfoType
QDBusArgument &operator << (QDBusArgument &arg,
    const Solid::Control::ModemGsmNetworkInterface::RegistrationInfoType &info);

const QDBusArgument &operator >> (const QDBusArgument &arg,
    Solid::Control::ModemGsmNetworkInterface::RegistrationInfoType &info);

inline void registerModemManagerTypes() {
        qDBusRegisterMetaType<Solid::Control::ModemInterface::Ip4ConfigType>();
        qDBusRegisterMetaType<Solid::Control::ModemInterface::InfoType>();
        qDBusRegisterMetaType<Solid::Control::ModemCdmaInterface::ServingSystemType>();
        qDBusRegisterMetaType<Solid::Control::ModemGsmContactsInterface::ContactType>();
        qDBusRegisterMetaType<Solid::Control::ModemGsmNetworkInterface::RegistrationInfoType>();
}

#endif // MODEMMANAGER_TYPES_H
