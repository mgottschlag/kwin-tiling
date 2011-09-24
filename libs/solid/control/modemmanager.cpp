/*  This file is part of the KDE project
    Copyright (C) 2010-2011 Lamarque Souza <lamarque@gmail.com>

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

#include "ifaces/modemmanager.h"
#include "ifaces/modeminterface.h"
#include "ifaces/modemmanagerinterface.h"
#include "ifaces/modemgsmcardinterface.h"
#include "ifaces/modemgsmnetworkinterface.h"

#include "soliddefs_p.h"
#include "modemmanager_p.h"
#include "modemmanagerinterface.h"

#include "modemmanager.h"

#include <kglobal.h>
#include <kdebug.h>

K_GLOBAL_STATIC(Solid::Control::ModemManagerPrivate, globalModemManager)

Solid::Control::ModemManagerPrivate::ModemManagerPrivate() : m_invalidDevice(0)
{
    loadBackend("Modem Management",
                "SolidModemManager",
                "Solid::Control::Ifaces::ModemManager");

    if (managerBackend()!=0) {
        connect(managerBackend(), SIGNAL(modemInterfaceAdded(QString)),
                this, SLOT(_k_modemInterfaceAdded(QString)));
        connect(managerBackend(), SIGNAL(modemInterfaceRemoved(QString)),
                this, SLOT(_k_modemInterfaceRemoved(QString)));
    }
}

Solid::Control::ModemManagerPrivate::~ModemManagerPrivate()
{
    // Delete all the devices, they are now outdated
    typedef QMap<ModemInterface::GsmInterfaceType, ModemInterfaceIfacePair> ModemInterfaceIfaceMap;

    foreach (const ModemInterfaceIfaceMap &map, m_modemInterfaceMap) {
        foreach (const ModemInterfaceIfacePair &pair, map) {
            delete pair.first;
            delete pair.second;
        }
    }

    m_modemInterfaceMap.clear();
}

Solid::Control::ModemInterfaceList Solid::Control::ModemManagerPrivate::buildDeviceList(const QStringList &udiList)
{
    ModemInterfaceList list;
    Ifaces::ModemManager *backend = qobject_cast<Ifaces::ModemManager *>(managerBackend());

    if (backend == 0) return list;

    foreach (const QString &udi, udiList)
    {
        ModemInterfaceIfacePair pair = findRegisteredModemInterface(udi, ModemInterface::GsmNetwork);

        if (pair.first!= 0)
        {
            list.append(pair.first);
        }
    }

    return list;
}

Solid::Control::ModemInterfaceList Solid::Control::ModemManagerPrivate::modemInterfaces()
{
    Ifaces::ModemManager *backend = qobject_cast<Ifaces::ModemManager *>(managerBackend());

    if (backend!= 0)
    {
        return buildDeviceList(backend->modemInterfaces());
    }
    else
    {
        return ModemInterfaceList();
    }
}

Solid::Control::ModemInterfaceList Solid::Control::ModemManager::modemInterfaces()
{
    return globalModemManager->modemInterfaces();
}

Solid::Control::ModemInterface * Solid::Control::ModemManagerPrivate::findModemInterface(const QString &udi, const ModemInterface::GsmInterfaceType ifaceType)
{
    Ifaces::ModemManager *backend = qobject_cast<Ifaces::ModemManager *>(managerBackend());

    if (backend == 0) return 0;

    if (!backend->modemInterfaces().contains(udi)) {
        return 0;
    }

    ModemInterfaceIfacePair pair = findRegisteredModemInterface(udi, ifaceType);

    if (pair.first != 0)
    {
        return pair.first;
    }
    else
    {
        return 0;
    }
}

Solid::Control::ModemInterface * Solid::Control::ModemManager::findModemInterface(const QString &udi, const ModemInterface::GsmInterfaceType ifaceType)
{
    return globalModemManager->findModemInterface(udi, ifaceType);
}

Solid::Control::ModemManager::Notifier * Solid::Control::ModemManager::notifier()
{
    return globalModemManager;
}

void Solid::Control::ModemManagerPrivate::_k_modemInterfaceAdded(const QString &udi)
{
    QList<QMap<ModemInterface::GsmInterfaceType, ModemInterfaceIfacePair> > list = m_modemInterfaceMap.values(udi);
    m_modemInterfaceMap.remove(udi);

    while (!list.isEmpty()) {
        QMap<ModemInterface::GsmInterfaceType, ModemInterfaceIfacePair> item = list.takeFirst();

        foreach (const ModemInterfaceIfacePair &pair, item) {
            if (pair.first!= 0)
            {
                // Oops, I'm not sure it should happen...
                // But well in this case we'd better kill the old device we got, it's probably outdated
                delete pair.first;
                delete pair.second;
            }
        }
    }

    emit modemInterfaceAdded(udi);
}

void Solid::Control::ModemManagerPrivate::_k_modemInterfaceRemoved(const QString &udi)
{
    emit modemInterfaceRemoved(udi);

    QList<QMap<ModemInterface::GsmInterfaceType, ModemInterfaceIfacePair> > list = m_modemInterfaceMap.values(udi);
    m_modemInterfaceMap.remove(udi);

    while (!list.isEmpty()) {
        QMap<ModemInterface::GsmInterfaceType, ModemInterfaceIfacePair> item = list.takeFirst();

        foreach (const ModemInterfaceIfacePair &pair, item) {
            if (pair.first!= 0)
            {
                delete pair.first;
                delete pair.second;
            }
        }
    }
}

void Solid::Control::ModemManagerPrivate::_k_destroyed(QObject *object)
{
    Ifaces::ModemInterface *device = qobject_cast<Ifaces::ModemInterface *>(object);

    if (device!=0)
    {
        QString udi = device->udi();
        QMap<QString, QMap<ModemInterface::GsmInterfaceType, ModemInterfaceIfacePair> >::iterator i;
        i = m_modemInterfaceMap.begin();
        while (i != m_modemInterfaceMap.end()) {
            if (i.key() != udi) {
                i++;
            }

            foreach (const ModemInterfaceIfacePair &pair, i.value()) {
                if (pair.first != 0 && pair.second == object) // TODO: test if this really works
                {
                    delete pair.first;
                }
            }
            i = m_modemInterfaceMap.erase(i);
        }
    }
}

QPair<Solid::Control::ModemInterface *, QObject *>
Solid::Control::ModemManagerPrivate::findRegisteredModemInterface(const QString &udi, const ModemInterface::GsmInterfaceType ifaceType)
{
    QList<QMap<ModemInterface::GsmInterfaceType, ModemInterfaceIfacePair> > list = m_modemInterfaceMap.values(udi);

    while (!list.isEmpty()) {
        QMap<ModemInterface::GsmInterfaceType, ModemInterfaceIfacePair> item = list.takeFirst();
        if (item.contains(ifaceType)) {
            return item.value(ifaceType);
        }
    }

    Ifaces::ModemManager *backend = qobject_cast<Ifaces::ModemManager *>(managerBackend());
    if (backend!=0)
    {
        QObject * iface = backend->createModemInterface(udi, ifaceType);
        ModemInterface *device = 0;
        if (qobject_cast<Ifaces::ModemGsmCardInterface *>(iface) != 0) {
            device = new ModemGsmCardInterface(iface);
        } else if (qobject_cast<Ifaces::ModemGsmNetworkInterface *>(iface) != 0) {
            device = new ModemGsmNetworkInterface(iface);
        } else { // TODO: Add ModemCdmaInterface, ModemLocationInterface and the other Gsm interfaces.
            kDebug() << "Unhandled network interface: " << udi;
        }
        if (device != 0) {
            ModemInterfaceIfacePair pair(device, iface);
            connect(iface, SIGNAL(destroyed(QObject*)),
                    this, SLOT(_k_destroyed(QObject*)));
            QMap<ModemInterface::GsmInterfaceType, ModemInterfaceIfacePair> map;
            map.insert(ifaceType, pair);
            m_modemInterfaceMap.insertMulti(udi, map);
            return pair;
        }
    }

    return ModemInterfaceIfacePair(0, 0);
}

#include "modemmanager_p.moc"
#include "modemmanager.moc"
