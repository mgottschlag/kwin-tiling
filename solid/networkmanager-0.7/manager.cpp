/*
Copyright 2008 Will Stephenson <wstephenson@kde.org>

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

#include <KDebug>
#include <NetworkManager.h>

#include "dbus/nm-deviceinterface.h"
#include "networkmanagerdefinitions.h"
#include "wirednetworkinterface.h"
#include "wirelessnetworkinterface.h"
#include "networkgsminterface.h"
#include "networkcdmainterface.h"
#include "networkbtinterface.h"

const QString NMNetworkManager::DBUS_SERVICE(QString::fromLatin1("org.freedesktop.NetworkManager"));
const QString NMNetworkManager::DBUS_DAEMON_PATH(QString::fromLatin1("/org/freedesktop/NetworkManager"));
const QString NMNetworkManager::DBUS_USER_SETTINGS_PATH(QString::fromLatin1("org.freedesktop.NetworkManagerUserSettings"));
const QString NMNetworkManager::DBUS_SYSTEM_SETTINGS_PATH(QString::fromLatin1("org.freedesktop.NetworkManagerSystemSettings"));


NMNetworkManagerPrivate::NMNetworkManagerPrivate() : iface(NMNetworkManager::DBUS_SERVICE, "/org/freedesktop/NetworkManager", QDBusConnection::systemBus())
{
    kDebug(1441) << NMNetworkManager::DBUS_SERVICE;
}

NMNetworkManager::NMNetworkManager(QObject * parent, const QVariantList &) 
{
    qDBusRegisterMetaType<QList<QDBusObjectPath> >();
    d_ptr = new NMNetworkManagerPrivate;
    Q_D(NMNetworkManager);
    d->nmState = d->iface.state();
    d->isWirelessHardwareEnabled = d->iface.wirelessHardwareEnabled();
    d->isWirelessEnabled = d->iface.wirelessEnabled();
    d->isWwanHardwareEnabled = d->iface.wwanHardwareEnabled();
    d->isWwanEnabled = d->iface.wwanEnabled();
    QVariant netEnabled = d->iface.property("NetworkingEnabled");
    if (!netEnabled.isNull()) {
        d->isNetworkingEnabled = netEnabled.toBool();
        d->NetworkingEnabledPropertyAvailable = true;
    } else {
        d->isNetworkingEnabled = !(NM_STATE_UNKNOWN == d->nmState || NM_STATE_ASLEEP == d->nmState);
        d->NetworkingEnabledPropertyAvailable = false;
    }
    connect( &d->iface, SIGNAL(DeviceAdded(QDBusObjectPath)),
                this, SLOT(deviceAdded(QDBusObjectPath)));
    connect( &d->iface, SIGNAL(DeviceRemoved(QDBusObjectPath)),
                this, SLOT(deviceRemoved(QDBusObjectPath)));
    connect( &d->iface, SIGNAL(PropertiesChanged(QVariantMap)),
                this, SLOT(propertiesChanged(QVariantMap)));
    connect( &d->iface, SIGNAL(StateChanged(uint)),
                this, SLOT(stateChanged(uint)));

    d->iface.connection().connect(QLatin1String("org.freedesktop.DBus"),
            QLatin1String("/org/freedesktop/DBus"), QLatin1String("org.freedesktop.DBus"),
            QLatin1String("NameOwnerChanged"), QLatin1String("sss"),
            this, SLOT(nameOwnerChanged(QString,QString,QString)));

    QDBusReply< QList <QDBusObjectPath> > deviceList = d->iface.GetDevices();
    if (deviceList.isValid())
    {
        kDebug(1441) << "Device list";
        QList <QDBusObjectPath> devices = deviceList.value();
        foreach (const QDBusObjectPath &op, devices)
        {
            d->networkInterfaces.append(op.path());
            kDebug(1441) << "  " << op.path();
        }
    }
    else
        kDebug(1441) << "Error getting device list: " << deviceList.error().name() << ": " << deviceList.error().message();

    kDebug(1441) << "Active connections:";
    QList <QDBusObjectPath> activeConnections = d->iface.activeConnections();
    foreach (const QDBusObjectPath &ac, activeConnections)
    {
        d->activeConnections.append(ac.path());
        kDebug(1441) << "  " << ac.path();
    }
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
    kDebug(1441);
    OrgFreedesktopNetworkManagerDeviceInterface devIface(NMNetworkManager::DBUS_SERVICE, uni, QDBusConnection::systemBus());
    uint deviceType = devIface.deviceType();
    NMNetworkInterface * createdInterface = 0;
    switch ( deviceType ) {
        case NM_DEVICE_TYPE_ETHERNET:
            createdInterface = new NMWiredNetworkInterface(uni, this, 0); // these are deleted by the frontend manager
            break;
        case NM_DEVICE_TYPE_WIFI:
            createdInterface = new NMWirelessNetworkInterface(uni, this, 0);
            break;
        case NM_DEVICE_TYPE_GSM:
            createdInterface = new NMGsmNetworkInterface(uni, this, 0);
            break;
        case NM_DEVICE_TYPE_CDMA:
            createdInterface = new NMCdmaNetworkInterface(uni, this, 0);
            break;
            /*
        case NM_DEVICE_TYPE_SERIAL:
            createdInterface = new NMSerialNetworkInterface(uni, this, 0);
            break;
            */
        case NM_DEVICE_TYPE_BT:
            createdInterface = new NMBtNetworkInterface(uni, this, 0);
            break;

        default:
            kDebug(1441) << "Can't create object of type " << deviceType;
            break;
    }

    return createdInterface;
}

