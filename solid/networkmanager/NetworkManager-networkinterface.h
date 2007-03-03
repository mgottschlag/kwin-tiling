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

#ifndef NETWORKMANAGER_NETWORKINTERFACE_H
#define NETWORKMANAGER_NETWORKINTERFACE_H

#include <kdemacros.h>

#include "NetworkManager-network.h"

#include <solid/ifaces/networkinterface.h>

struct NMDBusDeviceProperties {
	QDBusObjectPath path;
	QString interface;
	uint type;
	QString udi;
	bool active;
	uint activationStage;
	QString hardwareAddress;
	int mode;
	int strength;
	bool linkActive;
	int speed;
	uint capabilities;
	uint capabilitiesType;
	QString activeNetPath;
	QStringList networks;
};

Q_DECLARE_METATYPE(NMDBusDeviceProperties)

class NMNetworkInterfacePrivate;

class KDE_EXPORT NMNetworkInterface : public Solid::Ifaces::NetworkInterface
{
Q_OBJECT
Q_INTERFACES(Solid::Ifaces::NetworkInterface)
public:
    NMNetworkInterface( const QString & objectPath );
    virtual ~NMNetworkInterface();
    QString uni() const;
    bool isActive() const;
    Solid::NetworkInterface::Type type() const;
    Solid::NetworkInterface::ConnectionState connectionState() const;
    int signalStrength() const;
    int designSpeed() const;
    bool isLinkUp() const;
    Solid::NetworkInterface::Capabilities capabilities() const;
    QObject *createNetwork( const QString & uni );
    QStringList networks() const;
    // These setters are used to update the interface by the manager
    // in response to DBus signals
    void setProperties( const NMDBusDeviceProperties & );
	// Used for ethernet devices to create the network object implied by
	// NetworkManager using the info returned by getProperties
    void setNetwork( const NMDBusNetworkProperties & );
    void setSignalStrength( int );
    void setCarrierOn( bool );
    void setActive( bool );
    void setActivationStage( int activationStage );
    void addNetwork( const QDBusObjectPath & netPath );
    void removeNetwork( const QDBusObjectPath & netPath );
    void updateNetworkStrength( const QDBusObjectPath & netPath, int strength );
private:
    NMNetworkInterfacePrivate * d;
};

#endif
