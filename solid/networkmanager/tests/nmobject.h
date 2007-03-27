// object to handle network manager events
#ifndef WILL_NM_OBJECT
#define WILL_NM_OBJECT

#include <QCoreApplication>
#include <QStringList>
#include <qdbusextratypes.h>

class QDBusInterface;

struct NMDevice {
	QDBusObjectPath path;
	QString interface;
	uint type;
	QString udi;
	bool active;
	uint activationStage;
	QString ipv4Address;
	QString subnetMask;
	QString broadcast;
	QString hardwareAddress;
	QString route;
	QString primaryDNS;
	QString secondaryDNS;
	int mode;
	int strength;
	bool linkActive;
	int speed;
	uint capabilities;
	uint capabilitiesType;
	QString activeNetPath;
	QStringList networks;
};
Q_DECLARE_METATYPE(NMDevice)

class NMObject : public QCoreApplication
{
Q_OBJECT
	public:
	NMObject( int argc, char ** argv );
	~NMObject();
	
	void showDevices();
	public slots:
	void updateNetwork(QDBusObjectPath,QDBusObjectPath);
	void deviceStrengthChanged(QDBusObjectPath,int);
	void netStrengthChanged(QDBusObjectPath,QDBusObjectPath,int);
	void wirelessNetworkAppeared(QDBusObjectPath,QDBusObjectPath);
	void wirelessNetworkDisappeared(QDBusObjectPath,QDBusObjectPath);
	private:
	QDBusInterface * nmIface;
	QDBusInterface * nmDeviceIface;
};

#endif
	
