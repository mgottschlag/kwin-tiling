/*  This file is part of the KDE project
    Copyright (C) 2006 Will Stephenson <wstephenson@kde.org>

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

#include <QStringList>

#include "fakenetworkinterface.h"
#include "fakenetwork.h"

#include <kdebug.h>

FakeNetworkInterface::FakeNetworkInterface(const QMap<QString, QVariant>  & propertyMap, QObject * parent)
: Solid::Control::Ifaces::NetworkInterface(parent), mPropertyMap(propertyMap)
{
}

FakeNetworkInterface::~FakeNetworkInterface()
{
}


QString FakeNetworkInterface::uni() const
{
    return mPropertyMap["uni"].toString();
}

bool FakeNetworkInterface::isActive() const
{
    return mPropertyMap["active"].toBool();
}

Solid::Control::NetworkInterface::Type FakeNetworkInterface::type() const
{
    QString typeString = mPropertyMap["type"].toString();

    if (typeString == "ieee8023")
        return Solid::Control::NetworkInterface::Ieee8023;
    else if (typeString == "ieee80211")
        return Solid::Control::NetworkInterface::Ieee80211;
    else
        return Solid::Control::NetworkInterface::UnknownType;
}

Solid::Control::NetworkInterface::ConnectionState FakeNetworkInterface::connectionState() const
{
    QString stateString = mPropertyMap["state"].toString();

    if (stateString == "prepare")
        return Solid::Control::NetworkInterface::Prepare;
    else if (stateString == "configure")
        return Solid::Control::NetworkInterface::Configure;
    else if (stateString == "needuserkey")
        return Solid::Control::NetworkInterface::NeedUserKey;
    else if (stateString == "ipstart")
        return Solid::Control::NetworkInterface::IPStart;
    else if (stateString == "ipget")
        return Solid::Control::NetworkInterface::IPGet;
    else if (stateString == "ipcommit")
        return Solid::Control::NetworkInterface::IPCommit;
    else if (stateString == "activated")
        return Solid::Control::NetworkInterface::Activated;
    else if (stateString == "failed")
        return Solid::Control::NetworkInterface::Failed;
    else if (stateString == "cancelled")
        return Solid::Control::NetworkInterface::Cancelled;
    else
        return Solid::Control::NetworkInterface::UnknownState;
}

int FakeNetworkInterface::signalStrength() const
{
    return mPropertyMap["signalstrength"].toInt();
}

int FakeNetworkInterface::designSpeed() const
{
    return mPropertyMap["speed"].toInt();
}

bool FakeNetworkInterface::isLinkUp() const
{
    return mPropertyMap["linkup"].toBool();
}

Solid::Control::NetworkInterface::Capabilities FakeNetworkInterface::capabilities() const
{
    QStringList capStrings = mPropertyMap["capabilities"].toString().simplified().split(',');

    Solid::Control::NetworkInterface::Capabilities caps = 0;
    if (capStrings.contains("manageable"))
        caps |= Solid::Control::NetworkInterface::IsManageable;
    if (capStrings.contains("carrierdetect"))
        caps |= Solid::Control::NetworkInterface::SupportsCarrierDetect;
    if (capStrings.contains("wirelessscan"))
        caps |= Solid::Control::NetworkInterface::SupportsWirelessScan;

    return caps;
}

QObject * FakeNetworkInterface::createNetwork(const QString  & uni)
{
    if (mNetworks.contains(uni))
    {
        kDebug() << "found " << uni;
        return mNetworks[uni];
    }
    else
        kDebug() << "NOT found " << uni;
        return 0;
}

QStringList FakeNetworkInterface::networks() const
{
    return mNetworks.keys();
}

QString FakeNetworkInterface::activeNetwork() const
{
    return "/org/freedesktop/NetworkManager/Devices/eth0";
}
void FakeNetworkInterface::setActive(bool active)
{
    mPropertyMap.insert("active", QVariant(active));
}

void FakeNetworkInterface::injectNetwork(const QString  & uni, FakeNetwork * net)
{
   mNetworks.insert(uni, net);
}

#include "fakenetworkinterface.moc"
