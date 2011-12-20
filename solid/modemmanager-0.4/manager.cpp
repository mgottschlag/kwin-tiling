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
#include "manager_p.h"
#include "modemcdmainterface.h"
#include "modemgsmcardinterface.h"
#include "modemgsmcontactsinterface.h"
#include "modemgsmnetworkinterface.h"
#include "modemgsmsmsinterface.h"
#include "modemgsmhsointerface.h"
#include "modemgsmussdinterface.h"

#include <KDebug>

const QString MMModemManager::DBUS_SERVICE(QString::fromLatin1("org.freedesktop.ModemManager"));
const QString MMModemManager::DBUS_DAEMON_PATH(QString::fromLatin1("/org/freedesktop/ModemManager"));

MMModemManagerPrivate::MMModemManagerPrivate() : iface(MMModemManager::DBUS_SERVICE, "/org/freedesktop/ModemManager", QDBusConnection::systemBus())
{
    kDebug(1441) << MMModemManager::DBUS_SERVICE;
}

MMModemManager::MMModemManager(QObject * parent, const QVariantList &)
    : Solid::Control::Ifaces::ModemManager(parent)
{
    qDBusRegisterMetaType<QList<QDBusObjectPath> >();
    registerModemManagerTypes();
    d_ptr = new MMModemManagerPrivate;
    Q_D(MMModemManager);

    // TODO: determinate ModemManager initial state: Connected (MM running), Unknown (MM not running).
    d->mmState = Solid::Networking::Connected;

    connect( &d->iface, SIGNAL(DeviceAdded(QDBusObjectPath)),
                  this, SLOT(deviceAdded(QDBusObjectPath)));
    connect( &d->iface, SIGNAL(DeviceRemoved(QDBusObjectPath)),
                  this, SLOT(deviceRemoved(QDBusObjectPath)));

    d->iface.connection().connect(QLatin1String("org.freedesktop.DBus"),
            QLatin1String("/org/freedesktop/DBus"), QLatin1String("org.freedesktop.DBus"),
            QLatin1String("NameOwnerChanged"), QLatin1String("sss"),
            this, SLOT(nameOwnerChanged(QString,QString,QString)));

    QDBusReply< QList<QDBusObjectPath> > deviceList = d->iface.EnumerateDevices();
    if (deviceList.isValid())
    {
        QList <QDBusObjectPath> devices = deviceList.value();
        foreach (const QDBusObjectPath &op, devices)
        {
            d->modemInterfaces.append(op.path());
        }
    }
}

MMModemManager::~MMModemManager()
{
    delete d_ptr;
}

Solid::Networking::Status MMModemManager::status() const
{
    Q_D(const MMModemManager);
    return d->mmState;
}

QStringList MMModemManager::modemInterfaces() const
{
    Q_D(const MMModemManager);
    return d->modemInterfaces;
}

QObject *MMModemManager::createModemInterface(const QString &udi, const Solid::Control::ModemInterface::GsmInterfaceType ifaceType)
{
    kDebug(1441);
    OrgFreedesktopModemManagerModemInterface modemIface(MMModemManager::DBUS_SERVICE, udi, QDBusConnection::systemBus());
    uint modemType = modemIface.type();
    MMModemInterface * createdInterface = 0;
    switch ( modemType ) {
        case Solid::Control::ModemInterface::GsmType:
            switch (ifaceType) {
                case Solid::Control::ModemInterface::GsmCard:
                    createdInterface = new MMModemGsmCardInterface(udi, this, 0); // these are deleted by the frontend manager
                break;
                case Solid::Control::ModemInterface::GsmContacts:
                    createdInterface = new MMModemGsmContactsInterface(udi, this, 0); // these are deleted by the frontend manager
                break;
                case Solid::Control::ModemInterface::GsmNetwork:
                    createdInterface = new MMModemGsmNetworkInterface(udi, this, 0); // these are deleted by the frontend manager
                break;
                case Solid::Control::ModemInterface::GsmSms:
                    createdInterface = new MMModemGsmSmsInterface(udi, this, 0); // these are deleted by the frontend manager
                break;
                case Solid::Control::ModemInterface::GsmHso:
                    createdInterface = new MMModemGsmHsoInterface(udi, this, 0); // these are deleted by the frontend manager
                break;
                case Solid::Control::ModemInterface::GsmUssd:
                    createdInterface = new MMModemGsmUssdInterface(udi, this, 0); // these are deleted by the frontend manager
                break;
                case Solid::Control::ModemInterface::NotGsm: // to prevent compilation warning
                break;
            }
            break;
        case Solid::Control::ModemInterface::CdmaType:
            createdInterface = new MMModemCdmaInterface(udi, this, 0); // these are deleted by the frontend manager
            break;
        default:
            kDebug(1441) << "Can't create object of type " << modemType;
            break;
    }

    return createdInterface;
}

void MMModemManager::deviceAdded(const QDBusObjectPath & objpath)
{
    kDebug(1441);
    Q_D(MMModemManager);
    d->modemInterfaces.append(objpath.path());
    emit modemInterfaceAdded(objpath.path());
}

void MMModemManager::deviceRemoved(const QDBusObjectPath & objpath)
{
    kDebug(1441);
    Q_D(MMModemManager);
    d->modemInterfaces.removeAll(objpath.path());
    emit modemInterfaceRemoved(objpath.path());
}

void MMModemManager::stateChanged(Solid::Networking::Status state)
{
    Q_D(MMModemManager);
    if ( d->mmState != state ) {
        d->mmState = state;
        emit statusChanged( state );
    }
}

void MMModemManager::nameOwnerChanged(QString name, QString oldOwner, QString newOwner)
{
    Q_D(MMModemManager);

    if ( name == QLatin1String("org.freedesktop.ModemManager") ) {
        kDebug(1441) << "name: " << name << ", old owner: " << oldOwner << ", new owner: " << newOwner;
        if ( oldOwner.isEmpty() && !newOwner.isEmpty() ) {
            stateChanged(Solid::Networking::Connected);
        }
        if ( !oldOwner.isEmpty() && newOwner.isEmpty() ) {
            // ModemManager stopped, set status Unknown for safety
            stateChanged(Solid::Networking::Unknown);
            d->modemInterfaces.clear();
        }
    }
}

#include "manager.moc"

