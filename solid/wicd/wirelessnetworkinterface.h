/*  This file is part of the KDE project
    Copyright (C) 2008 Dario Freddi <drf54321@gmail.com>

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

#ifndef WICD_WIRELESSNETWORKINTERFACE_H
#define WICD_WIRELESSNETWORKINTERFACE_H

#include <solid/control/ifaces/wirelessnetworkinterface.h>
#include <solid/control/wirelessnetworkinterface.h>

#include "networkinterface.h"

class WicdWirelessNetworkInterface : public WicdNetworkInterface, virtual public Solid::Control::Ifaces::WirelessNetworkInterface
{
    Q_OBJECT
    Q_INTERFACES(Solid::Control::Ifaces::WirelessNetworkInterface)
public:
    WicdWirelessNetworkInterface(const QString  & objectPath);
    virtual ~WicdWirelessNetworkInterface();
    Solid::Control::NetworkInterface::Type type() const;
    QString driver() const;
    Solid::Control::NetworkInterface::ConnectionState connectionState() const;
    bool isActive() const;
    Solid::Control::NetworkInterface::Capabilities capabilities() const;
    int bitRate() const;
    Solid::Control::WirelessNetworkInterface::Capabilities wirelessCapabilities() const;
    Solid::Control::WirelessNetworkInterface::OperationMode mode() const;
    MacAddressList accessPoints() const;
    QString activeAccessPoint() const;
    QString hardwareAddress() const;
    QObject * createAccessPoint(const QString & uni);
private Q_SLOTS:
    void recacheInformation();
Q_SIGNALS:
    void bitRateChanged(int bitrate);
    void activeAccessPointChanged(const QString &);
    void modeChanged(Solid::Control::WirelessNetworkInterface::OperationMode);
    void accessPointAppeared(const QString &);
    void accessPointDisappeared(const QString &);
private:
    class Private;
    Private *d;
};

#endif //SOLID_IFACES_WIRELESSNETWORKINTERFACE_H

