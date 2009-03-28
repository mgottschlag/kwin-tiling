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

#ifndef WICD_WIREDNETWORKINTERFACE_H
#define WICD_WIREDNETWORKINTERFACE_H

#include <solid/control/ifaces/wirednetworkinterface.h>
#include <solid/control/wirednetworkinterface.h>

#include "networkinterface.h"

class WicdWiredNetworkInterface : public WicdNetworkInterface, virtual public Solid::Control::Ifaces::WiredNetworkInterface
{
    Q_OBJECT
    Q_INTERFACES(Solid::Control::Ifaces::WiredNetworkInterface)

public:
    WicdWiredNetworkInterface(const QString  & objectPath);
    virtual ~WicdWiredNetworkInterface();

    Solid::Control::NetworkInterface::Type type() const;
    Solid::Control::NetworkInterface::ConnectionState connectionState() const;
    bool isActive() const;
    Solid::Control::NetworkInterface::Capabilities capabilities() const;
    QString driver() const;
    QString hardwareAddress() const;
    int bitRate() const;
    bool carrier() const;

    /* reimp */ bool activateConnection(const QString & connectionUni, const QVariantMap & connectionParameters);
    /* reimp */ bool deactivateConnection();

private Q_SLOTS:
    void recacheInformation();
Q_SIGNALS:
    void bitRateChanged(int bitRate);
    void carrierChanged(bool plugged);
private:
    class Private;
    Private *d;
};

#endif // WICD_WIRELESSNETWORKINTERFACE_H

