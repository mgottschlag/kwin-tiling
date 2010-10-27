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

#ifndef MM04_MODEMMANAGER_H
#define MM04_MODEMMANAGER_H

#include "solid/control/ifaces/modemmanager.h"
#include <QDBusObjectPath>
#include <kdemacros.h>

class MMModemManagerPrivate;

class KDE_EXPORT MMModemManager : public Solid::Control::Ifaces::ModemManager
{
Q_OBJECT
Q_DECLARE_PRIVATE(MMModemManager)
Q_INTERFACES(Solid::Control::Ifaces::ModemManager)

public:
    static const QString DBUS_SERVICE;
    static const QString DBUS_DAEMON_PATH;
    static const QString DBUS_USER_SETTINGS_PATH;
    static const QString DBUS_SYSTEM_SETTINGS_PATH;

    MMModemManager(QObject * parent, const QVariantList & args);
    ~MMModemManager();
    Solid::Networking::Status status() const;
    QStringList modemInterfaces() const;
    QObject *createModemInterface(const QString &udi, const Solid::Control::ModemInterface::GsmInterfaceType ifaceType);

Q_SIGNALS:
    void statusChanged(Solid::Networking::Status status);
    void modemInterfaceAdded(const QString & udi);
    void modemInterfaceRemoved(const QString & udi);

protected Q_SLOTS:
    void deviceAdded(const QDBusObjectPath & device);
    void deviceRemoved(const QDBusObjectPath & device);
    void stateChanged(Solid::Networking::Status state);
    void nameOwnerChanged(QString, QString, QString);

private:
    MMModemManagerPrivate * d_ptr;
};

#endif