bool NMNetworkManager::isNetworkingEnabled() const
{
    Q_D(const NMNetworkManager);
    return d->isNetworkingEnabled;
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

bool NMNetworkManager::isWwanEnabled() const
{
    Q_D(const NMNetworkManager);
    return d->isWwanEnabled;
}

bool NMNetworkManager::isWwanHardwareEnabled() const
{
    Q_D(const NMNetworkManager);
    return d->isWwanHardwareEnabled;
}

void NMNetworkManager::activateConnection(const QString & interfaceUni, const QString & connectionUni, const QVariantMap & connectionParameters)
{
    Q_D(NMNetworkManager);
    QString serviceName = connectionUni.section(' ', 0, 0);
    QString connectionPath = connectionUni.section(' ', 1, 1);
    // ### FIXME find a better name for the parameter needed for NM 0.7
    QString extra_connection_parameter = connectionParameters.value("extra_connection_parameter").toString();
    if (extra_connection_parameter.isEmpty()) {
        extra_connection_parameter = QLatin1String("/");
    }
    if ( serviceName.isEmpty() || connectionPath.isEmpty() ) {
        return;
    }
    // TODO store error code
    QDBusObjectPath connPath(connectionPath);
    QDBusObjectPath interfacePath(interfaceUni);
    kDebug(1441) << "Activating connection" << connPath.path() << "on service" << serviceName << "on interface" << interfacePath.path() << "with extra" << extra_connection_parameter;
    d->iface.ActivateConnection(serviceName, connPath, interfacePath, QDBusObjectPath(extra_connection_parameter));
}

void NMNetworkManager::deactivateConnection( const QString & activeConnectionPath )
{
    Q_D(NMNetworkManager);
    d->iface.DeactivateConnection(QDBusObjectPath(activeConnectionPath));
}

void NMNetworkManager::setNetworkingEnabled(bool enabled)
{
    Q_D(NMNetworkManager);

    QDBusPendingReply<> reply = d->iface.Enable(enabled);
    reply.waitForFinished();
    if (reply.isError()) {
        kDebug(1441) << "Enable() D-Bus method not available:" << reply.error();
        kDebug(1441) << "Calling Sleep() instead";
        d->iface.Sleep(!enabled);
    }
}

void NMNetworkManager::setWirelessEnabled(bool enabled)
{
    Q_D(NMNetworkManager);
    d->iface.setWirelessEnabled(enabled);
}

void NMNetworkManager::setWwanEnabled(bool enabled)
{
    Q_D(NMNetworkManager);
    d->iface.setWwanEnabled(enabled);
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

        // When "NetworkingEnabled" property is not available, set isNetworkingEnabled flag and emit signal here.
        // It has to be done before emitting statusChanged(), else it would cause infinite status switching.
        if (!d->NetworkingEnabledPropertyAvailable) {
            d->isNetworkingEnabled = !(NM_STATE_UNKNOWN == state || NM_STATE_ASLEEP == state);
            emit networkingEnabledChanged(d->isNetworkingEnabled);
        }

        // set new state
        d->nmState = state;
        emit statusChanged( convertNMState( state ) );
    }

}

