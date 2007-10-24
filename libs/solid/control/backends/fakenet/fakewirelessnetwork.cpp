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

#include "fakewirelessnetwork.h"

FakeWirelessNetwork::FakeWirelessNetwork(const QString  & uni,
                                          const QMap<QString, QVariant>  & propertyMap,
                                          QObject * parent)
: FakeNetwork(uni, propertyMap, parent)
{
}

FakeWirelessNetwork::~FakeWirelessNetwork()
{

}

// PHY stuff
int FakeWirelessNetwork::signalStrength() const
{
    return mPropertyMap["signalstrength"].toInt();
}

int FakeWirelessNetwork::bitrate() const
{
    return mPropertyMap["bitrate"].toInt();
}

double FakeWirelessNetwork::frequency() const
{
    return mPropertyMap["frequency"].toInt();
}

Solid::Control::WirelessNetwork::Capabilities FakeWirelessNetwork::capabilities() const
{
    QStringList capStrings = mPropertyMap["capabilities"].toStringList();

    Solid::Control::WirelessNetwork::Capabilities caps = 0;
    if (capStrings.contains("wep"))
        caps |= Solid::Control::WirelessNetwork::Wep;
    if (capStrings.contains("wpa"))
        caps |= Solid::Control::WirelessNetwork::Wpa;
    if (capStrings.contains("wpa2"))
        caps |= Solid::Control::WirelessNetwork::Wpa2;
    if (capStrings.contains("psk"))
        caps |= Solid::Control::WirelessNetwork::Psk;
    if (capStrings.contains("ieee8021x"))
        caps |= Solid::Control::WirelessNetwork::Ieee8021x;
    if (capStrings.contains("wep40"))
        caps |= Solid::Control::WirelessNetwork::Wep40;
    if (capStrings.contains("wep104"))
        caps |= Solid::Control::WirelessNetwork::Wep104;
    if (capStrings.contains("wep192"))
        caps |= Solid::Control::WirelessNetwork::Wep192;
    if (capStrings.contains("wep256"))
        caps |= Solid::Control::WirelessNetwork::Wep256;
    if (capStrings.contains("wep_other"))
        caps |= Solid::Control::WirelessNetwork::WepOther;
    if (capStrings.contains("tkip"))
        caps |= Solid::Control::WirelessNetwork::Tkip;
    if (capStrings.contains("ccmp"))
        caps |= Solid::Control::WirelessNetwork::Ccmp;

    return caps;
}

QString FakeWirelessNetwork::essid() const
{
    return mPropertyMap["essid"].toString();
}

Solid::Control::WirelessNetwork::OperationMode FakeWirelessNetwork::mode() const
{
    QString modeName = mPropertyMap["mode"].toString();

    if (modeName == "adhoc")
        return Solid::Control::WirelessNetwork::Adhoc;
    else if (modeName == "managed")
        return Solid::Control::WirelessNetwork::Managed;
    else if (modeName == "master")
        return Solid::Control::WirelessNetwork::Master;
    else if (modeName == "repeater")
        return Solid::Control::WirelessNetwork::Repeater;
    else
        return Solid::Control::WirelessNetwork::Unassociated;
}

bool FakeWirelessNetwork::isAssociated() const
{
    return mPropertyMap["signalstrength"].toBool();
}

bool FakeWirelessNetwork::isEncrypted() const
{
    return mPropertyMap["encrypted"].toBool();
}

bool FakeWirelessNetwork::isHidden() const
{
    return mPropertyMap["hidden"].toBool();
}

MacAddressList FakeWirelessNetwork::bssList() const
{
    return mPropertyMap["bsslist"].toStringList();
}

Solid::Control::Authentication *FakeWirelessNetwork::authentication() const
{
    return mAuthentication;
}

void FakeWirelessNetwork::setAuthentication(Solid::Control::Authentication *authentication)
{
    mAuthentication = authentication;
}


#include "fakewirelessnetwork.moc"
