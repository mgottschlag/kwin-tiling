#include <QtDBus>
#include <QDBusArgument>
#include <QMetaType>
#include <qdbusextratypes.h>
#include <QList>
#include <kdebug.h>

#include "nmobject.h"

NMObject::NMObject( int argc, char ** argv ) : QCoreApplication( argc, argv )
{

	nmIface = new QDBusInterface( "org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager", "org.freedesktop.NetworkManager", QDBusConnection::systemBus() );
	nmIface->connection().connect( "org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager", "org.freedesktop.NetworkManager", "DeviceStrengthChanged", this, SLOT(deviceStrengthChanged(QDBusObjectPath,int)) );
	nmIface->connection().connect( "org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager", "org.freedesktop.NetworkManager", "WirelessNetworkStrengthChanged", this, SLOT(netStrengthChanged(QDBusObjectPath,QDBusObjectPath,int)) );
//	nmIface->connection().connect( "org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager", "org.freedesktop.NetworkManager", "WirelessNetworkAppeared", this, SLOT(updateNetwork(QDBusObjectPath,QDBusObjectPath)) );
	nmIface->connection().connect( "org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager", "org.freedesktop.NetworkManager", "WirelessNetworkAppeared", this, SLOT(wirelessNetworkAppeared(QDBusObjectPath,QDBusObjectPath)) );
	nmIface->connection().connect( "org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager", "org.freedesktop.NetworkManager", "WirelessNetworkDisappeared", this, SLOT(wirelessNetworkDisappeared(QDBusObjectPath,QDBusObjectPath)) );
}

NMObject::~NMObject( )
{
}

void NMObject::showDevices()
{
	kDebug() << "Hello, world!" << endl;
	QDBusConnection bus = QDBusConnection::systemBus();
	QDBusReply<uint> state = nmIface->call( "state" );
	if ( state.isValid() )
		kDebug() << "State: " << state.value() << endl;
	else
	{
		QDBusError err = state.error();
		kDebug() << "error: " << err.name() << "msg: " << err.message() <<endl;
	}


	QDBusInterface * deviceIface = new QDBusInterface( "org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager", "org.freedesktop.NetworkManager.Devices", bus );
	//QDBusMessage deviceList = deviceIface->call( "getDevices" );
	
	qDBusRegisterMetaType<QList<QDBusObjectPath> >();

	QDBusReply< QList <QDBusObjectPath> > deviceList = deviceIface->call( "getDevices" );
	if ( true )
	{
		kDebug() << "Got device list" << endl; //Signature: " << deviceList.signature() << endl;
		QList <QDBusObjectPath> devices = deviceList.value();
		foreach ( QDBusObjectPath op, devices )
		{
			kDebug() << "  " << op.path() << endl;
		}
	}

}

void NMObject::netStrengthChanged( QDBusObjectPath devPath, QDBusObjectPath netPath, int strength )
{
	kDebug() << "netStrengthChanged() device: " << devPath.path() << " net: " << netPath.path() << " strength: " << strength << endl;
}
void NMObject::deviceStrengthChanged( QDBusObjectPath objPath, int strength )
{
	kDebug() << "deviceStrengthChanged() obj: " << objPath.path() << " strength: " << strength << endl;
}
void NMObject::updateNetwork( QDBusObjectPath objPath, QDBusObjectPath netPath )
{
	kDebug() << "updateNetwork() obj: " << objPath.path() << " net: " << netPath.path() << endl;
}
void NMObject::wirelessNetworkAppeared( QDBusObjectPath objPath, QDBusObjectPath netPath )
{
	kDebug() << "wirelessNetworkAppeared() obj: " << objPath.path() << " net: " << netPath.path() << endl;
}
void NMObject::wirelessNetworkDisappeared( QDBusObjectPath objPath, QDBusObjectPath netPath )
{
	kDebug() << "wirelessNetworkDisappeared() obj: " << objPath.path() << " net: " << netPath.path() << endl;
}

#include "nmobject.moc"