void NMNetworkManager::propertiesChanged(const QVariantMap &properties)
{
    Q_D(NMNetworkManager);
    kDebug(1441) << properties.keys();
    QLatin1String activeConnKey("ActiveConnections");
    QLatin1String netEnabledKey("NetworkingEnabled");
    QLatin1String wifiHwKey("WirelessHardwareEnabled");
    QLatin1String wifiEnabledKey("WirelessEnabled");
    QLatin1String wwanHwKey("WwanHardwareEnabled");
    QLatin1String wwanEnabledKey("WwanEnabled");
    QVariantMap::const_iterator it = properties.find(activeConnKey);
    if ( it != properties.end()) {
        QList<QDBusObjectPath> activePaths = qdbus_cast< QList<QDBusObjectPath> >(*it);
        d->activeConnections.clear();
        if ( activePaths.count() ) {
            kDebug(1441) << activeConnKey;
        }
        foreach (const QDBusObjectPath &ac, activePaths)
        {
            d->activeConnections.append(ac.path());
            kDebug(1441) << "  " << ac.path();
        }
        emit activeConnectionsChanged();
    }
    it = properties.find(wifiHwKey);
    if ( it != properties.end()) {
        d->isWirelessHardwareEnabled = it->toBool();
        kDebug(1441) << wifiHwKey << d->isWirelessHardwareEnabled;
        emit wirelessHardwareEnabledChanged(d->isWirelessHardwareEnabled);
    }
    it = properties.find(wifiEnabledKey);
    if ( it != properties.end()) {
        d->isWirelessEnabled = it->toBool();
        kDebug(1441) << wifiEnabledKey << d->isWirelessEnabled;
        emit wirelessEnabledChanged(d->isWirelessEnabled);
    }
    it = properties.find(wwanHwKey);
    if ( it != properties.end()) {
        d->isWwanHardwareEnabled = it->toBool();
        kDebug(1441) << wwanHwKey << d->isWwanHardwareEnabled;
    }
    it = properties.find(wwanEnabledKey);
    if ( it != properties.end()) {
        d->isWwanEnabled = it->toBool();
        kDebug(1441) << wwanEnabledKey << d->isWwanEnabled;
        emit wwanEnabledChanged(d->isWwanEnabled);
    }
    it = properties.find(netEnabledKey);
    if ( it != properties.end()) {
        d->isNetworkingEnabled = it->toBool();
        kDebug(1441) << netEnabledKey << d->isNetworkingEnabled;
        emit networkingEnabledChanged(d->isNetworkingEnabled);
    }
}

Solid::Networking::Status NMNetworkManager::convertNMState(uint state)
{
    Solid::Networking::Status status = Solid::Networking::Unknown;
    switch (state) {
        case NM_STATE_UNKNOWN:
        case NM_STATE_ASLEEP:
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

void NMNetworkManager::nameOwnerChanged(QString name, QString oldOwner, QString newOwner)
{
    if ( name == QLatin1String("org.freedesktop.NetworkManager") ) {
        kDebug(1441) << "name: " << name << ", old owner: " << oldOwner << ", new owner: " << newOwner;
        if ( oldOwner.isEmpty() && !newOwner.isEmpty() ) {
            // NetworkManager started, but we are already listening to StateChanged so we should get
            // its status that way
            ;
        }
        if ( !oldOwner.isEmpty() && newOwner.isEmpty() ) {
            // NetworkManager stopped, set status Unknown for safety
            stateChanged(NM_STATE_UNKNOWN);
        }
    }
}

QStringList NMNetworkManager::activeConnections() const
{
    Q_D(const NMNetworkManager);
    return d->activeConnections;
}

Solid::Control::NetworkInterface::Types NMNetworkManager::supportedInterfaceTypes() const
{
    return (Solid::Control::NetworkInterface::Types) (
           Solid::Control::NetworkInterface::Ieee80211 |
           Solid::Control::NetworkInterface::Ieee8023 |
           Solid::Control::NetworkInterface::Gsm |
           Solid::Control::NetworkInterface::Cdma |
           Solid::Control::NetworkInterface::Serial
           );
}

#include "manager.moc"

