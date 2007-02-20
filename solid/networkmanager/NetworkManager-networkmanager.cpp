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

#include <QtDBus>

#include <kdebug.h>

#include "NetworkManager-networkmanager.h"

Q_DECLARE_METATYPE(QList<QDBusObjectPath>)

class NMNetworkManagerPrivate
{
public:
    NMNetworkManagerPrivate() : manager( "org.freedesktop.NetworkManager",
                                         "/org/freedesktop/NetworkManager",
                                         "org.freedesktop.NetworkManager",
                                         QDBusConnection::systemBus() ) { }
    QDBusInterface manager;
};

NMNetworkManager::NMNetworkManager( QObject * parent, const QStringList & args )
 : NetworkManager( parent ), d( new NMNetworkManagerPrivate )
{
}

NMNetworkManager::~NMNetworkManager()
{
    delete d;
}

QStringList NMNetworkManager::networkInterfaces() const
{
    kDebug() << "NMNetworkManager::networkInterfaces() implement me" << endl;
    QStringList networkInterfaces;

    qDBusRegisterMetaType<QList<QDBusObjectPath> >();

    QDBusReply< QList <QDBusObjectPath> > deviceList = d->manager.call( "getDevices" );
    if ( true )
    {
        kDebug() << "Got device list" << endl; //Signature: " << deviceList.signature() << endl;
        QList <QDBusObjectPath> devices = deviceList.value();
        foreach ( QDBusObjectPath op, devices )
        {
            networkInterfaces.append( op.path() );
            kDebug() << "  " << op.path() << endl;
        }
    }
    return networkInterfaces;
}

QStringList NMNetworkManager::activeNetworkInterfaces() const
{
    kDebug() << "NMNetworkManager::activeNetworkInterfaces() implement me" << endl;
    return QStringList();
}

QObject * NMNetworkManager::createNetworkInterface( const QString & )
{
    kDebug() << "NMNetworkManager::createNetworkInterface() implement me" << endl;
    return 0;
}

QObject * NMNetworkManager::createAuthenticationValidator()
{
    kDebug() << "NMNetworkManager::createAuthenticationValidator() implement me" << endl;
    return 0;
}

bool NMNetworkManager::isNetworkingEnabled( ) const
{
    kDebug() << "NMNetworkManager::isNetworkingEnabled() implement me" << endl;
    return false;
}

bool NMNetworkManager::isWirelessEnabled() const
{
    kDebug() << "NMNetworkManager::isNetworkingEnabled() implement me" << endl;
    return false;
}

void NMNetworkManager::setNetworkingEnabled( bool enabled )
{
    kDebug() << "NMNetworkManager::setNetworkingEnabled() implement me" << endl;
}

void NMNetworkManager::setWirelessEnabled( bool enabled )
{
    kDebug() << "NMNetworkManager::setWirelessEnabled() implement me" << endl;
}

void NMNetworkManager::notifyHiddenNetwork( const QString & netname )
{
    kDebug() << "NMNetworkManager::notifyHiddenNetwork() implement me" << endl;
}
#include "NetworkManager-networkmanager.moc"
