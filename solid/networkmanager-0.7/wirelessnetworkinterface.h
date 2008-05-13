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

#ifndef NM07_WIRELESSNETWORKINTERFACE_H
#define NM07_WIRELESSNETWORKINTERFACE_H

#include "networkinterface.h"

#include <solid/control/ifaces/wirelessnetworkinterface.h>

#include <QDBusObjectPath>
#include <kdemacros.h>
#include "dbus/generic-types.h"

class NMNetworkManager;
class NMWirelessNetworkInterfacePrivate;

class KDE_EXPORT NMWirelessNetworkInterface : public NMNetworkInterface, virtual public Solid::Control::Ifaces::WirelessNetworkInterface
{
Q_OBJECT
Q_DECLARE_PRIVATE(NMWirelessNetworkInterface)
Q_INTERFACES(Solid::Control::Ifaces::WirelessNetworkInterface)

public:
    NMWirelessNetworkInterface(const QString & path, NMNetworkManager * manager, QObject * parent);
    ~NMWirelessNetworkInterface();
    MacAddressList accessPoints() const;
    QString activeAccessPoint() const;
    QString hardwareAddress() const;
    Solid::Control::WirelessNetworkInterface::OperationMode mode() const;
    int bitRate() const;
    Solid::Control::WirelessNetworkInterface::Capabilities wirelessCapabilities() const;
    QObject * createAccessPoint(const QString & uni);

    static Solid::Control::WirelessNetworkInterface::OperationMode convertOperationMode(uint);
    static Solid::Control::WirelessNetworkInterface::Capabilities convertCapabilities(uint);
protected Q_SLOTS:
    void wirelessPropertiesChanged(const QVariantMap &);
    void accessPointAdded(const QDBusObjectPath &);
    void accessPointRemoved(const QDBusObjectPath &);
Q_SIGNALS:
    void bitrateChanged(int bitrate);
    void activeAccessPointChanged(const QString &);
    void modeChanged(Solid::Control::WirelessNetworkInterface::OperationMode);
    void accessPointAppeared(const QString &);
    void accessPointDisappeared(const QString &);
};

#endif //NM07_WIRELESSNETWORKINTERFACE_H

