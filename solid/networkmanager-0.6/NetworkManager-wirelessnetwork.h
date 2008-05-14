/*  This file is part of the KDE project
    Copyright (C) 2007 Will Stephenson <wstephenson@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#ifndef NETWORKMANAGER_WIRELESSNETWORK_H
#define NETWORKMANAGER_WIRELESSNETWORK_H

#include <kdemacros.h>

#include <QStringList>
#include <qdbusextratypes.h>

#include <solid/control/ifaces/wirelessnetworkinterface.h>
#include <solid/control/wirelessnetworkinterface.h>

#include "NetworkManager-networkinterface.h"

//typedef QString MacAddress;
//typedef QStringList MacAddressList;

namespace Solid {
namespace Control {
class Authentication;
}
}
class NMWirelessNetworkPrivate;

class KDE_EXPORT NMWirelessNetwork : public NMNetworkInterface, virtual public Solid::Control::Ifaces::WirelessNetworkInterface
{
Q_OBJECT
Q_INTERFACES(Solid::Control::Ifaces::WirelessNetworkInterface)
public:
    NMWirelessNetwork(const QString  & networkPath);
    virtual ~NMWirelessNetwork();
    int bitRate() const;
    Solid::Control::WirelessNetworkInterface::Capabilities wirelessCapabilities() const;
    Solid::Control::WirelessNetworkInterface::OperationMode mode() const;
    bool isAssociated() const; // move to Device, is this a property on device?
    bool isEncrypted() const;
    bool isHidden() const;
    Solid::Control::Authentication *authentication() const;
    void setAuthentication(Solid::Control::Authentication *authentication);
    void setSignalStrength(const QDBusObjectPath & netPath, int strength);
    void setBitrate(int rate);
#if 0
    virtual void setActivated(bool activated);
#endif
    MacAddressList accessPoints() const;
    QString activeAccessPoint() const;
    QString hardwareAddress() const;
    QObject * createAccessPoint(const QString & uni);
Q_SIGNALS:
    void bitRateChanged(int bitrate);
    void activeAccessPointChanged(const QString &);
    void modeChanged(Solid::Control::WirelessNetworkInterface::OperationMode);
    void accessPointAppeared(const QString &);
    void accessPointDisappeared(const QString &);
private:
    Q_DECLARE_PRIVATE(NMWirelessNetwork)
    Q_DISABLE_COPY(NMWirelessNetwork)
};

#endif
