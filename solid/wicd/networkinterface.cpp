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

#include "networkinterface.h"

#include <QProcess>

class WicdNetworkInterfacePrivate
{
public:
    WicdNetworkInterfacePrivate(const QString &name);

    quint32 parseIPv4Address(const QString & addressString);

    QString name;

};

WicdNetworkInterfacePrivate::WicdNetworkInterfacePrivate(const QString &n)
        : name(n)
{
}

quint32 WicdNetworkInterfacePrivate::parseIPv4Address(const QString & addressString)
{
    const QStringList parts = addressString.split(QChar::fromLatin1('.'), QString::SkipEmptyParts);
    if (parts.count() != 4)
        return 0;

    quint32 address = 0;
    for (int i = 0; i < 4; ++i) {
        bool ok = false;
        const short value = parts.at(i).toShort(&ok);
        if (value < 0 || value > 255)
            return 0;
        address |= (value << ((3 - i) * 8));
    }
    return address;
}

WicdNetworkInterface::WicdNetworkInterface(const QString &name)
        : NetworkInterface(), d(new WicdNetworkInterfacePrivate(name))
{

}

WicdNetworkInterface::~WicdNetworkInterface()
{
    delete d;
}

QString WicdNetworkInterface::interfaceName() const
{
    return d->name;
}

QString WicdNetworkInterface::ipInterfaceName() const
{
    return d->name;
}

bool WicdNetworkInterface::firmwareMissing() const
{
    // TODO: check if wicd can detect firmware missing.
    return false;
}

Solid::Control::IPv4Config WicdNetworkInterface::ipV4Config() const
{
    // Let's parse ifconfig here

    QProcess ifconfig;

    ifconfig.setEnvironment(QStringList() << QProcess::systemEnvironment() << "LANG=C");
    ifconfig.start(QString("ifconfig %1").arg(d->name));
    ifconfig.waitForFinished();

    QString result = ifconfig.readAllStandardOutput();

    QStringList lines = result.split('\n');

    if (!result.contains("inet addr:")) {
        return Solid::Control::IPv4Config(
                   QList<Solid::Control::IPv4Address>(),
                   QList<quint32>() /*nameservers*/,
                   QStringList() /* domains */,
                   QList<Solid::Control::IPv4Route>() /* routes*/);
    }
    QString inetadd = lines.at(1).split("inet addr:").at(1).split(' ').at(0);
    QString bcast = lines.at(1).split("Bcast:").at(1).split(' ').at(0);
    QString mask = lines.at(1).split("Mask:").at(1);

    Solid::Control::IPv4Address address(
        d->parseIPv4Address(inetadd),
        d->parseIPv4Address(mask),
        d->parseIPv4Address(bcast));

    QList<quint32> dnsServers;
    dnsServers.append(d->parseIPv4Address(bcast));

    return Solid::Control::IPv4Config(
               QList<Solid::Control::IPv4Address>() << address,
               dnsServers /*nameservers*/,
               QStringList() /* domains */,
               QList<Solid::Control::IPv4Route>() /* routes*/);
}

QString WicdNetworkInterface::uni() const
{
    return d->name;
}

QString WicdNetworkInterface::udi() const
{
    return d->name;
}

int WicdNetworkInterface::designSpeed() const
{
    return 0;
}

bool WicdNetworkInterface::activateConnection(const QString &, const QVariantMap &)
{
    return false;
}

bool WicdNetworkInterface::deactivateConnection()
{
    return false;
}

void WicdNetworkInterface::disconnectInterface()
{
}

#include "networkinterface.moc"
