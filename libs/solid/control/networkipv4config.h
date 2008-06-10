/*
* Copyright 2008 Will Stephenson <wstephenson@kde.org>

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

#ifndef SOLID_CONTROL_NETWORKIPV4CONFIG_H
#define SOLID_CONTROL_NETWORKIPV4CONFIG_H

#include <QtCore/QStringList>

#include "solid_control_export.h"

namespace Solid
{
namespace Control
{

class SOLIDCONTROL_EXPORT IPv4Config
{
public:
    IPv4Config(const QList< QList<quint32> > &addresses,
        quint32 broadcast, const QString &hostname, const QList<quint32> &nameservers,
        const QStringList &domains, const QString &nisDomain, const QList<quint32> &nisServers);
    IPv4Config();
    ~IPv4Config();
    IPv4Config(const IPv4Config&);
    /** 
     * List of IP addresses related to this configuration. 
     * Each inner list represents a triple <IP address, subnet mask, gateway>
     * The outer list allows a configuration to have multiple addresses
     */
    QList< QList<quint32> >addresses() const;
    /** May go away, need to check if the broadcast address is *ALWAYS* .255 */
    quint32 broadcast() const;
    QString hostname() const;
    QList<quint32> nameservers() const;
    QStringList domains() const;
    QString nisDomain() const;
    QList<quint32> nisServers() const;
    IPv4Config &operator=(const IPv4Config&);
    bool isValid() const;
private:
    class Private;
    Private * d;
};

} // namespace Control
} // namespace Solid

#endif // NETWORKIPV4CONFIG_H
