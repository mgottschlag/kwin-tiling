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

#include <NetworkManager/NetworkManager.h>

#include <QtDBus>

#include <kdebug.h>

#include "NetworkManager-networkinterface.h"
#include "NetworkManager-networkmanager.h"

Q_DECLARE_METATYPE(QList<QDBusObjectPath>)

class NMNetworkManagerPrivate
{
public:
    NMNetworkManagerPrivate() : manager( "org.freedesktop.NetworkManager",
                                         "/org/freedesktop/NetworkManager",
                                         "org.freedesktop.NetworkManager",
                                         QDBusConnection::systemBus() ), cachedState( NM_STATE_UNKNOWN ) { }
    QDBusInterface manager;
    QMap<QString, NMNetworkInterface*> interfaces;
    uint cachedState;
};

NMNetworkManager::NMNetworkManager( QObject * parent, const QStringList & /*args*/ )
 : NetworkManager( parent ), d( new NMNetworkManagerPrivate )
{
    #define connectNMToThis( signal, slot ) \
    d->manager.connection().connect( "org.freedesktop.NetworkManager", \
                                     "/org/freedesktop/NetworkManager", \
                                     "org.freedesktop.NetworkManager", \
                                     signal, this, SLOT(slot) );
    connectNMToThis( NM_DBUS_SIGNAL_STATE_CHANGE, stateChanged(uint) );
    connectNMToThis( "DeviceAdded", receivedDeviceAdded(QDBusObjectPath) );
    connectNMToThis( "DeviceRemoved", receivedDeviceRemoved(QDBusObjectPath) );
    connectNMToThis( "DeviceStrengthChanged", deviceStrengthChanged(QDBusObjectPath,int) );
    connectNMToThis( "WirelessNetworkStrengthChanged", networkStrengthChanged(QDBusObjectPath,QDBusObjectPath,int) );
    connectNMToThis( "WirelessNetworkAppeared", wirelessNetworkAppeared(QDBusObjectPath,QDBusObjectPath) );
    connectNMToThis( "WirelessNetworkDisappeared", wirelessNetworkDisappeared(QDBusObjectPath,QDBusObjectPath) );
    connectNMToThis( "DeviceActivationStage", deviceActivationStageChanged(QDBusObjectPath,uint) );

    connectNMToThis( "DeviceCarrierOn", carrierOn(QDBusObjectPath) );
    connectNMToThis( "DeviceCarrierOff", carrierOff(QDBusObjectPath) );
    connectNMToThis( "DeviceNowActive", nowActive(QDBusObjectPath) );
    connectNMToThis( "DeviceNoLongerActive", noLongerActive(QDBusObjectPath) );
    connectNMToThis( "DeviceActivating", activating(QDBusObjectPath) );
    //TODO: find a way to connect to the wireless variant of this, incl essid
    connectNMToThis( "DeviceActivationFailed", activationFailed(QDBusObjectPath) );
}

NMNetworkManager::~NMNetworkManager()
{
    delete d;
}

QStringList NMNetworkManager::networkInterfaces() const
{
    kDebug(1441) << "NMNetworkManager::networkInterfaces()" << endl;
    QStringList networkInterfaces;

    qDBusRegisterMetaType<QList<QDBusObjectPath> >();

    // wtf does this work when not called on org.freedesktop.NetworkManager.Devices?
    QDBusReply< QList <QDBusObjectPath> > deviceList = d->manager.call( "getDevices" );
    if ( deviceList.isValid() )
    {
        kDebug(1441) << "Got device list" << endl;
        QList <QDBusObjectPath> devices = deviceList.value();
        foreach ( QDBusObjectPath op, devices )
        {
            networkInterfaces.append( op.path() );
            kDebug(1441) << "  " << op.path() << endl;
        }
    }
    else
        kDebug( 1441 ) << "Error getting device list: " << deviceList.error().name() << ": " << deviceList.error().message() << endl;
    return networkInterfaces;
}

