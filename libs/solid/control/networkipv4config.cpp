/*
Copyright 2008 Will Stephenson <wstephenson@kde.org>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of
the License or (at your option) version 3 or any later version
accepted by the membership of KDE e.V. (or its successor approved
by the membership of KDE e.V.), which shall act as a proxy 
defined in Section 14 of version 3 of the license.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "networkipv4config.h"

namespace Solid
{
namespace Control
{
class IPv4Config::Private
{
public:
    Private(const QList< QList<quint32> > &theAddresses,
        quint32 theBroadcast, const QString &theHostname, const QList<quint32> &theNameservers,
        const QStringList &theDomains, const QString &theNisDomain, const QList<quint32> &theNisServers)
        : addresses(theAddresses), broadcast(theBroadcast),
    hostname(theHostname), nameservers(theNameservers), domains(theDomains),
    nisDomain(theNisDomain), nisServers(theNisServers)
    {}
    Private()
        : broadcast(0)
    {}
    QList< QList<quint32> > addresses;
    quint32 broadcast;
    QString hostname;
    QList<quint32> nameservers;
    QStringList domains;
    QString nisDomain;
    QList<quint32> nisServers;
};
}
}

Solid::Control::IPv4Config::IPv4Config(const QList< QList<quint32> > &addresses,
        quint32 broadcast, const QString &hostname, const QList<quint32> &nameservers,
        const QStringList &domains, const QString &nisDomain, const QList<quint32> &nisServers)
: d(new Private(addresses, broadcast, hostname, nameservers, domains, 
            nisDomain, nisServers))
{
}

Solid::Control::IPv4Config::IPv4Config()
: d(new Private())
{
}

Solid::Control::IPv4Config::IPv4Config(const Solid::Control::IPv4Config& other)
{
    d = new Private(*other.d);
}

Solid::Control::IPv4Config::~IPv4Config()
{
    delete d;
}

QList<QList<quint32> > Solid::Control::IPv4Config::addresses() const
{
    return d->addresses;
}

quint32 Solid::Control::IPv4Config::broadcast() const
{
    return d->broadcast;
}

QString Solid::Control::IPv4Config::hostname() const
{
    return d->hostname;
}

QList<quint32> Solid::Control::IPv4Config::nameservers() const
{
    return d->nameservers;
}

QStringList Solid::Control::IPv4Config::domains() const
{
    return d->domains;
}

QString Solid::Control::IPv4Config::nisDomain() const
{
    return d->nisDomain;
}

QList<quint32> Solid::Control::IPv4Config::nisServers() const
{
    return d->nisServers;
}

Solid::Control::IPv4Config &Solid::Control::IPv4Config::operator=(const Solid::Control::IPv4Config& other)
{
    if (this == &other)
        return *this;

    *d = *other.d;
    return *this;
}

bool Solid::Control::IPv4Config::isValid() const
{
    return !d->addresses.isEmpty();
}

