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

#include "networkserialinterface.h"
#include "solid/control/ifaces/networkgsminterface.h"
#include "solid/control/modemgsmnetworkinterface.h"

#include "dbus/generic-types.h"

#ifndef NM07_NETWORKGSMINTERFACE_H
#define NM07_NETWORKGSMINTERFACE_H

class NMNetworkManager;
class NMGsmNetworkInterfacePrivate;

class KDE_EXPORT NMGsmNetworkInterface : public NMSerialNetworkInterface, virtual public Solid::Control::Ifaces::GsmNetworkInterface
{
Q_OBJECT
Q_DECLARE_PRIVATE(NMGsmNetworkInterface)
Q_INTERFACES(Solid::Control::Ifaces::GsmNetworkInterface)
public:
    NMGsmNetworkInterface(const QString & path, NMNetworkManager * manager, QObject * parent);
    NMGsmNetworkInterface(NMGsmNetworkInterfacePrivate &dd, NMNetworkManager * manager, QObject * parent);
    virtual ~NMGsmNetworkInterface();
    Solid::Control::ModemGsmCardInterface * getModemCardIface();
    void setModemCardIface(Solid::Control::ModemGsmCardInterface * iface);
    Solid::Control::ModemGsmNetworkInterface * getModemNetworkIface();
    void setModemNetworkIface(Solid::Control::ModemGsmNetworkInterface * iface);
public Q_SLOTS:
    void gsmPropertiesChanged(const QVariantMap & changedProperties);
    void modemRemoved(const QString & modemUdi);
protected:
    Solid::Control::ModemGsmCardInterface *modemGsmCardIface;
    Solid::Control::ModemGsmNetworkInterface *modemGsmNetworkIface;
    QString getUdiForModemManager();
};

#endif
