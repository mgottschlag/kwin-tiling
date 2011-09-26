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

#ifndef NM09_NETWORKMANAGER_H
#define NM09_NETWORKMANAGER_H

#include "solid/control/ifaces/networkmanager.h"
#include <QDBusObjectPath>
#include <kdemacros.h>

class NMNetworkInterface;
class NMNetworkManagerPrivate;

class KDE_EXPORT NMNetworkManager : public Solid::Control::Ifaces::NetworkManager
{
Q_OBJECT
Q_DECLARE_PRIVATE(NMNetworkManager)
Q_INTERFACES(Solid::Control::Ifaces::NetworkManager)

public:
    static const QString DBUS_SERVICE;
    static const QString DBUS_DAEMON_PATH;
    static const QString DBUS_USER_SETTINGS_PATH;
    static const QString DBUS_SYSTEM_SETTINGS_PATH;

    NMNetworkManager(QObject * parent, const QVariantList & args);
    ~NMNetworkManager();
    Solid::Networking::Status status() const;
    QStringList networkInterfaces() const;
    QObject *createNetworkInterface(const QString &uni);
    bool isNetworkingEnabled() const;
    bool isWirelessEnabled() const;
    bool isWirelessHardwareEnabled() const;
    bool isWwanEnabled() const;
    bool isWwanHardwareEnabled() const;
    void activateConnection(const QString & interfaceUni, const QString & connectionUni, const QVariantMap & connectionParameters);
    void deactivateConnection(const QString & activeConnection);
    QStringList activeConnections() const;
    Solid::Control::NetworkInterface::Types supportedInterfaceTypes() const;
public Q_SLOTS:
    void setNetworkingEnabled(bool enabled);
    void setWirelessEnabled(bool enabled);
    void setWwanEnabled(bool enabled);

Q_SIGNALS:
    void networkingEnabledChanged(bool);

protected Q_SLOTS:
    void deviceAdded(const QDBusObjectPath &state);
    void deviceRemoved(const QDBusObjectPath &state);
    void stateChanged(uint state);
    void propertiesChanged(const QVariantMap &properties);
    void nameOwnerChanged(QString, QString, QString);
private:
    static Solid::Networking::Status convertNMState(uint state);
    NMNetworkManagerPrivate * d_ptr;
};

#endif

