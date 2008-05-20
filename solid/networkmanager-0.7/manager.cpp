/*
Copyright 2008 Will Stephenson <wstephenson@kde.org>

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

#include "manager.h"
#include "manager_p.h"

#include <KDebug>
#include <NetworkManager.h>

#include "dbus/nm-deviceinterface.h"
#include "networkmanagerdefinitions.h"
#include "wirednetworkinterface.h"
#include "wirelessnetworkinterface.h"

const QString NMNetworkManager::DBUS_SERVICE(QString::fromLatin1("org.freedesktop.NetworkManager"));
const QString NMNetworkManager::DBUS_DAEMON_PATH(QString::fromLatin1("/org/freedesktop/NetworkManager"));
const QString NMNetworkManager::DBUS_USER_SETTINGS_PATH(QString::fromLatin1("org.freedesktop.NetworkManagerUserSettings"));
const QString NMNetworkManager::DBUS_SYSTEM_SETTINGS_PATH(QString::fromLatin1("org.freedesktop.NetworkManagerSystemSettings"));


NMNetworkManagerPrivate::NMNetworkManagerPrivate() : iface(NMNetworkManager::DBUS_SERVICE, "/org/freedesktop/NetworkManager", QDBusConnection::systemBus())
{
    kDebug() << NMNetworkManager::DBUS_SERVICE;
}

NMNetworkManager::NMNetworkManager(QObject * parent, const QStringList &) : d_ptr(new NMNetworkManagerPrivate)
{
    Q_D(NMNetworkManager);
    d->nmState = d->iface.state();
    d->isWirelessHardwareEnabled = d->iface.wirelessHardwareEnabled();
    d->isWirelessEnabled = d->iface.wirelessEnabled();
    connect( &d->iface, SIGNAL(DeviceAdded(const QDBusObjectPath &)),
                this, SLOT(deviceAdded(const QDBusObjectPath &)));
    connect( &d->iface, SIGNAL(DeviceRemoved(const QDBusObjectPath &)),
                this, SLOT(deviceRemoved(const QDBusObjectPath &)));
    connect( &d->iface, SIGNAL(PropertiesChanged(const QVariantMap &)),
                this, SLOT(propertiesChanged(const QVariantMap &)));
    connect( &d->iface, SIGNAL(StateChanged(uint)),
                this, SLOT(stateChanged(uint)));


    qDBusRegisterMetaType<QList<QDBusObjectPath> >();
    QDBusReply< QList <QDBusObjectPath> > deviceList = d->iface.GetDevices();
    if (deviceList.isValid())
    {
        kDebug(1441) << "Got device list";
        QList <QDBusObjectPath> devices = deviceList.value();
        foreach (QDBusObjectPath op, devices)
        {
            d->networkInterfaces.append(op.path());
            kDebug(1441) << "  " << op.path();
        }
    }
    else
        kDebug(1441) << "Error getting device list: " << deviceList.error().name() << ": " << deviceList.error().message();
}

NMNetworkManager::~NMNetworkManager()
{
    delete d_ptr;
}

Solid::Networking::Status NMNetworkManager::status() const
{
    Q_D(const NMNetworkManager);
    return convertNMState(d->nmState);
}

QStringList NMNetworkManager::networkInterfaces() const
{
    Q_D(const NMNetworkManager);
    return d->networkInterfaces;
}

QObject *NMNetworkManager::createNetworkInterface(const QString &uni)
{
    kDebug();
    OrgFreedesktopNetworkManagerDeviceInterface devIface(NMNetworkManager::DBUS_SERVICE, uni, QDBusConnection::systemBus());
    uint deviceType = devIface.deviceType();
    NMNetworkInterface * createdInterface = 0;
    switch ( deviceType ) {
        case DEVICE_TYPE_802_3_ETHERNET:
            createdInterface = new NMWiredNetworkInterface(uni, this, 0); // these are deleted by the frontent manager
            break;
        case DEVICE_TYPE_802_11_WIRELESS:
            createdInterface = new NMWirelessNetworkInterface(uni, this, 0);
            break;
        case DEVICE_TYPE_GSM:
        case DEVICE_TYPE_CDMA:
        default:
            break;
    }

    return createdInterface;
}

bool NMNetworkManager::isNetworkingEnabled() const
{
    Q_D(const NMNetworkManager);
    return !(NM_STATE_UNKNOWN == d->nmState || NM_STATE_ASLEEP == d->nmState);
}

bool NMNetworkManager::isWirelessEnabled() const
{
    Q_D(const NMNetworkManager);
    return d->isWirelessEnabled;
}

bool NMNetworkManager::isWirelessHardwareEnabled() const
{
    Q_D(const NMNetworkManager);
    return d->isWirelessHardwareEnabled;
}

void NMNetworkManager::activateConnection(const QString & interfaceUni, const QString & connectionUni, const QVariantMap & connectionParameters)
{
    Q_D(NMNetworkManager);
    QString serviceName = connectionUni.split(' ')[0];
    QString connectionPath = connectionUni.split(' ')[1];
    // ### FIXME find a better name for the parameter needed for NM 0.7
    QString extra_connection_parameter = connectionParameters.value("extra_connection_parameter").toString();
    if ( serviceName.isEmpty() || connectionPath.isEmpty() ) {
        return;
    }
    // TODO store error code
    d->iface.ActivateConnection(serviceName, QDBusObjectPath(connectionPath), QDBusObjectPath(interfaceUni), QDBusObjectPath(extra_connection_parameter));
}

void NMNetworkManager::deactivateConnection( const QString & activeConnectionPath )
{
    Q_D(NMNetworkManager);
    d->iface.DeactivateConnection(QDBusObjectPath(activeConnectionPath));
}

void NMNetworkManager::setNetworkingEnabled(bool enabled)
{
    Q_D(NMNetworkManager);
    d->iface.Sleep(!enabled);
}

void NMNetworkManager::setWirelessEnabled(bool enabled)
{
    Q_D(NMNetworkManager);
    d->iface.setWirelessEnabled(enabled);
}

void NMNetworkManager::deviceAdded(const QDBusObjectPath & objpath)
{
    kDebug(1441);
    Q_D(NMNetworkManager);
    d->networkInterfaces.append(objpath.path());
    emit networkInterfaceAdded(objpath.path());
}

void NMNetworkManager::deviceRemoved(const QDBusObjectPath & objpath)
{
    kDebug(1441);
    Q_D(NMNetworkManager);
    d->networkInterfaces.removeAll(objpath.path());
    emit networkInterfaceRemoved(objpath.path());
}

void NMNetworkManager::stateChanged(uint state)
{
    Q_D(NMNetworkManager);
    if ( d->nmState != state ) {
        d->nmState = state;
        emit statusChanged( convertNMState( state ) );
    }
}

void NMNetworkManager::propertiesChanged(const QVariantMap &properties)
{
#warning TODO NMNetworkManager::propertiesChanged implement
    kDebug();
}

Solid::Networking::Status NMNetworkManager::convertNMState(uint state)
{
    Solid::Networking::Status status;
    switch (state) {
        case NM_STATE_UNKNOWN:
        case NM_STATE_ASLEEP:
            status = Solid::Networking::Unknown;
            break;
        case NM_STATE_CONNECTING:
            status = Solid::Networking::Connecting;
            break;
        case NM_STATE_CONNECTED:
            status = Solid::Networking::Connected;
            break;
        case NM_STATE_DISCONNECTED:
            status = Solid::Networking::Unconnected;
            break;
    }
    return status;
}

#include "manager.moc"

