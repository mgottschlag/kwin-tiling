/*
Copyright 2011 Lamarque Souza <lamarque@gmail.com>

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

#include <KDebug>

#include "networkbtinterface.h"
#include "networkbtinterface_p.h"
#include "manager.h"

#include "solid/control/modemmanager.h"

NMBtNetworkInterfacePrivate::NMBtNetworkInterfacePrivate(const QString & path, QObject * owner)
    : NMGsmNetworkInterfacePrivate(path, owner), btIface(NMNetworkManager::DBUS_SERVICE, path, QDBusConnection::systemBus())
{
    btCapabilities = static_cast<QFlags<Solid::Control::BtNetworkInterface::Capability> >(btIface.btCapabilities());
    hardwareAddress = btIface.hwAddress();
    name = btIface.name();
}

NMBtNetworkInterface::NMBtNetworkInterface(const QString & path, NMNetworkManager * manager, QObject * parent)
    : NMGsmNetworkInterface(*new NMBtNetworkInterfacePrivate(path, this), manager, parent)
{
    Q_D(NMBtNetworkInterface);
    connect( &d->btIface, SIGNAL(PropertiesChanged(QVariantMap)),
                this, SLOT(btPropertiesChanged(QVariantMap)));
}

NMBtNetworkInterface::~NMBtNetworkInterface()
{

}

void NMBtNetworkInterface::btPropertiesChanged(const QVariantMap & changedProperties)
{
    kDebug(1441) << changedProperties.keys();
    Q_D(NMBtNetworkInterface);
    QLatin1String hwAddrKey("HwAddress"),
                  name("Name"),
                  capabilities("BtCapabilities");
    QVariantMap::const_iterator it = changedProperties.find(hwAddrKey);
    it = changedProperties.find(hwAddrKey);
    if (it != changedProperties.end()) {
        d->hardwareAddress = it->toString();
    }
    it = changedProperties.find(name);
    if (it != changedProperties.end()) {
        d->name = it->toString();
        emit networkNameChanged(d->name);
    }
    if (it != changedProperties.end()) {
        d->btCapabilities = static_cast<Solid::Control::BtNetworkInterface::Capabilities>(it->toUInt());
    }
}

Solid::Control::BtNetworkInterface::Capabilities NMBtNetworkInterface::btCapabilities() const
{
    Q_D(const NMBtNetworkInterface);
    return d->btCapabilities;
}

QString NMBtNetworkInterface::hardwareAddress() const
{
    Q_D(const NMBtNetworkInterface);
    return d->hardwareAddress;
}

QString NMBtNetworkInterface::name() const
{
    Q_D(const NMBtNetworkInterface);
    return d->name;
}

#include "networkbtinterface.moc"

// vim: sw=4 sts=4 et tw=100
