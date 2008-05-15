/*  This file is part of the KDE project
    Copyright (C) 2007 Will Stephenson <wstephenson@kde.org>
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

#ifndef NETWORKMANAGER_NETWORK_MANAGER_H
#define NETWORKMANAGER_NETWORK_MANAGER_H

#include <solid/control/ifaces/networkmanager.h>

class QDBusObjectPath;
typedef class QList<QVariant> QVariantList;

class NMNetworkManagerPrivate;
class KDE_EXPORT NMNetworkManager : public Solid::Control::Ifaces::NetworkManager
{
Q_INTERFACES(Solid::Control::Ifaces::NetworkManager)
Q_OBJECT
    public:
        NMNetworkManager(QObject * parent, const QVariantList  & args);
        virtual ~NMNetworkManager();
        Solid::Networking::Status status() const;
        QStringList networkInterfaces() const;
        QStringList activeNetworkInterfaces() const;
        QObject * createNetworkInterface(const QString &);
        QObject * createAuthenticationValidator();
        void activateConnection(const QString & interfaceUni, const QString & connectionUni, const QString & extra_connection_parameter);
        void deactivateConnection(const QString & activeConnection);

        bool isNetworkingEnabled() const;
        bool isWirelessEnabled() const;
        bool isWirelessHardwareEnabled() const;
    public Q_SLOTS:
        void setWirelessEnabled(bool);
        void setNetworkingEnabled(bool);
        void notifyHiddenNetwork(const QString  &);
    protected Q_SLOTS:
        void stateChanged(uint);
        void receivedDeviceAdded(const QDBusObjectPath &);
        void receivedDeviceRemoved(const QDBusObjectPath &);
        void deviceStrengthChanged(const QDBusObjectPath &, int strength);
        void networkStrengthChanged(const QDBusObjectPath &, const QDBusObjectPath &,int);
        void wirelessNetworkAppeared(const QDBusObjectPath &, const QDBusObjectPath &);
        void wirelessNetworkDisappeared(const QDBusObjectPath &, const QDBusObjectPath &);
        void deviceActivationStageChanged(const QDBusObjectPath &, uint);
        void carrierOn(const QDBusObjectPath &);
        void carrierOff(const QDBusObjectPath &);
        void nowActive(const QDBusObjectPath &);
        void noLongerActive(const QDBusObjectPath &);
        void activating(const QDBusObjectPath &);
        void activationFailed(const QDBusObjectPath &);
    private:
        NMNetworkManagerPrivate * d;
};

#endif

