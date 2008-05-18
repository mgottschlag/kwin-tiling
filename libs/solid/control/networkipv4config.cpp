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
    uint address;
    uint gateway;
    uint netmask;
    uint broadcast;
    uint hostname;
    QList<uint> nameservers;
    QStringList domains;
    QString nisDomain;
    QList<uint> nisServers;
};
}
}

Solid::Control::IPv4Config::IPv4Config()
{
    d = new Private;
}

Solid::Control::IPv4Config::IPv4Config(const Solid::Control::IPv4Config& other)
{
    d = new Private(*other.d);
}

Solid::Control::IPv4Config::~IPv4Config()
{
}

uint Solid::Control::IPv4Config::address() const
{
    return d->address;
}

uint Solid::Control::IPv4Config::gateway() const
{
    return d->gateway;
}

uint Solid::Control::IPv4Config::netmask() const
{
    return d->netmask;
}

uint Solid::Control::IPv4Config::broadcast() const
{
    return d->broadcast;
}

uint Solid::Control::IPv4Config::hostname() const
{
    return d->hostname;
}

QList<uint> Solid::Control::IPv4Config::nameservers() const
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

QList<uint> Solid::Control::IPv4Config::nisServers() const
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

