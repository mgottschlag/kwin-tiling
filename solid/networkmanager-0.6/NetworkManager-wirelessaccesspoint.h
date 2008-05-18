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

#ifndef NETWORKMANAGER_WIRELESSACCESSPOINT_H
#define NETWORKMANAGER_WIRELESSACCESSPOINT_H

#include <QtCore/qobject.h>

#include <solid/control/ifaces/wirelessaccesspoint.h>

class NMAccessPointPrivate;

/**
 * This interface represents a generic Internet Protocol (IP) network which we may be connected to.
 */
class NMAccessPoint : public Solid::Control::Ifaces::AccessPoint
{
Q_OBJECT
Q_INTERFACES(Solid::Control::Ifaces::AccessPoint)
public:
    /**
     * Constructs a network and looks up its properties over DBus.
     * @param net contains the IP details of the network.
     */
    NMAccessPoint(const QString  & networkPath);
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
    void setSignalStrength(int strength);
    int signalStrength() const;
private:
    NMAccessPointPrivate * d;
};

#endif
