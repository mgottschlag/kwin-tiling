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
#include "modemgsmussdinterface.h"
#include "modemgsmussdinterface_p.h"

#include <KDebug>

MMModemGsmUssdInterfacePrivate::MMModemGsmUssdInterfacePrivate(const QString &path, QObject *owner)
    : MMModemInterfacePrivate(path, owner), modemGsmUssdIface(MMModemManager::DBUS_SERVICE, path, QDBusConnection::systemBus())
{
}

MMModemGsmUssdInterface::MMModemGsmUssdInterface(const QString & path, MMModemManager * manager, QObject * parent)
    : MMModemInterface(*new MMModemGsmUssdInterfacePrivate(path, this), manager, parent)
{
    Q_D(MMModemGsmUssdInterface);

    d->modemGsmUssdIface.connection().connect(MMModemManager::DBUS_SERVICE,
        path, QLatin1String("org.freedesktop.DBus.Properties"),
        QLatin1String("MmPropertiesChanged"), QLatin1String("sa{sv}"),
        this, SLOT(propertiesChanged(QString,QVariantMap)));
}

MMModemGsmUssdInterface::~MMModemGsmUssdInterface()
{

}

void MMModemGsmUssdInterface::propertiesChanged(const QString & interface, const QVariantMap & properties)
{
    kDebug(1441) << interface << properties.keys();

    if (interface == QString("org.freedesktop.ModemManager.Modem.Gsm.Ussd")) {
        QLatin1String state("State");
        QLatin1String networkNotification("NetworkNotification");
        QLatin1String networkRequest("NetworkRequest");

        QVariantMap::const_iterator it = properties.find(state);
        if ( it != properties.end()) {
            emit stateChanged(it->toString());
        }
        it = properties.find(networkNotification);
        if ( it != properties.end()) {
            emit networkNotificationChanged(it->toString());
        }
        it = properties.find(networkRequest);
        if ( it != properties.end()) {
            emit networkRequestChanged(it->toString());
        }
    }
}

QString MMModemGsmUssdInterface::initiate(const QString & command)
{
    Q_D(MMModemGsmUssdInterface);
    QDBusReply<QString> reply = d->modemGsmUssdIface.Initiate(command);

    if (reply.isValid()) {
        return reply.value();
    }
    return QString();
}

void MMModemGsmUssdInterface::respond(const QString response)
{
    Q_D(MMModemGsmUssdInterface);
    d->modemGsmUssdIface.Respond(response);
}

void MMModemGsmUssdInterface::cancel()
{
    Q_D(MMModemGsmUssdInterface);
    d->modemGsmUssdIface.Cancel();
}

QString MMModemGsmUssdInterface::getState()
{
    Q_D(const MMModemGsmUssdInterface);
    return d->modemGsmUssdIface.state();
}

QString MMModemGsmUssdInterface::getNetworkNotification()
{
    Q_D(const MMModemGsmUssdInterface);
    return d->modemGsmUssdIface.networkNotification();
}

QString MMModemGsmUssdInterface::getNetworkRequest()
{
    Q_D(const MMModemGsmUssdInterface);
    return d->modemGsmUssdIface.networkRequest();
}

#include "modemgsmussdinterface.moc"

