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

#include "NetworkManager-networkinterface.h"
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
    QMap<QString, NMNetworkInterface*> interfaces;
};

NMNetworkManager::NMNetworkManager( QObject * parent, const QStringList & args )
 : NetworkManager( parent ), d( new NMNetworkManagerPrivate )
{
    #define connectNMToThis( signal, slot ) \
    d->manager.connection().connect( "org.freedesktop.NetworkManager", \
                                     "/org/freedesktop/NetworkManager", \
                                     "org.freedesktop.NetworkManager", \
                                     signal, this, SLOT(slot) );
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
    kDebug() << "NMNetworkManager::networkInterfaces()" << endl;
    QStringList networkInterfaces;

    qDBusRegisterMetaType<QList<QDBusObjectPath> >();

    // wtf does this work when not called on org.freedesktop.NetworkManager.Devices?
    QDBusReply< QList <QDBusObjectPath> > deviceList = d->manager.call( "getDevices" );
    if ( deviceList.isValid() )
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

QObject * NMNetworkManager::createNetworkInterface( const QString & uni )
{
    kDebug() << "NMNetworkManager::createNetworkInterface()" << endl;
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
    kDebug() << "NMNetworkManager::createAuthenticationValidator() implement me" << endl;
    return 0;
}

bool NMNetworkManager::isNetworkingEnabled( ) const
{
#warning implement NMNetworkManager::isNetworkingEnabled()
    kDebug() << "NMNetworkManager::isNetworkingEnabled()" << endl;
    QDBusReply< uint > state = d->manager.call( "state" );
    if ( state.isValid() )
    {
        kDebug() << "  state: " << state.value() << endl;
    }
    return state > 1 ; // HACK, include NetworkManager.h
}

bool NMNetworkManager::isWirelessEnabled() const
{
    kDebug() << "NMNetworkManager::isWirelessEnabled()" << endl;
    QDBusReply< bool > wirelessEnabled = d->manager.call( "getWirelessEnabled" );
    if ( wirelessEnabled.isValid() )
    {
        kDebug() << "  wireless enabled: " << wirelessEnabled.value() << endl;
    }
    return wirelessEnabled.value();
}

void NMNetworkManager::setNetworkingEnabled( bool enabled )
{
    kDebug() << "NMNetworkManager::setNetworkingEnabled()" << endl;
    d->manager.call( enabled ? "wake" : "sleep" ); //TODO Find out the semantics of the optional bool argument to 'sleep'
}

void NMNetworkManager::setWirelessEnabled( bool enabled )
{
    kDebug() << "NMNetworkManager::setWirelessEnabled()" << endl;
    d->manager.call( "setWirelessEnabled", enabled );
}

void NMNetworkManager::notifyHiddenNetwork( const QString & netname )
{
    kDebug() << "NMNetworkManager::notifyHiddenNetwork() implement me" << endl;
}

void NMNetworkManager::receivedDeviceAdded( QDBusObjectPath objpath )
{
    kDebug() << "NMNetworkManager::receivedDeviceAdded()" << endl;
    emit networkInterfaceAdded( objpath.path() );
}

void NMNetworkManager::receivedDeviceRemoved( QDBusObjectPath objpath )
{
    kDebug() << "NMNetworkManager::receivedDeviceRemoved()" << endl;
    emit networkInterfaceRemoved( objpath.path() );
}

void NMNetworkManager::deviceStrengthChanged(QDBusObjectPath devpath, int strength)
{
    kDebug() << "NMNetworkManager::deviceStrengthChanged() ("<< strength << ")" << endl;
    if ( d->interfaces.contains( devpath.path() ) )
        d->interfaces[ devpath.path() ]->setSignalStrength( strength );
}

void NMNetworkManager::networkStrengthChanged(QDBusObjectPath devPath,QDBusObjectPath netPath, int strength )
{
    kDebug() << "NMNetworkManager::networkStrengthChanged(): " << devPath.path() << ", " << netPath.path() << ", " << strength << endl;
}

void NMNetworkManager::wirelessNetworkAppeared( QDBusObjectPath devPath,QDBusObjectPath netPath )
{
    kDebug() << "NMNetworkManager::wirelessNetworkAppeared(): " << devPath.path() << ", " << netPath.path() << endl;
}

void NMNetworkManager::wirelessNetworkDisappeared( QDBusObjectPath devPath,QDBusObjectPath netPath )
{
    kDebug() << "NMNetworkManager::wirelessNetworkDisappeared(): " << devPath.path() << ", " << netPath.path() << endl;
}

void NMNetworkManager::deviceActivationStageChanged( QDBusObjectPath devPath, uint stage )
{
    kDebug() << "NMNetworkManager::deviceActivationStageChanged() " << devPath.path() << " ("<< stage << ")" << endl;
}
void NMNetworkManager::carrierOn(QDBusObjectPath devPath)
{
    kDebug() << "NMNetworkManager::carrierOn(): " << devPath.path() << endl;
}
void NMNetworkManager::carrierOff(QDBusObjectPath devPath)
{
    kDebug() << "NMNetworkManager::carrierOff(): " << devPath.path() << endl;
}
void NMNetworkManager::nowActive(QDBusObjectPath devPath)
{
    kDebug() << "NMNetworkManager::nowActive(): " << devPath.path() << endl;
}
void NMNetworkManager::noLongerActive(QDBusObjectPath devPath)
{
    kDebug() << "NMNetworkManager::noLongerActive(): " << devPath.path() << endl;
}
void NMNetworkManager::activating(QDBusObjectPath devPath)
{
    kDebug() << "NMNetworkManager::activating(): " << devPath.path() << endl;
}
void NMNetworkManager::activationFailed(QDBusObjectPath devPath)
{
    kDebug() << "NMNetworkManager::activationFailed(): " << devPath.path() << endl;
}

// state change
// check for bum input ie devPath
#include "NetworkManager-networkmanager.moc"
