/*  This file is part of the KDE project
    Copyright (C) 2007 Will Stephenson <wstephenson@kde.org>
    Copyright (C) 2008 Pino Toscano <pino@kde.org>

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

#include <solid/control/ifaces/wirelessnetworkinterface.h>
#include <solid/control/wirelessnetworkinterface.h>

#include "NetworkManager-networkinterface.h"

class NMWirelessNetworkPrivate;

class NMWirelessNetwork : public NMNetworkInterface, virtual public Solid::Control::Ifaces::WirelessNetworkInterface
{
Q_OBJECT
Q_INTERFACES(Solid::Control::Ifaces::WirelessNetworkInterface)
public:
    NMWirelessNetwork(const QString  & networkPath);
    virtual ~NMWirelessNetwork();
    int bitRate() const;
    Solid::Control::WirelessNetworkInterface::Capabilities wirelessCapabilities() const;
    Solid::Control::WirelessNetworkInterface::OperationMode mode() const;
    void setSignalStrength(const QDBusObjectPath & netPath, int strength);
    void setBitrate(int rate);
#if 0
    virtual void setActivated(bool activated);
#endif
    MacAddressList accessPoints() const;
    QString activeAccessPoint() const;
    QString hardwareAddress() const;
    QObject * createAccessPoint(const QString & uni);
    /* reimp */ bool activateConnection(const QString & connectionUni, const QVariantMap & connectionParameters);
    /* reimp */ bool deactivateConnection();
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