QStringList NMNetworkManager::activeNetworkInterfaces() const
{
    kDebug(1441) << "NMNetworkManager::activeNetworkInterfaces() implement me" << endl;
    return QStringList();
}

QObject * NMNetworkManager::createNetworkInterface( const QString & uni )
{
    //kDebug(1441) << "NMNetworkManager::createNetworkInterface()" << endl;
    NMNetworkInterface * netInterface;
    if ( d->interfaces.contains( uni ) )
    {
        netInterface = d->interfaces[ uni ];
    }
    else
    {
        netInterface = new NMNetworkInterface( uni );
        d->interfaces.insert( uni, netInterface );
    }
    return netInterface;
}

QObject * NMNetworkManager::createAuthenticationValidator()
{
    kDebug(1441) << "NMNetworkManager::createAuthenticationValidator() implement me" << endl;
    return 0;
}

bool NMNetworkManager::isNetworkingEnabled( ) const
{
    kDebug(1441) << "NMNetworkManager::isNetworkingEnabled()" << endl;
    if ( NM_STATE_UNKNOWN == d->cachedState )
    {
        QDBusReply< uint > state = d->manager.call( "state" );
        if ( state.isValid() )
        {
            kDebug(1441) << "  got state: " << state.value() << endl;
            d->cachedState = state.value();
        }
    }
    return NM_STATE_CONNECTING == d->cachedState || NM_STATE_CONNECTED == d->cachedState || NM_STATE_DISCONNECTED == d->cachedState;
}

bool NMNetworkManager::isWirelessEnabled() const
{
    kDebug(1441) << "NMNetworkManager::isWirelessEnabled()" << endl;
    QDBusReply< bool > wirelessEnabled = d->manager.call( "getWirelessEnabled" );
    if ( wirelessEnabled.isValid() )
    {
        kDebug(1441) << "  wireless enabled: " << wirelessEnabled.value() << endl;
    }
    return wirelessEnabled.value();
}

void NMNetworkManager::setNetworkingEnabled( bool enabled )
{
    kDebug(1441) << "NMNetworkManager::setNetworkingEnabled()" << endl;
    d->manager.call( enabled ? "wake" : "sleep" ); //TODO Find out the semantics of the optional bool argument to 'sleep'
}

void NMNetworkManager::setWirelessEnabled( bool enabled )
{
    kDebug(1441) << "NMNetworkManager::setWirelessEnabled()" << endl;
    d->manager.call( "setWirelessEnabled", enabled );
}

void NMNetworkManager::notifyHiddenNetwork( const QString & /*netname*/ )
{
#warning NMNetworkManager::notifyHiddenNetwork() is unimplemented
    kDebug(1441) << "NMNetworkManager::notifyHiddenNetwork() implement me" << endl;
}

void NMNetworkManager::stateChanged( uint state )
{
    kDebug(1441) << "NMNetworkManager::stateChanged() (" << state << ")" << endl;
    d->cachedState = state;
}

void NMNetworkManager::receivedDeviceAdded( QDBusObjectPath objpath )
{
    kDebug(1441) << "NMNetworkManager::receivedDeviceAdded()" << endl;
    emit networkInterfaceAdded( objpath.path() );
}

void NMNetworkManager::receivedDeviceRemoved( QDBusObjectPath objpath )
{
    kDebug(1441) << "NMNetworkManager::receivedDeviceRemoved()" << endl;
    emit networkInterfaceRemoved( objpath.path() );
}

void NMNetworkManager::deviceStrengthChanged(QDBusObjectPath devPath, int strength)
{
    kDebug(1441) << "NMNetworkManager::deviceStrengthChanged() ("<< strength << ")" << endl;
    NMNetworkInterface * interface = 0;
    if ( d->interfaces.contains( devPath.path() ) && ( interface = d->interfaces[ devPath.path() ] ) != 0 )
        d->interfaces[ devPath.path() ]->setSignalStrength( strength );
}

