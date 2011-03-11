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

#ifndef WICD_NETWORKMANAGER
#define WICD_NETWORKMANAGER

#include <QObject>
#include <solid/control/ifaces/networkmanager.h>


class WicdNetworkManagerPrivate;
class WicdNetworkManager : public Solid::Control::Ifaces::NetworkManager
{
    Q_INTERFACES(Solid::Control::Ifaces::NetworkManager)
    Q_OBJECT

public:
    WicdNetworkManager(QObject * parent, const QVariantList  & args);
    virtual ~WicdNetworkManager();
    Solid::Networking::Status status() const;
    QStringList networkInterfaces() const;
    QObject * createNetworkInterface(const QString &);
    void activateConnection(const QString & interfaceUni, const QString & connectionUni, const QVariantMap & connectionParameters);
    void deactivateConnection(const QString & activeConnection);

    bool isNetworkingEnabled() const;
    bool isWirelessEnabled() const;
    bool isWirelessHardwareEnabled() const;
    bool isWwanEnabled() const;
    bool isWwanHardwareEnabled() const;
    QStringList activeConnections() const;
    Solid::Control::NetworkInterface::Types supportedInterfaceTypes() const;

public Q_SLOTS:
    void setWirelessEnabled(bool);
    void setWwanEnabled(bool);
    void setNetworkingEnabled(bool);

private Q_SLOTS:
    void refreshStatus();

private:
    WicdNetworkManagerPrivate * d;
};

#endif
