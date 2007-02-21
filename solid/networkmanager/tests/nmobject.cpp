#include <QtDBus>
#include <QDBusArgument>
#include <QMetaType>
#include <qdbusextratypes.h>
#include <QList>
#include <kdebug.h>

#include "nmobject.h"

void dump( const NMDevice & device )
{
	kDebug() << "Object path: " << device.path.path() << "\nInterface: " << device.interface
		<< "\nType: " << device.type << "\nUdi: " << device.udi << "\nActive: "<< device.active
		<< "\nActivation stage: " << device.activationStage << "\nIPV4 address: " << device.ipv4Address
		<< "\nsubnet mask: " << device.subnetMask << "\nBroadcast: " << device.broadcast
		<< "\nroute: " << device.route << "\nprimary dns: " << device.primaryDNS 
		<< "\nsecondary dns: " << device.secondaryDNS << "\nmode: " << device.mode 
		<< "\nStrength: " << device.strength << "\nLink active: " << device.linkActive
		<< "\nSpeed: " << device.speed << "\nCapabilities: " << device.capabilities 
		<< "\nCapabilities type: " << device.capabilitiesType << "\nactive net path: "
		<< device.activeNetPath << "\nNetworks:" << device.networks << endl;
}

void deserialize( const QDBusMessage &message, NMDevice & device )
{
	kDebug() << /*"deserialize args: " << message.arguments() << */"signature: " << message.signature() << endl;
	QList<QVariant> args = message.arguments();
	device.path.setPath( args.takeFirst().toString() );
	device.interface = args.takeFirst().toString();
	device.type = args.takeFirst().toUInt();
	device.udi = args.takeFirst().toString();
	device.active = args.takeFirst().toBool();
	device.activationStage = args.takeFirst().toUInt();
	device.ipv4Address = args.takeFirst().toString();
	device.subnetMask = args.takeFirst().toString();
	device.broadcast = args.takeFirst().toString();
	device.hardwareAddress = args.takeFirst().toString();
	device.route = args.takeFirst().toString();
	device.primaryDNS = args.takeFirst().toString();
	device.secondaryDNS = args.takeFirst().toString();
	device.mode = args.takeFirst().toInt();
	device.strength = args.takeFirst().toInt();
	device.linkActive = args.takeFirst().toBool();
	device.speed = args.takeFirst().toInt();
	device.capabilities = args.takeFirst().toUInt();
	device.capabilitiesType = args.takeFirst().toUInt();
	device.activeNetPath = args.takeFirst().toString();
	device.networks = args.takeFirst().toStringList();
	kDebug() << "deserialize: objpath is " << device.path.path() << "'," << device.interface << endl;
}

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
		kDebug() << "NMObject::showDevices() Got device list" << endl; //Signature: " << deviceList.signature() << endl;
		QList <QDBusObjectPath> devices = deviceList.value();
		foreach ( QDBusObjectPath op, devices )
		{
			kDebug() << "  " << op.path() << endl;
			QDBusInterface * deviceIface2 = new QDBusInterface( "org.freedesktop.NetworkManager", op.path(), "org.freedesktop.NetworkManager.Devices", bus );
			QDBusMessage reply = deviceIface2->call( "getProperties" );
			//kDebug() << "  getProperties call" << ( reply.isValid() ? "is" : "is not" ) << " valid." << endl;
			NMDevice dev;
			deserialize( reply, dev );
			dump( dev );
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
