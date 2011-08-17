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

#include "networkgsminterface.h"

#include <KDebug>

#include "networkgsminterface_p.h"
#include "manager.h"

#include "solid/control/modemmanager.h"

NMGsmNetworkInterfacePrivate::NMGsmNetworkInterfacePrivate(const QString & path, QObject * owner)
    : NMSerialNetworkInterfacePrivate(path, owner), gsmIface(NMNetworkManager::DBUS_SERVICE, path, QDBusConnection::systemBus())
{
}

NMGsmNetworkInterface::NMGsmNetworkInterface(const QString & path, NMNetworkManager * manager, QObject * parent)
    : NMSerialNetworkInterface(*new NMGsmNetworkInterfacePrivate(path, this), manager, parent),
      modemGsmCardIface(0), modemGsmNetworkIface(0)
{
    Q_D(NMGsmNetworkInterface);
    connect( &d->gsmIface, SIGNAL(PropertiesChanged(QVariantMap)),
                this, SLOT(gsmPropertiesChanged(QVariantMap)));
}

NMGsmNetworkInterface::NMGsmNetworkInterface(NMGsmNetworkInterfacePrivate & dd, NMNetworkManager * manager, QObject * parent) : NMSerialNetworkInterface(dd, manager, parent),
      modemGsmCardIface(0), modemGsmNetworkIface(0)
{
    Q_D(NMGsmNetworkInterface);
    connect( &d->gsmIface, SIGNAL(PropertiesChanged(QVariantMap)),
                this, SLOT(gsmPropertiesChanged(QVariantMap)));
}

NMGsmNetworkInterface::~NMGsmNetworkInterface()
{

}

void NMGsmNetworkInterface::gsmPropertiesChanged(const QVariantMap & changedProperties)
{
    kDebug(1441) << changedProperties.keys();
}

QString NMGsmNetworkInterface::getUdiForModemManager()
{
    if (driver() != QLatin1String("bluez")) {
        return udi();
    }

    /* BlueZ knows about the rfcommX string that we could use to find the device in ModemManager
     * but does not export this info, so let's use the first bluetooth device we find in ModemManager.
     * Modem will be registered in ModemManager only after someone execute its org.bluez.Serial.Connect method. */
    foreach(const Solid::Control::ModemInterface *modem, Solid::Control::ModemManager::modemInterfaces()) {
        if (modem->driver() == QLatin1String("bluetooth")) {
            return modem->udi();
        }
    }

    modemRemoved(udi());
    return QString();
}

Solid::Control::ModemGsmCardInterface * NMGsmNetworkInterface::getModemCardIface()
{
    QString correctUdi = getUdiForModemManager();
    if (correctUdi.isEmpty()) {
        return 0;
    }
    if (modemGsmCardIface == 0) {
        modemGsmCardIface = qobject_cast<Solid::Control::ModemGsmCardInterface*> (Solid::Control::ModemManager::findModemInterface(correctUdi, Solid::Control::ModemInterface::GsmCard));
        connect(Solid::Control::ModemManager::notifier(), SIGNAL(modemInterfaceRemoved(QString)), this, SLOT(modemRemoved(QString)));
    }

    return modemGsmCardIface;
}

Solid::Control::ModemGsmNetworkInterface * NMGsmNetworkInterface::getModemNetworkIface()
{
    QString correctUdi = getUdiForModemManager();
    if (correctUdi.isEmpty()) {
        return 0;
    }
    if (modemGsmNetworkIface == 0) {
        modemGsmNetworkIface = qobject_cast<Solid::Control::ModemGsmNetworkInterface*> (Solid::Control::ModemManager::findModemInterface(correctUdi, Solid::Control::ModemInterface::GsmNetwork));
        if (modemGsmNetworkIface) {
            connect(Solid::Control::ModemManager::notifier(), SIGNAL(modemInterfaceRemoved(QString)), this, SLOT(modemRemoved(QString)));
        }
    }

    return modemGsmNetworkIface;
}

void NMGsmNetworkInterface::modemRemoved(const QString & modemUdi)
{
    if (modemUdi == udi()) {
        modemGsmNetworkIface = 0;
        modemGsmCardIface = 0;
    }
}

void NMGsmNetworkInterface::setModemCardIface(Solid::Control::ModemGsmCardInterface * iface)
{
    modemGsmCardIface = iface;
}

void NMGsmNetworkInterface::setModemNetworkIface(Solid::Control::ModemGsmNetworkInterface * iface)
{
    modemGsmNetworkIface = iface;
}

#include "networkgsminterface.moc"

// vim: sw=4 sts=4 et tw=100
