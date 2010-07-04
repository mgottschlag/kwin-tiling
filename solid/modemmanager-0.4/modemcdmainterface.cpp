/*
Copyright 2008 Will Stephenson <wstephenson@kde.org>
Copyright 2010 Lamarque Souza <lamarque@gmail.com>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of
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

#include "manager.h"
#include "modemcdmainterface.h"
#include "modemcdmainterface_p.h"

#include <KDebug>

MMModemCdmaInterfacePrivate::MMModemCdmaInterfacePrivate(const QString &path, QObject *owner)
    : MMModemInterfacePrivate(path, owner), modemCdmaIface(MMModemManager::DBUS_SERVICE, path, QDBusConnection::systemBus())
{
}

MMModemCdmaInterface::MMModemCdmaInterface(const QString & path, MMModemManager * manager, QObject * parent)
    : MMModemInterface(*new MMModemCdmaInterfacePrivate(path, this), manager, parent)
{
    Q_D(MMModemCdmaInterface);

    connect( &d->modemCdmaIface, SIGNAL(RegistrationStateChanged(const Solid::Control::ModemCdmaInterface::RegistrationState,
                                                                 const Solid::Control::ModemCdmaInterface::RegistrationState)),
                this, SIGNAL(registrationStateChanged(const Solid::Control::ModemCdmaInterface::RegistrationState,
                                                      const Solid::Control::ModemCdmaInterface::RegistrationState)));
    connect( &d->modemCdmaIface, SIGNAL(SignalQuality(uint)),
                this, SIGNAL(signalQualityChanged(uint)));
}

MMModemCdmaInterface::~MMModemCdmaInterface()
{

}

uint MMModemCdmaInterface::getSignalQuality()
{
    Q_D(MMModemCdmaInterface);
    QDBusReply< uint > signalQuality = d->modemCdmaIface.GetSignalQuality();

    if (signalQuality.isValid())
        return signalQuality.value();

    kDebug(1441) << "Error getting signal quality: " << signalQuality.error().name() << ": " << signalQuality.error().message();
    return 0;
}

QString MMModemCdmaInterface::getEsn()
{
    Q_D(MMModemCdmaInterface);
    QDBusReply<QString> esn = d->modemCdmaIface.GetEsn();

    if (esn.isValid())
        return esn.value();

    kDebug(1441) << "Error getting ESN: " << esn.error().name() << ": " << esn.error().message();
    return QString();
}

ServingSystemType MMModemCdmaInterface::getServingSystem()
{
    Q_D(MMModemCdmaInterface);
    QDBusReply<ServingSystemType> servingSystem = d->modemCdmaIface.GetServingSystem();

    if (servingSystem.isValid())
        return servingSystem.value();

    kDebug(1441) << "Error getting serving system info: " << servingSystem.error().name() << ": " << servingSystem.error().message();
    return ServingSystemType();
}

RegistrationStateResult MMModemCdmaInterface::getRegistrationState()
{
    Q_D(MMModemCdmaInterface);
    QDBusReply<RegistrationStateResult> registrationState = d->modemCdmaIface.GetRegistrationState();

    if (registrationState.isValid())
        return registrationState.value();

    kDebug(1441) << "Error getting registration state: " << registrationState.error().name() << ": " << registrationState.error().message();
    return RegistrationStateResult();
}

#include "modemcdmainterface.moc"
