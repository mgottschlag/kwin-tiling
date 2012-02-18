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

#ifndef MM04_MODEMGSMNETWORKINTERFACE_H
#define MM04_MODEMGSMNETWORKINTERFACE_H

#include "modeminterface.h"
#include "solid/control/ifaces/modemgsmnetworkinterface.h"
#include "dbus/generic-types.h"

class MMModemGsmNetworkInterfacePrivate;

class KDE_EXPORT MMModemGsmNetworkInterface : public MMModemInterface, virtual public Solid::Control::Ifaces::ModemGsmNetworkInterface
{
Q_OBJECT
Q_DECLARE_PRIVATE(MMModemGsmNetworkInterface)
Q_INTERFACES(Solid::Control::Ifaces::ModemGsmNetworkInterface)

public:
    MMModemGsmNetworkInterface(const QString & path, MMModemManager * manager, QObject * parent);
    ~MMModemGsmNetworkInterface();

    void registerToNetwork(const QString & networkId);
    ScanResultsType scan();
    void setApn(const QString & apn);
    uint getSignalQuality();
    void setBand(const Solid::Control::ModemInterface::Band band);
    Solid::Control::ModemInterface::Band getBand();
    RegistrationInfoType getRegistrationInfo();
    void setAllowedMode(const Solid::Control::ModemInterface::AllowedMode mode);
    // properties
    Solid::Control::ModemInterface::AllowedMode getAllowedMode() const;
    Solid::Control::ModemInterface::AccessTechnology getAccessTechnology() const;
public Q_SLOTS:
    void slotSignalQualityChanged(uint signalQuality);
    void slotRegistrationInfoChanged(uint status, const QString & operatorCode, const QString &operatorName);
    void propertiesChanged(const QString & interface, const QVariantMap & properties);
Q_SIGNALS:
    void signalQualityChanged(uint signalQuality);
    void registrationInfoChanged(const Solid::Control::ModemGsmNetworkInterface::RegistrationInfoType &registrationInfo);
    // properties
    void allowedModeChanged(const Solid::Control::ModemInterface::AllowedMode mode);
    void accessTechnologyChanged(const Solid::Control::ModemInterface::AccessTechnology tech);
};

#endif

