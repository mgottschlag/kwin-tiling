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

#include "NetworkManager-wirednetwork.h"

#include "NetworkManager-networkinterface_p.h"

#include <kdebug.h>

class NMWiredNetworkPrivate : public NMNetworkInterfacePrivate
{
public:
    NMWiredNetworkPrivate(const QString & netPath);
    Q_DECLARE_PUBLIC(NMWiredNetwork)
    /* reimp */ void applyProperties(const NMDBusDeviceProperties & props);
    QString hwAddr;
    int rate;
};

NMWiredNetworkPrivate::NMWiredNetworkPrivate(const QString & netPath)
    : NMNetworkInterfacePrivate(netPath)
    , rate(0)
{
}

void NMWiredNetworkPrivate::applyProperties(const NMDBusDeviceProperties & props)
{
    NMNetworkInterfacePrivate::applyProperties(props);

    hwAddr = props.hardwareAddress;
}


NMWiredNetwork::NMWiredNetwork(const QString  & networkPath)
 : NMNetworkInterface(*new NMWiredNetworkPrivate(networkPath))
{
}

NMWiredNetwork::~NMWiredNetwork()
{
}

QString NMWiredNetwork::hardwareAddress() const
{
    Q_D(const NMWiredNetwork);
    return d->hwAddr;
}

int NMWiredNetwork::bitRate() const
{
    Q_D(const NMWiredNetwork);
    return d->rate;
}

bool NMWiredNetwork::carrier() const
{
    Q_D(const NMWiredNetwork);
    return d->carrier;
}

void NMWiredNetwork::setBitrate(int rate)
{
    Q_D(NMWiredNetwork);
    if (d->rate == rate)
        return;

    d->rate = rate;
    emit bitRateChanged(rate);
}

bool NMWiredNetwork::activateConnection(const QString & connectionUni, const QString & extra_connection_parameter)
{
    Q_UNUSED(connectionUni)
    Q_UNUSED(extra_connection_parameter)
    return false;
}

bool NMWiredNetwork::deactivateConnection()
{
    return false;
}

#include "NetworkManager-wirednetwork.moc"
