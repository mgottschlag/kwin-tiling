/*  This file is part of the KDE project
    Copyright (C) 2006 Will Stephenson <wstephenson@kde.org>
    Copyright (C) 2006-2007 Kevin Ottens <ervin@kde.org>
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
#ifndef SOLID_MODEMMANAGER_P_H
#define SOLID_MODEMMANAGER_P_H

#include <QObject>
#include <QMap>
#include <QPair>

#include "managerbase_p.h"

#include "modemmanager.h"
#include "modemmanagerinterface.h"

namespace Solid
{
namespace Control
{
    namespace Ifaces
    {
        class ModemManagerInterface;
    }

    class ModemManagerPrivate : public ModemManager::Notifier, public ManagerBasePrivate
    {
        Q_OBJECT
    public:
        ModemManagerPrivate();
        ~ModemManagerPrivate();

        ModemInterfaceList modemInterfaces();
        ModemInterface *findModemInterface(const QString &udi, const ModemInterface::GsmInterfaceType ifaceType);

    private Q_SLOTS:
        void _k_modemInterfaceAdded(const QString &udi);
        void _k_modemInterfaceRemoved(const QString &udi);
        void _k_destroyed(QObject *object);

    private:
        typedef QPair<ModemInterface *, QObject *> ModemInterfaceIfacePair;

        ModemInterfaceList buildDeviceList(const QStringList &udiList);
        ModemInterfaceIfacePair findRegisteredModemInterface(const QString &udi, const ModemInterface::GsmInterfaceType ifaceType);

        QMap<QString, QMap<ModemInterface::GsmInterfaceType, ModemInterfaceIfacePair> > m_modemInterfaceMap;
        ModemInterface m_invalidDevice;
    };
}
}

#endif
