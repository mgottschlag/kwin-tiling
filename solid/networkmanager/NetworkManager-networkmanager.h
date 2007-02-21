/*  This file is part of the KDE project
    Copyright (C) 2007 Will Stephenson <wstephenson@kde.org>

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

#include <QObject>
#include <qdbusextratypes.h>
#include <QStringList>

#include <kdemacros.h>

#include <solid/ifaces/networkmanager.h>

class NMNetworkManagerPrivate;
class KDE_EXPORT NMNetworkManager : public Solid::Ifaces::NetworkManager
{
Q_OBJECT
Q_INTERFACES(Solid::Ifaces::NetworkManager)
    public:
        NMNetworkManager( QObject * parent, const QStringList & args );
        virtual ~NMNetworkManager();
        QStringList networkInterfaces() const;
        QStringList activeNetworkInterfaces() const;
        QObject * createNetworkInterface( const QString &);
        QObject * createAuthenticationValidator();

        bool isNetworkingEnabled() const;
        bool isWirelessEnabled() const;
    public Q_SLOTS:
        void setWirelessEnabled( bool );
        void setNetworkingEnabled( bool );
        void notifyHiddenNetwork( const QString & );
    protected Q_SLOTS:
        void receivedDeviceAdded( QDBusObjectPath );
        void receivedDeviceRemoved( QDBusObjectPath );
    private:
        NMNetworkManagerPrivate * d;
};

#endif

