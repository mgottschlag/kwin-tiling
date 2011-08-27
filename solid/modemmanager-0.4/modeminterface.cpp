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

#include <KDebug>

#include "manager.h"
#include "modeminterface.h"
#include "modeminterface_p.h"

MMModemInterfacePrivate::MMModemInterfacePrivate( const QString & path, QObject * owner ) : modemIface(MMModemManager::DBUS_SERVICE, path, QDBusConnection::systemBus()), modemSimpleIface(MMModemManager::DBUS_SERVICE, path, QDBusConnection::systemBus()), udi(path), manager(0)
{
    Q_UNUSED(owner);
}

MMModemInterfacePrivate::~MMModemInterfacePrivate()
{

}

MMModemInterface::MMModemInterface(const QString & path, MMModemManager * manager, QObject * parent) : QObject(parent), d_ptr(new MMModemInterfacePrivate(path, this))
{
    Q_D(MMModemInterface);
    init();
    d->manager = manager;
}

MMModemInterface::MMModemInterface(MMModemInterfacePrivate & dd, MMModemManager * manager, QObject * parent) : QObject(parent), d_ptr(&dd)
{
    Q_D(MMModemInterface);
    init();
    d->manager = manager;
}

MMModemInterface::~MMModemInterface()
{
    delete d_ptr;
}

void MMModemInterface::init()
{
    Q_D(MMModemInterface);
    d->device = d->modemIface.device();
    d->masterDevice = d->modemIface.masterDevice();
    d->driver = d->modemIface.driver();
    d->type = (Solid::Control::ModemInterface::Type) d->modemIface.type();
    d->enabled = d->modemIface.enabled();
    d->unlockRequired = d->modemIface.unlockRequired();
    d->ipMethod = (Solid::Control::ModemInterface::Method) d->modemIface.ipMethod();

    d->modemIface.connection().connect(MMModemManager::DBUS_SERVICE,
                                       d->udi, QLatin1String("org.freedesktop.DBus.Properties"),
                                       QLatin1String("MmPropertiesChanged"), QLatin1String("sa{sv}"),
                                       this, SLOT(propertiesChanged(QString,QVariantMap)));
}

QString MMModemInterface::udi() const
{
    Q_D(const MMModemInterface);
    return d->udi;
}


/*** From org.freedesktop.ModemManager.Modem ***/

void MMModemInterface::enable(const bool enable)
{
    Q_D(MMModemInterface);
    d->modemIface.Enable(enable);
}

void MMModemInterface::connectModem(const QString & number)
{
    Q_D(MMModemInterface);
    d->modemIface.Connect(number);
}

void MMModemInterface::disconnectModem()
{
    Q_D(MMModemInterface);
    d->modemIface.Disconnect();
}

Solid::Control::ModemInterface::Ip4ConfigType MMModemInterface::getIp4Config()
{
    Q_D(MMModemInterface);
    QDBusReply<Ip4ConfigType> config = d->modemIface.GetIP4Config();

    if (config.isValid()) {
        return config.value();
    }

    return Ip4ConfigType();
}

Solid::Control::ModemInterface::InfoType MMModemInterface::getInfo()
{
    Q_D(MMModemInterface);
    QDBusReply<InfoType> info = d->modemIface.GetInfo();

    if (info.isValid()) {
        return info.value();
    }

    return InfoType();
}

QString MMModemInterface::device() const
{
    Q_D(const MMModemInterface);
    return d->device;
}

QString MMModemInterface::masterDevice() const
{
    Q_D(const MMModemInterface);
    return d->masterDevice;
}

QString MMModemInterface::driver() const
{
    Q_D(const MMModemInterface);
    return d->driver;
}

Solid::Control::ModemInterface::Type MMModemInterface::type() const
{
    Q_D(const MMModemInterface);
    return d->type;
}

bool MMModemInterface::enabled() const
{
    Q_D(const MMModemInterface);
    return d->enabled;
}

QString MMModemInterface::unlockRequired() const
{
    Q_D(const MMModemInterface);
    return d->unlockRequired;
}

Solid::Control::ModemInterface::Method MMModemInterface::ipMethod() const
{
    Q_D(const MMModemInterface);
    return d->ipMethod;
}

void MMModemInterface::propertiesChanged(const QString & interface, const QVariantMap & properties)
{
    Q_D(MMModemInterface);
    kDebug(1441) << interface << properties.keys();

    if (interface == QString("org.freedesktop.ModemManager.Modem")) {
        QLatin1String device("Device");
        QLatin1String masterDevice("MasterDevice");
        QLatin1String driver("Driver");
        QLatin1String type("Type");
        QLatin1String enabled("Enabled");
        QLatin1String unlockRequired("UnlockRequired");
        QLatin1String ipMethod("IpMethod");

        QVariantMap::const_iterator it = properties.find(device);
        if ( it != properties.end()) {
            d->device = it->toString();
            emit deviceChanged(d->device);
        }
        it = properties.find(masterDevice);
        if ( it != properties.end()) {
            d->masterDevice = it->toString();
            emit masterDeviceChanged(d->masterDevice);
        }
        it = properties.find(driver);
        if ( it != properties.end()) {
            d->driver = it->toString();
            emit driverChanged(d->driver);
        }
        it = properties.find(type);
        if ( it != properties.end()) {
            d->type = (Solid::Control::ModemInterface::Type) it->toInt();
            emit typeChanged(d->type);
        }
        it = properties.find(enabled);
        if ( it != properties.end()) {
            d->enabled = it->toBool();
            emit enabledChanged(d->enabled);
        }
        it = properties.find(unlockRequired);
        if ( it != properties.end()) {
            d->unlockRequired = it->toString();
            emit unlockRequiredChanged(d->unlockRequired);
        }
        it = properties.find(ipMethod);
        if ( it != properties.end()) {
            d->ipMethod = (Solid::Control::ModemInterface::Method) it->toInt();
            emit ipMethodChanged(d->ipMethod);
        }
    }
}


/*** From org.freedesktop.ModemManager.Modem.Simple ***/

void MMModemInterface::connectModem(const QVariantMap & properties)
{
    Q_D(MMModemInterface);
    d->modemSimpleIface.Connect(properties);
}

QVariantMap MMModemInterface::getStatus()
{
    Q_D(MMModemInterface);

    QDBusReply<QVariantMap> status = d->modemSimpleIface.GetStatus();

    if (status.isValid()) {
        return status.value();
    }

    return QVariantMap();
}

#include "modeminterface.moc"

