#include <kdebug.h>
#include <QApplication>
#include <QObject>

#include "bluez-bluetoothmanager.h"
#include "bluez-bluetoothinterface.h"
#include "bluez-bluetoothremotedevice.h"

void foobar(const QString&, uint deviceClass, short rssi)
{
    kDebug() << k_funcinfo << endl;
}

int main(int argc, char **argv)
{

    QApplication app(argc, argv);
    BluezBluetoothManager mgr(0, QStringList());

    kDebug() << "Interfaces: " << mgr.bluetoothInterfaces() << endl;
    kDebug() << "Default Interface: " << mgr.defaultInterface() << endl;

    kDebug() << "Bluetooth Input Devices: " << mgr.bluetoothInputDevices() << endl;

    BluezBluetoothInterface iface(mgr.defaultInterface());

//    iface.discoverDevices();

    iface.startPeriodicDiscovery();

#if 0
    BluezBluetoothRemoteDevice remote( "/org/bluez/hci0/00:16:BC:15:A3:FF" );

    kDebug() << "Name: " << remote.name() << endl;
    kDebug() << "Company: " << remote.company() << endl;
    kDebug() << "Services: " << remote.serviceClasses() << endl;
    kDebug() << "Major Class: " << remote.majorClass() << endl;
    kDebug() << "Minor Class: " << remote.minorClass() << endl;

    if ( remote.hasBonding() )
    {
        remote.removeBonding();
    }

    remote.createBonding();


    kDebug() << mgr.setupInputDevice("/org/bluez/hci0/00:04:61:81:75:FF") << endl;
#endif    

    return app.exec();
}
