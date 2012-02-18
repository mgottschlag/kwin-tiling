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
#include "modemgsmnetworkinterface.h"
#include "modemgsmnetworkinterface_p.h"

#include <KDebug>

MMModemGsmNetworkInterfacePrivate::MMModemGsmNetworkInterfacePrivate(const QString &path, QObject *owner)
    : MMModemInterfacePrivate(path, owner), modemGsmNetworkIface(MMModemManager::DBUS_SERVICE, path, QDBusConnection::systemBus())
{
}

MMModemGsmNetworkInterface::MMModemGsmNetworkInterface(const QString & path, MMModemManager * manager, QObject * parent)
    : MMModemInterface(*new MMModemGsmNetworkInterfacePrivate(path, this), manager, parent)
{
    Q_D(MMModemGsmNetworkInterface);

    d->modemGsmNetworkIface.connection().connect(MMModemManager::DBUS_SERVICE,
        path, QLatin1String("org.freedesktop.DBus.Properties"),
        QLatin1String("MmPropertiesChanged"), QLatin1String("sa{sv}"),
        this, SLOT(propertiesChanged(QString,QVariantMap)));

    connect( &d->modemGsmNetworkIface, SIGNAL(RegistrationInfo(uint,QString,QString)),
                this, SLOT(slotRegistrationInfoChanged(uint,QString,QString)));
    connect( &d->modemGsmNetworkIface, SIGNAL(SignalQuality(uint)),
                this, SIGNAL(signalQualityChanged(uint)));

    d->signalQuality = d->modemGsmNetworkIface.GetSignalQuality();
    d->registrationInfo = d->modemGsmNetworkIface.GetRegistrationInfo();
    d->accessTechnology = (Solid::Control::ModemInterface::AccessTechnology)d->modemGsmNetworkIface.accessTechnology();
    d->allowedMode = (Solid::Control::ModemInterface::AllowedMode)d->modemGsmNetworkIface.allowedMode();
}

MMModemGsmNetworkInterface::~MMModemGsmNetworkInterface()
{

}

void MMModemGsmNetworkInterface::propertiesChanged(const QString & interface, const QVariantMap & properties)
{
    kDebug(1441) << interface << properties.keys();

    if (interface == QString("org.freedesktop.ModemManager.Modem.Gsm.Network")) {
        QLatin1String allowedMode("AllowedMode");
        QLatin1String accessTechnology("AccessTechnology");

        QVariantMap::const_iterator it = properties.find(allowedMode);
        if ( it != properties.end()) {
            emit allowedModeChanged((Solid::Control::ModemInterface::AllowedMode) it->toInt());
        }
        it = properties.find(accessTechnology);
        if ( it != properties.end()) {
            emit accessTechnologyChanged((Solid::Control::ModemInterface::AccessTechnology) it->toInt());
        }
    }
}

void MMModemGsmNetworkInterface::slotSignalQualityChanged(uint signalQuality)
{
    Q_D(MMModemGsmNetworkInterface);
    d->signalQuality = signalQuality;
    emit signalQualityChanged(d->signalQuality);
}

void MMModemGsmNetworkInterface::slotRegistrationInfoChanged(uint status, const QString & operatorCode, const QString &operatorName)
{
    Q_D(MMModemGsmNetworkInterface);

    d->registrationInfo.status = (Solid::Control::ModemGsmNetworkInterface::RegistrationStatus) status;
    d->registrationInfo.operatorCode = operatorCode;
    d->registrationInfo.operatorName = operatorName;

    emit registrationInfoChanged(d->registrationInfo);
}

Solid::Control::ModemInterface::AllowedMode MMModemGsmNetworkInterface::getAllowedMode() const
{
    Q_D(const MMModemGsmNetworkInterface);
    return d->allowedMode;
}

Solid::Control::ModemInterface::AccessTechnology MMModemGsmNetworkInterface::getAccessTechnology() const
{
    Q_D(const MMModemGsmNetworkInterface);
    return d->accessTechnology;
}

void MMModemGsmNetworkInterface::registerToNetwork(const QString & networkId)
{
    Q_D(MMModemGsmNetworkInterface);
    d->modemGsmNetworkIface.Register(networkId);
}

ScanResultsType MMModemGsmNetworkInterface::scan()
{
    Q_D(MMModemGsmNetworkInterface);
    return d->modemGsmNetworkIface.Scan();
}

void MMModemGsmNetworkInterface::setApn(const QString & apn)
{
    Q_D(MMModemGsmNetworkInterface);
    d->modemGsmNetworkIface.SetApn(apn);
}

void MMModemGsmNetworkInterface::setBand(const Solid::Control::ModemInterface::Band band)
{
    Q_D(MMModemGsmNetworkInterface);
    d->modemGsmNetworkIface.SetBand(band);
}

Solid::Control::ModemInterface::Band MMModemGsmNetworkInterface::getBand()
{
    Q_D(MMModemGsmNetworkInterface);
    QDBusReply< uint > band = d->modemGsmNetworkIface.GetBand();

    if (band.isValid())
        return (Solid::Control::ModemInterface::Band) band.value();

    kDebug(1441) << "Error getting band setting info: " << band.error().name() << ": " << band.error().message();
    return Solid::Control::ModemInterface::UnknownBand;
}

RegistrationInfoType MMModemGsmNetworkInterface::getRegistrationInfo()
{
    Q_D(const MMModemGsmNetworkInterface);
    return d->registrationInfo;
}

uint MMModemGsmNetworkInterface::getSignalQuality()
{
    Q_D(const MMModemGsmNetworkInterface);
    return d->signalQuality;
}

void MMModemGsmNetworkInterface::setAllowedMode(const Solid::Control::ModemInterface::AllowedMode mode)
{
    Q_D(MMModemGsmNetworkInterface);
    d->modemGsmNetworkIface.SetAllowedMode(mode);
}

#include "modemgsmnetworkinterface.moc"

