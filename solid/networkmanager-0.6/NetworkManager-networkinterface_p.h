/*  This file is part of the KDE project
    Copyright (C) 2008 Pino Toscano <pino@kde.org>

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

#ifndef NETWORKMANAGER_NETWORKINTERFACE_P_H
#define NETWORKMANAGER_NETWORKINTERFACE_P_H

#include <QtCore/qstring.h>
#include <QtDBus/QDBusInterface>

class NMNetworkInterface;
class NMDBusDeviceProperties;

class NMNetworkInterfacePrivate
{
public:
    NMNetworkInterfacePrivate(const QString  & objPath);
    virtual ~NMNetworkInterfacePrivate();

    Q_DECLARE_PUBLIC(NMNetworkInterface)

    void initGeneric();

    virtual void notifyNewNetwork(const QDBusObjectPath & netPath) { Q_UNUSED(netPath) }
    virtual void notifyRemoveNetwork(const QDBusObjectPath & netPath) { Q_UNUSED(netPath) }
    virtual void applyProperties(const NMDBusDeviceProperties & props);

    NMNetworkInterface * q_ptr;

    QDBusInterface iface;
    QString objectPath;
    QDBusInterface * manager;
    bool active;
    Solid::Control::NetworkInterface::Type type;
    int activationStage;
    int designSpeed;
    Solid::Control::NetworkInterface::Capabilities capabilities;
    QString activeNetPath;
    QString interface;
    QString driver;
};

#endif
