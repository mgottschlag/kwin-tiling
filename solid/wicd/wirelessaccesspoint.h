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

#ifndef WICD_WIRELESSAP_H
#define WICD_WIRELESSAP_H

#include <QtCore/qobject.h>

#include <solid/control/ifaces/wirelessaccesspoint.h>

class WicdAccessPointPrivate;

class WicdAccessPoint : public Solid::Control::Ifaces::AccessPoint
{
Q_OBJECT
Q_INTERFACES(Solid::Control::Ifaces::AccessPoint)
public:
    WicdAccessPoint(const QString  & networkPath);
    virtual ~WicdAccessPoint();
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
private:
    WicdAccessPointPrivate * d;
};

#endif // WICD_WIRELESSAP_H

