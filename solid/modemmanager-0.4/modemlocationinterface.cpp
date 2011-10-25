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
#include "modemlocationinterface.h"
#include "modemlocationinterface_p.h"

#include <KDebug>

MMModemLocationInterfacePrivate::MMModemLocationInterfacePrivate(const QString &path, QObject *owner)
    : MMModemInterfacePrivate(path, owner), modemLocationIface(MMModemManager::DBUS_SERVICE, path, QDBusConnection::systemBus())
{
}

MMModemLocationInterface::MMModemLocationInterface(const QString & path, MMModemManager * manager, QObject * parent)
    : MMModemInterface(*new MMModemLocationInterfacePrivate(path, this), manager, parent)
{
    Q_D(const MMModemLocationInterface);
    d->modemLocationIface.connection().connect(MMModemManager::DBUS_SERVICE,
        path, QLatin1String("org.freedesktop.DBus.Properties"),
        QLatin1String("MmPropertiesChanged"), QLatin1String("sa{sv}"),
        this, SLOT(propertiesChanged(QString,QVariantMap)));
}

MMModemLocationInterface::~MMModemLocationInterface()
{
}

void MMModemLocationInterface::propertiesChanged(const QString & interface, const QVariantMap & properties)
{
    kDebug(1441) << interface << properties.keys();

    if (interface == QString("org.freedesktop.ModemManager.Modem.Location")) {
        QLatin1String capabilities("Capabilities");
        QLatin1String enabled("Enabled");
        QLatin1String signalsLocation("SignalsLocation");
        QLatin1String location("Location");

        QVariantMap::const_iterator it = properties.find(capabilities);
        if ( it != properties.end()) {
            emit capabilitiesChanged((Solid::Control::ModemLocationInterface::Capability)it->toInt());
        }
        it = properties.find(enabled);
        if ( it != properties.end()) {
            emit enabledChanged(it->toBool());
        }
        it = properties.find(signalsLocation);
        if ( it != properties.end()) {
            emit signalsLocationChanged(it->toBool());
        }
        it = properties.find(location);
        if ( it != properties.end()) {
            QVariant v = it.value();
            LocationInformationMap map;
            if (v.canConvert<LocationInformationMap>()) {
                map = v.value<LocationInformationMap>();
            } else {
                kDebug(1441) << "Error converting LocationInformationMap property";
            }
            emit locationChanged(map);
        }
    }
}

void MMModemLocationInterface::enableLocation(const bool enable, const bool signalLocation)
{
    Q_D(MMModemLocationInterface);
    d->modemLocationIface.Enable(enable, signalLocation);
}

Solid::Control::ModemLocationInterface::LocationInformationMap MMModemLocationInterface::getLocation()
{
    Q_D(MMModemLocationInterface);
    QDBusReply<LocationInformationMap> location = d->modemLocationIface.GetLocation();

    if (location.isValid()) {
        return location.value();
    }

    return LocationInformationMap();
}

Solid::Control::ModemLocationInterface::Capability MMModemLocationInterface::getCapability() const
{
    Q_D(const MMModemLocationInterface);
    return (Solid::Control::ModemLocationInterface::Capability) d->modemLocationIface.capabilities();
}

bool MMModemLocationInterface::enabled() const
{
    Q_D(const MMModemLocationInterface);
    return d->modemLocationIface.enabled();
}

bool MMModemLocationInterface::signalsLocation() const
{
    Q_D(const MMModemLocationInterface);
    return d->modemLocationIface.signalsLocation();
}

#include "modemlocationinterface.moc"

