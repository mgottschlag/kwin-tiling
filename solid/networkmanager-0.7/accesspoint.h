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

#ifndef NM07_ACCESSPOINT_H
#define NM07_ACCESSPOINT_H
#include <solid/control/ifaces/wirelessaccesspoint.h>

#include <solid/control/wirelessaccesspoint.h>
#include <solid/control/wirelessnetworkinterface.h>
#include "dbus/generic-types.h"

#include <kdemacros.h>

class KDE_EXPORT NMAccessPoint : public Solid::Control::Ifaces::AccessPoint
{
Q_OBJECT
public:
    NMAccessPoint( const QString & path, QObject * parent = 0 );
    virtual ~NMAccessPoint();

    QString uni() const;
    Solid::Control::AccessPoint::Capabilities capabilities() const;
    Solid::Control::AccessPoint::WpaFlags wpaFlags() const;
    Solid::Control::AccessPoint::WpaFlags rsnFlags() const;
    QString ssid() const;
    uint frequency() const;
    QString hardwareAddress() const;
    uint maxBitRate() const;
    Solid::Control::WirelessNetworkInterface::OperationMode mode() const;
    int signalStrength() const;
public Q_SLOTS:
    void propertiesChanged(const QVariantMap &properties);
Q_SIGNALS:
    void signalStrengthChanged(int strength);
    void wpaFlagsChanged(Solid::Control::AccessPoint::WpaFlags flags);
    void rsnFlagsChanged(Solid::Control::AccessPoint::WpaFlags flags);
    void ssidChanged(const QString &ssid);
    void frequencyChanged(uint frequency);
    void bitRateChanged(int);
private:
    static Solid::Control::AccessPoint::Capabilities convertCapabilities(int);
    static Solid::Control::AccessPoint::WpaFlags convertWpaFlags(uint);
private:
    class Private;
    Private * d;
};
#endif


