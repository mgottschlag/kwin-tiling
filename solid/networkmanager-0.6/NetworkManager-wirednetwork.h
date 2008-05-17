/*  This file is part of the KDE project
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

#ifndef NETWORKMANAGER_WIREDNETWORK_H
#define NETWORKMANAGER_WIREDNETWORK_H

#include <solid/control/ifaces/wirednetworkinterface.h>
#include <solid/control/wirednetworkinterface.h>

#include "NetworkManager-networkinterface.h"

class NMWiredNetworkPrivate;

class KDE_EXPORT NMWiredNetwork : public NMNetworkInterface, virtual public Solid::Control::Ifaces::WiredNetworkInterface
{
Q_OBJECT
Q_INTERFACES(Solid::Control::Ifaces::WiredNetworkInterface)
public:
    NMWiredNetwork(const QString  & networkPath);
    virtual ~NMWiredNetwork();
    QString hardwareAddress() const;
    int bitRate() const;
    bool carrier() const;
    void setBitrate(int rate);
    void setCarrier(bool carrier);
    /* reimp */ bool activateConnection(const QString & connectionUni, const QString & extra_connection_parameter);
    /* reimp */ bool deactivateConnection();
Q_SIGNALS:
    void bitRateChanged(int bitRate);
    void carrierChanged(bool plugged);
private:
    Q_DECLARE_PRIVATE(NMWiredNetwork)
    Q_DISABLE_COPY(NMWiredNetwork)
};

#endif
