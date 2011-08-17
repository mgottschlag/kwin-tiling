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
#include "modemgsmcardinterface.h"
#include "modemgsmcardinterface_p.h"

#include <KDebug>

MMModemGsmCardInterfacePrivate::MMModemGsmCardInterfacePrivate(const QString &path, QObject *owner)
    : MMModemInterfacePrivate(path, owner), modemGsmCardIface(MMModemManager::DBUS_SERVICE, path, QDBusConnection::systemBus())
{
}

MMModemGsmCardInterface::MMModemGsmCardInterface(const QString & path, MMModemManager * manager, QObject * parent)
    : MMModemInterface(*new MMModemGsmCardInterfacePrivate(path, this), manager, parent)
{
    Q_D(MMModemGsmCardInterface);

    d->modemGsmCardIface.connection().connect(MMModemManager::DBUS_SERVICE,
        path, QLatin1String("org.freedesktop.DBus.Properties"),
        QLatin1String("MmPropertiesChanged"), QLatin1String("sa{sv}"),
        this, SLOT(propertiesChanged(QString,QVariantMap)));
}

MMModemGsmCardInterface::~MMModemGsmCardInterface()
{

}

void MMModemGsmCardInterface::propertiesChanged(const QString & interface, const QVariantMap & properties)
{
    kDebug(1441) << interface << properties.keys();

    if (interface == QString("org.freedesktop.ModemManager.Modem.Gsm.Card")) {
        QLatin1String supportedBands("SupportedBands");
        QLatin1String supportedModes("SupportedModes");

        QVariantMap::const_iterator it = properties.find(supportedBands);
        if ( it != properties.end()) {
            emit supportedBandsChanged((Solid::Control::ModemInterface::Band) it->toInt());
        }
        it = properties.find(supportedModes);
        if ( it != properties.end()) {
            emit supportedModesChanged((Solid::Control::ModemInterface::Mode) it->toInt());
        }
    }
}

QString MMModemGsmCardInterface::getImei()
{
    Q_D(MMModemGsmCardInterface);
    QDBusReply<QString> imei = d->modemGsmCardIface.GetImei();

    if (imei.isValid()) {
        return imei.value();
    }

    return QString();
}

QString MMModemGsmCardInterface::getImsi()
{
    Q_D(MMModemGsmCardInterface);
    QDBusReply<QString> imsi = d->modemGsmCardIface.GetImsi();

    if (imsi.isValid()) {
        return imsi.value();
    }

    return QString();
}

QDBusPendingReply<> MMModemGsmCardInterface::sendPuk(const QString & puk, const QString & pin)
{
    Q_D(MMModemGsmCardInterface);
    return d->modemGsmCardIface.SendPuk(puk, pin);
}

QDBusPendingReply<> MMModemGsmCardInterface::sendPin(const QString & pin)
{
    Q_D(MMModemGsmCardInterface);
    return d->modemGsmCardIface.SendPin(pin);
}

QDBusPendingReply<> MMModemGsmCardInterface::enablePin(const QString & pin, const bool enabled)
{
    Q_D(MMModemGsmCardInterface);
    return d->modemGsmCardIface.EnablePin(pin, enabled);
}

QDBusPendingReply<> MMModemGsmCardInterface::changePin(const QString & oldPin, const QString & newPin)
{
    Q_D(MMModemGsmCardInterface);
    return d->modemGsmCardIface.ChangePin(oldPin, newPin);
}

Solid::Control::ModemInterface::Band MMModemGsmCardInterface::getSupportedBands() const
{
    Q_D(const MMModemGsmCardInterface);
    return (Solid::Control::ModemInterface::Band) d->modemGsmCardIface.supportedBands();
}

Solid::Control::ModemInterface::Mode MMModemGsmCardInterface::getSupportedModes() const
{
    Q_D(const MMModemGsmCardInterface);
    return (Solid::Control::ModemInterface::Mode) d->modemGsmCardIface.supportedModes();
}

#include "modemgsmcardinterface.moc"

