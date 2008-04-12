/*  This file is part of kdebase/workspace/solid
    Copyright (C) 2005,2007 Will Stephenson <wstephenson@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library.  If not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this library
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "networkstatus.h"

#include <QMap>

#include <KDebug>
#include <solid/control/networkmanager.h>

#include "network.h"
#include "clientadaptor.h"
#include "serviceadaptor.h"

#include <kpluginfactory.h>

K_PLUGIN_FACTORY(NetworkStatusFactory,
                 registerPlugin<NetworkStatusModule>();
    )
K_EXPORT_PLUGIN(NetworkStatusFactory("networkstatus"))

// INTERNALLY USED STRUCTS AND TYPEDEFS

typedef QMap< QString, Network * > NetworkMap;

class NetworkStatusModule::Private
{
public:
    Private() : status( Solid::Networking::Unknown )
    {

    }
    ~Private()
    {

    }
    NetworkMap networks;
    Solid::Networking::Status status;
    Solid::Control::NetworkManager::Notifier * notifier;
};

// CTORS/DTORS

NetworkStatusModule::NetworkStatusModule(QObject* parent, const QList<QVariant>&)
    : KDEDModule(parent), d( new Private )
{
    new ClientAdaptor( this );
    new ServiceAdaptor( this );

    QDBusConnection dbus = QDBusConnection::sessionBus();
    QDBusConnectionInterface * sessionBus = dbus.interface();

    connect( sessionBus, SIGNAL(serviceOwnerChanged(const QString&,const QString&,const QString&)), this, SLOT(serviceOwnerChanged(const QString&,const QString&,const QString&)) );
    init();
}

NetworkStatusModule::~NetworkStatusModule()
{
    Q_FOREACH ( Network * net, d->networks ) {
        delete net;
    }

    delete d;
}

// CLIENT INTERFACE

int NetworkStatusModule::status()
{
    kDebug( 1222 ) << " status: " << (int)d->status;
    return (int)d->status;
}

//protected:

void NetworkStatusModule::updateStatus()
{
    Solid::Networking::Status bestStatus = Solid::Networking::Unknown;
    const Solid::Networking::Status oldStatus = d->status;

    Q_FOREACH ( Network * net, d->networks ) {
        if ( net->status() > bestStatus )
            bestStatus = net->status();
    }
    d->status = bestStatus;

    if ( oldStatus != d->status ) {
        emit statusChanged( (uint)d->status );
    }
}

void NetworkStatusModule::serviceOwnerChanged( const QString & name ,const QString & oldOwner, const QString & newOwner )
{
  if ( !oldOwner.isEmpty() && newOwner.isEmpty( ) ) {
    // unregister and delete any networks owned by a service that has just unregistered
    QMutableMapIterator<QString,Network*> it( d->networks );
    while ( it.hasNext() ) {
      it.next();
      if ( it.value()->service() == name )
      {
        kDebug( 1222 ) << "Departing service " << name << " owned network " << it.value()->name() << ", removing it";
        Network * removedNet = it.value();
        it.remove();
        updateStatus();
        delete removedNet;
      }
    }
  }
}

// SERVICE INTERFACE //

QStringList NetworkStatusModule::networks()
{
    if ( d->networks.count() ) {
      kDebug() << "Network status module is aware of " << d->networks.count() << " networks";
    } else {
      kDebug( 1222 ) << "Network status module is not aware of any networks";
    }
    return d->networks.keys();
}

void NetworkStatusModule::setNetworkStatus( const QString & networkName, int st )
{
    kDebug( 1222 ) << networkName << ", " << st;
    Solid::Networking::Status changedStatus = (Solid::Networking::Status)st;
    if ( d->networks.contains( networkName ) ) {
      Network * net = d->networks[ networkName ];
        net->setStatus( changedStatus );
        updateStatus();
    } else {
      kDebug( 1222 ) << "  No network named '" << networkName << "' known.";
    }
}

void NetworkStatusModule::registerNetwork( const QString & networkName, int status, const QString & serviceName )
{
    QDBusConnection dbus = QDBusConnection::sessionBus();
    QDBusConnectionInterface * sessionBus = dbus.interface();
    QString uniqueOwner = sessionBus->serviceOwner( serviceName ).value();

    kDebug( 1222 ) << networkName << ", with status " << status << " is owned by " << uniqueOwner;

    d->networks.insert( networkName, new Network( networkName, status, uniqueOwner ) );
    updateStatus();
}

void NetworkStatusModule::unregisterNetwork( const QString & networkName )
{
    if ( networkName != QLatin1String("SolidNetwork") ) {
        kDebug( 1222 ) << networkName << " unregistered.";

        d->networks.remove( networkName );
        updateStatus();
    }
}

void NetworkStatusModule::init()
{
    d->notifier = Solid::Control::NetworkManager::notifier();
    connect( d->notifier, SIGNAL(statusChanged(Solid::Networking::Status)),
            this, SLOT(solidNetworkingStatusChanged(Solid::Networking::Status)));
    Solid::Networking::Status status = Solid::Control::NetworkManager::status();
    registerNetwork( QLatin1String("SolidNetwork"), status, QLatin1String("org.kde.kded") );
}

void NetworkStatusModule::solidNetworkingStatusChanged( Solid::Networking::Status status )
{
    kDebug( 1222 ) << "SolidNetwork changed status: " << status;
    setNetworkStatus( QLatin1String("SolidNetwork"), status );
}

#include "networkstatus.moc"
// vim: set noet sw=4 ts=4:
