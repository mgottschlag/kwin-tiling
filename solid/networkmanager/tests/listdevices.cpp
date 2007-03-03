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

#include <QApplication>
#include <QStringList>

#include <kdebug.h>
#include <solid/ifaces/networkinterface.h>

#include "nmobject.h"

#include "NetworkManager-network.h"
#include "NetworkManager-networkinterface.h"
#include "NetworkManager-networkmanager.h"

int main( int argc, char** argv )
{
#if 1
    QApplication app( argc, argv );
    NMNetworkManager mgr( 0, QStringList() );
    mgr.networkInterfaces();
    mgr.isNetworkingEnabled();
    NMNetworkInterface * ethernetIface = qobject_cast<NMNetworkInterface*>( mgr.createNetworkInterface( "/org/freedesktop/NetworkManager/Devices/eth0" ) );
    NMNetworkInterface * wifiIface = qobject_cast<NMNetworkInterface*>( mgr.createNetworkInterface( "/org/freedesktop/NetworkManager/Devices/eth1" ) );
	Solid::Ifaces::NetworkInterface * solidIface;
	const QMetaObject * parentMo = wifiIface->metaObject()->superClass();
	kDebug() << parentMo->className() << endl;
    QStringList networks = ethernetIface->networks();

	foreach ( QString netPath, networks )
	{
		kDebug() << "Creating network: " << netPath << endl;
		NMNetwork * network = qobject_cast<NMNetwork*>( wifiIface->createNetwork( netPath ) );
		//if ( netPath == "/org/freedesktop/NetworkManager/Devices/eth1/Networks/banjaxed" )
			network->setActivated( true );
	}

    //kDebug() << "Interface: " <<  netIface->uni() << ", " << netIface->signalStrength() << endl;
    //mgr.setWirelessEnabled( true );
    return app.exec();
#else
    //	QApplication app( argc, argv );
    NMObject obj( argc, argv );
    obj.showDevices();
    NMNetworkManager mgr( 0, QStringList() );
    mgr.networkInterfaces();
    return obj.exec();
#endif
}