void NMNetworkManager::networkStrengthChanged(QDBusObjectPath devPath,QDBusObjectPath netPath, int strength )
{
    kDebug(1441) << "NMNetworkManager::networkStrengthChanged(): " << devPath.path() << ", " << netPath.path() << ", " << strength << endl;
    NMNetworkInterface * interface = 0;
    if ( d->interfaces.contains( devPath.path() ) && ( interface = d->interfaces[ devPath.path() ] ) != 0 )
    {
        interface->updateNetworkStrength( netPath, strength );
    }
}

void NMNetworkManager::wirelessNetworkAppeared( QDBusObjectPath devPath,QDBusObjectPath netPath )
{
    kDebug(1441) << "NMNetworkManager::wirelessNetworkAppeared(): " << devPath.path() << ", " << netPath.path() << endl;
    if ( d->interfaces.contains( devPath.path() ) )
    {
        NMNetworkInterface * interface = d->interfaces[ devPath.path() ];
        interface->addNetwork( netPath );
    }
}

void NMNetworkManager::wirelessNetworkDisappeared( QDBusObjectPath devPath,QDBusObjectPath netPath )
{
    kDebug(1441) << "NMNetworkManager::wirelessNetworkDisappeared(): " << devPath.path() << ", " << netPath.path() << endl;
    if ( d->interfaces.contains( devPath.path() ) )
    {
        NMNetworkInterface * interface = d->interfaces[ devPath.path() ];
        interface->removeNetwork( netPath );
    }
}

void NMNetworkManager::deviceActivationStageChanged( QDBusObjectPath devPath, uint stage )
{
    kDebug(1441) << "NMNetworkManager::deviceActivationStageChanged() " << devPath.path() << " ("<< stage << ")" << endl;
    if ( d->interfaces.contains( devPath.path() ) )
        d->interfaces[ devPath.path() ]->setActivationStage( stage );
}

void NMNetworkManager::carrierOn(QDBusObjectPath devPath)
{
    kDebug(1441) << "NMNetworkManager::carrierOn(): " << devPath.path() << endl;
    if ( d->interfaces.contains( devPath.path() ) )
        d->interfaces[ devPath.path() ]->setCarrierOn( true );
}
void NMNetworkManager::carrierOff(QDBusObjectPath devPath)
{
    kDebug(1441) << "NMNetworkManager::carrierOff(): " << devPath.path() << endl;
    if ( d->interfaces.contains( devPath.path() ) )
        d->interfaces[ devPath.path() ]->setCarrierOn( false );
}

void NMNetworkManager::nowActive(QDBusObjectPath devPath)
{
    kDebug(1441) << "NMNetworkManager::nowActive(): " << devPath.path() << endl;
    if ( d->interfaces.contains( devPath.path() ) )
        d->interfaces[ devPath.path() ]->setActive( true );
}

void NMNetworkManager::noLongerActive(QDBusObjectPath devPath)
{
    kDebug(1441) << "NMNetworkManager::noLongerActive(): " << devPath.path() << endl;
    if ( d->interfaces.contains( devPath.path() ) )
        d->interfaces[ devPath.path() ]->setActive( false );
}

void NMNetworkManager::activating(QDBusObjectPath devPath)
{
    kDebug(1441) << "NMNetworkManager::activating(): " << devPath.path() << endl;
    // We don't do anything with this signal as it is duplicated by connectionStateChanged
}

void NMNetworkManager::activationFailed(QDBusObjectPath devPath)
{
    kDebug(1441) << "NMNetworkManager::activationFailed(): " << devPath.path() << endl;
    if ( d->interfaces.contains( devPath.path() ) )
        d->interfaces[ devPath.path() ]->setActivationStage( NM_ACT_STAGE_FAILED );
}

// TODO check for bum input at least to public methods ie devPath
#include "NetworkManager-networkmanager.moc"
