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
#include "wicd-defines.h"

#include <QtDBus/QDBusReply>

#include <KDebug>

class WicdAccessPoint::Private
{
public:
    Private(WicdAccessPoint *parent)
            : q(parent) {
        createChanMap();
    };

    void createChanMap();

    void recacheInformation();
    void reloadNetworkId();

    WicdAccessPoint *q;
    int networkid;
    QString essid;
    QString bssid;
    int channel;
    QString mode;
    int strength;
    int quality;
    QString encryption_method;
    QString enctype;
    uint frequency;
    QMap<int, uint> chanToFreq;
    int bitrate;
};

void WicdAccessPoint::Private::createChanMap()
{
    chanToFreq[1] = 2412;
    chanToFreq[2] = 2417;
    chanToFreq[3] = 2422;
    chanToFreq[4] = 2427;
    chanToFreq[5] = 2432;
    chanToFreq[6] = 2437;
    chanToFreq[7] = 2442;
    chanToFreq[8] = 2447;
    chanToFreq[9] = 2452;
    chanToFreq[10] = 2457;
    chanToFreq[11] = 2462;
    chanToFreq[12] = 2467;
    chanToFreq[13] = 2472;
    chanToFreq[14] = 2484;
}

void WicdAccessPoint::Private::recacheInformation()
{
    QDBusReply< QString > essidr = WicdDbusInterface::instance()->wireless().call("GetWirelessProperty", networkid, "essid");
    QDBusReply< QString > bssidr = WicdDbusInterface::instance()->wireless().call("GetWirelessProperty", networkid, "bssid");
    QDBusReply< QString > channelr = WicdDbusInterface::instance()->wireless().call("GetWirelessProperty", networkid, "channel");
    QDBusReply< QString > bitrater = WicdDbusInterface::instance()->wireless().call("GetWirelessProperty", networkid, "bitrates");
    QDBusReply< QString > moder = WicdDbusInterface::instance()->wireless().call("GetWirelessProperty", networkid, "mode");
    QDBusReply< int > strengthr = WicdDbusInterface::instance()->wireless().call("GetWirelessProperty", networkid, "strength");
    QDBusReply< int > qualityr = WicdDbusInterface::instance()->wireless().call("GetWirelessProperty", networkid, "quality");
    QDBusReply< QString > encryption_methodr = WicdDbusInterface::instance()->wireless().call("GetWirelessProperty", networkid, "encryption_method");
    QDBusReply< QString > enctyper = WicdDbusInterface::instance()->wireless().call("GetWirelessProperty", networkid, "enctype");

    if (essidr.value() != essid) {
        essid = essidr.value();
        emit q->ssidChanged(essid);
    }
    bssid = bssidr.value();
    channel = channelr.value().toInt();
    mode = moder.value();
    if (strength != strengthr.value()) {
        strength = strengthr.value();
        emit q->signalStrengthChanged(strength);
    }
    quality = qualityr.value();
    encryption_method = encryption_methodr.value();
    enctype = enctyper.value();
    if (frequency != chanToFreq[channel]) {
        frequency = chanToFreq[channel];
        emit q->frequencyChanged(frequency);
    }
    QStringList bitrates = bitrater.value().split(" Mb/s; ");
    bitrates.last().remove(" Mb/s");
    int bitrate_new = 0;

    foreach(const QString &b, bitrates) {
        bitrate_new = qMax(bitrate_new, b.toInt() * 1000);
    }

    if (bitrate_new != bitrate) {
        bitrate = bitrate_new;
        emit q->bitRateChanged(bitrate);
    }
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
        , d(new Private(this))
{
    d->networkid = networkid;
    d->recacheInformation();
    QDBusConnection::systemBus().connect(WICD_DBUS_SERVICE, WICD_DAEMON_DBUS_PATH, WICD_DAEMON_DBUS_INTERFACE,
                                         "StatusChanged", this, SLOT(refreshStatus()));
}

WicdAccessPoint::~WicdAccessPoint()
{
    delete d;
}

void WicdAccessPoint::refreshStatus()
{
    d->recacheInformation();
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

QByteArray WicdAccessPoint::rawSsid() const
{
    return d->essid.toUtf8();
}

uint WicdAccessPoint::frequency() const
{
    return d->frequency;
}

QString WicdAccessPoint::hardwareAddress() const
{
    return d->bssid;
}

uint WicdAccessPoint::maxBitRate() const
{
    return d->bitrate;
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
