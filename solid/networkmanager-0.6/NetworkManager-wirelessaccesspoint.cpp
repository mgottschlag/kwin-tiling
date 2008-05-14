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

#include "NetworkManager-wirelessaccesspoint.h"

#include <QtDBus>
#include <kdebug.h>

namespace AP
{

Solid::Control::WirelessNetworkInterface::OperationMode getOperationMode(int opmode)
{
    Solid::Control::WirelessNetworkInterface::OperationMode mode;
    return mode;
}

Solid::Control::AccessPoint::Capabilities getCapabilities(int netcap)
{
    Solid::Control::AccessPoint::Capabilities cap;
    return cap;
}

}


class NMAccessPointPrivate
{
public:
    NMAccessPointPrivate(const QString  & networkPath) : netPath(networkPath) { }

    void deserialize(const QDBusMessage & message);

    QString netPath;
    Solid::Control::AccessPoint::Capabilities capabilities;
    Solid::Control::AccessPoint::WpaFlags wpaFlags;
    Solid::Control::AccessPoint::WpaFlags rsnFlags;
    QString ssid;
    uint frequency;
    QString hardwareAddress;
    uint maxBitRate;
    Solid::Control::WirelessNetworkInterface::OperationMode mode;
    int signalStrength;
    bool broadcast;
};

void NMAccessPointPrivate::deserialize(const QDBusMessage &message)
{
    const QList<QVariant> args = message.arguments();
    if (args.size() > 0) ssid = args[1].toString();
    if (args.size() > 1) hardwareAddress = args[2].toString();
    if (args.size() > 2) signalStrength = args[3].toInt();
    // frequency: NM 0.6 provides it in Hz, while we need MHz
    if (args.size() > 3) frequency = static_cast<uint>(args[4].toDouble() / 1000000);
    if (args.size() > 4) maxBitRate = args[5].toUInt();
    if (args.size() > 5) mode = AP::getOperationMode(args[6].toInt());
    if (args.size() > 6) capabilities = AP::getCapabilities(args[7].toInt());
    if (args.size() > 7) broadcast = args[8].toBool();
}


NMAccessPoint::NMAccessPoint(const QString  & netPath)
    : Solid::Control::Ifaces::AccessPoint(0), d(new NMAccessPointPrivate(netPath))
{
    QDBusInterface iface("org.freedesktop.NetworkManager",
            netPath,
            "org.freedesktop.NetworkManager.Devices",
            QDBusConnection::systemBus());
    QDBusMessage reply = iface.call("getProperties");
    d->deserialize(reply);
}

NMAccessPoint::~NMAccessPoint()
{
    delete d;
}

QString NMAccessPoint::uni() const
{
    return d->netPath;
}

Solid::Control::AccessPoint::Capabilities NMAccessPoint::capabilities() const
{
    return d->capabilities;
}

Solid::Control::AccessPoint::WpaFlags NMAccessPoint::wpaFlags() const
{
    return d->wpaFlags;
}

Solid::Control::AccessPoint::WpaFlags NMAccessPoint::rsnFlags() const
{
    return d->rsnFlags;
}

QString NMAccessPoint::ssid() const
{
    return d->ssid;
}

uint NMAccessPoint::frequency() const
{
    return d->frequency;
}

QString NMAccessPoint::hardwareAddress() const
{
    return d->hardwareAddress;
}

uint NMAccessPoint::maxBitRate() const
{
    return d->maxBitRate;
}

Solid::Control::WirelessNetworkInterface::OperationMode NMAccessPoint::mode() const
{
    return d->mode;
}

void NMAccessPoint::setSignalStrength(int strength)
{
    if (strength == d->signalStrength)
        return;

    d->signalStrength = strength;
    emit signalStrengthChanged(d->signalStrength);
}

int NMAccessPoint::signalStrength() const
{
    return d->signalStrength;
}

#include "NetworkManager-wirelessaccesspoint.moc"
