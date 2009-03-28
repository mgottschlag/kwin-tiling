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

#include "wirelessaccesspoint.h"

#include "wicddbusinterface.h"

#include <QtDBus/QDBusReply>

class WicdAccessPoint::Private
{
    public:
        Private() {};

        void recacheInformation();
        void reloadNetworkId();

        int networkid;
        QString essid;
        QString bssid;
        int channel;
        QString mode;
        int strength;
        int quality;
        QString encryption_method;
        QString enctype;
};

void WicdAccessPoint::Private::recacheInformation()
{
    QDBusReply< QString > essidr = WicdDbusInterface::instance()->wireless().call("GetWirelessProperty", networkid, "essid");
    QDBusReply< QString > bssidr = WicdDbusInterface::instance()->wireless().call("GetWirelessProperty", networkid, "bssid");
    QDBusReply< int > channelr = WicdDbusInterface::instance()->wireless().call("GetWirelessProperty", networkid, "channel");
    QDBusReply< QString > moder = WicdDbusInterface::instance()->wireless().call("GetWirelessProperty", networkid, "mode");
    QDBusReply< int > strengthr = WicdDbusInterface::instance()->wireless().call("GetWirelessProperty", networkid, "strength");
    QDBusReply< int > qualityr = WicdDbusInterface::instance()->wireless().call("GetWirelessProperty", networkid, "qualityr");
    QDBusReply< QString > encryption_methodr = WicdDbusInterface::instance()->wireless().call("GetWirelessProperty", networkid, "encryption_method");
    QDBusReply< QString > enctyper = WicdDbusInterface::instance()->wireless().call("GetWirelessProperty", networkid, "enctype");

    essid = essidr.value();
    bssid = bssidr.value();
    channel = channelr.value();
    mode = moder.value();
    strength = strengthr.value();
    quality = qualityr.value();
    encryption_method = encryption_methodr.value();
    enctype = enctyper.value();
}

void WicdAccessPoint::Private::reloadNetworkId()
{
    QDBusReply< int > networks = WicdDbusInterface::instance()->wireless().call("GetNumberOfNetworks");

    for (int i = 0; i < networks.value(); ++i) {
        QDBusReply< QString > r = WicdDbusInterface::instance()->wireless().call("GetWirelessProperty", i, "bssid");
        if (r.value() == bssid) {
            networkid = i;
            return;
        }
    }
}

WicdAccessPoint::WicdAccessPoint(int networkid)
 : AccessPoint(0)
{
    d->networkid = networkid;
    d->recacheInformation();
}

WicdAccessPoint::~WicdAccessPoint()
{

}

QString WicdAccessPoint::uni() const
{
    return d->bssid;
}

Solid::Control::AccessPoint::Capabilities WicdAccessPoint::capabilities() const
{
    return 0;
}

Solid::Control::AccessPoint::WpaFlags WicdAccessPoint::wpaFlags() const
{
    Solid::Control::AccessPoint::WpaFlags f = (Solid::Control::AccessPoint::WpaFlags)0;

    if (d->encryption_method == "WPA") {
        f |= Solid::Control::AccessPoint::KeyMgmtPsk;
        f |= Solid::Control::AccessPoint::GroupTkip;
    } else if (d->encryption_method == "WPA2") {
        f |= Solid::Control::AccessPoint::KeyMgmtPsk;
        f |= Solid::Control::AccessPoint::GroupTkip;
    } else if (d->encryption_method == "WEP") {
        f |= Solid::Control::AccessPoint::PairWep104;
        f |= Solid::Control::AccessPoint::PairWep40;
    }

    return f;
}

Solid::Control::AccessPoint::WpaFlags WicdAccessPoint::rsnFlags() const
{
    return 0;
}

QString WicdAccessPoint::ssid() const
{
    return d->essid;
}

uint WicdAccessPoint::frequency() const
{
    return 0;
}

QString WicdAccessPoint::hardwareAddress() const
{
    return d->bssid;
}

uint WicdAccessPoint::maxBitRate() const
{
    return 0;
}

Solid::Control::WirelessNetworkInterface::OperationMode WicdAccessPoint::mode() const
{
    if (d->mode == "Master") {
        return Solid::Control::WirelessNetworkInterface::Master;
    } else if (d->mode == "Managed") {
        return Solid::Control::WirelessNetworkInterface::Managed;
    } else if (d->mode == "Adhoc") {
        return Solid::Control::WirelessNetworkInterface::Adhoc;
    }

    return Solid::Control::WirelessNetworkInterface::Master;
}

int WicdAccessPoint::signalStrength() const
{
    return d->quality;
}

#include "wirelessaccesspoint.moc"
