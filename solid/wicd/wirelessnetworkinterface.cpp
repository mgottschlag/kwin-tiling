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

#include "wirelessnetworkinterface.h"

#include "wirelessaccesspoint.h"

#include "wicddbusinterface.h"

#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>
#include <QProcess>

#include <KDebug>

class WicdWirelessNetworkInterface::Private
{
public:
    Private() {};

    QMap<int, QString> getAccessPointsWithId();
};

QMap<int, QString> WicdWirelessNetworkInterface::Private::getAccessPointsWithId()
{
    QDBusReply< int > networks = WicdDbusInterface::instance()->wireless().call("GetNumberOfNetworks");
    QMap<int, QString> retlist;

    for (int i = 0; i < networks.value(); ++i) {
        QDBusReply< QString > r = WicdDbusInterface::instance()->wireless().call("GetWirelessProperty", i, "bssid");
        retlist[i] = r;
    }

    return retlist;
}

WicdWirelessNetworkInterface::WicdWirelessNetworkInterface(const QString &objectPath)
 : WicdNetworkInterface(objectPath)
 , d(new Private())
{

}

WicdWirelessNetworkInterface::~WicdWirelessNetworkInterface()
{
    delete d;
}

int WicdWirelessNetworkInterface::bitRate() const
{

}

Solid::Control::WirelessNetworkInterface::Capabilities WicdWirelessNetworkInterface::wirelessCapabilities() const
{

}

Solid::Control::WirelessNetworkInterface::OperationMode WicdWirelessNetworkInterface::mode() const
{

}

MacAddressList WicdWirelessNetworkInterface::accessPoints() const
{
    return d->getAccessPointsWithId().values();
}

QString WicdWirelessNetworkInterface::activeAccessPoint() const
{

}

QString WicdWirelessNetworkInterface::hardwareAddress() const
{

}

QObject * WicdWirelessNetworkInterface::createAccessPoint(const QString & uni)
{
    QMap<int, QString> aps = d->getAccessPointsWithId();

    if (aps.values().contains(uni)) {
        kDebug() << "Requested a non existant AP";
    }

    int network = aps.key(uni);

    return new WicdAccessPoint(network);
}

#include "wirelessnetworkinterface.moc"
