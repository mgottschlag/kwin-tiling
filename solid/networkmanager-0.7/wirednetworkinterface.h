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

#ifndef NM07_WIREDNETWORKINTERFACE_H
#define NM07_WIREDNETWORKINTERFACE_H

#include "networkinterface.h"
#include <solid/control/ifaces/wirednetworkinterface.h>

#include <KDebug>
#include "dbus/generic-types.h"

class NMNetworkManager;
class NMWiredNetworkInterfacePrivate;

class KDE_EXPORT NMWiredNetworkInterface : public NMNetworkInterface, virtual public Solid::Control::Ifaces::WiredNetworkInterface
{
Q_OBJECT
Q_DECLARE_PRIVATE(NMWiredNetworkInterface)
Q_INTERFACES(Solid::Control::Ifaces::WiredNetworkInterface)
Q_PROPERTY(bool carrier READ carrier WRITE setCarrier NOTIFY carrierChanged)
Q_PROPERTY(int bitRate READ bitRate WRITE setBitRate NOTIFY bitrateChanged)

public:
    NMWiredNetworkInterface(const QString & path, NMNetworkManager * manager, QObject * parent);
    ~NMWiredNetworkInterface();
    QString hardwareAddress() const;
    int bitRate() const;
    void setBitRate(const QVariant&);
    bool carrier() const;
    void setCarrier(const QVariant&);
protected Q_SLOTS:
    void wiredPropertiesChanged(const QVariantMap &);
Q_SIGNALS:
    void bitRateChanged(int bitRate);
    void carrierChanged(bool plugged);
};

#endif // NM07_WIREDNETWORKINTERFACE_H

