/*  This file is part of the KDE project
    Copyright (C) 2006 Kevin Ottens <ervin@kde.org>

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

#include "solid-bluetooth.h"


#include <QString>
#include <QStringList>
#include <QMetaProperty>
#include <QMetaEnum>
#include <QTimer>

#include <kcomponentdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <k3socketaddress.h>
#include <kdebug.h>

#include <solid/device.h>
#include <solid/genericinterface.h>
#include <solid/storageaccess.h>
#include <solid/opticaldrive.h>

#include <solid/control/bluetoothmanager.h>
#include <solid/control/bluetoothinterface.h>
#include <solid/control/bluetoothremotedevice.h>
#include <solid/control/bluetoothinputdevice.h>

#include <kjob.h>


#include <iostream>
using namespace std;

static const char appName[] = "solid-bluetooth";
static const char programName[] = I18N_NOOP("solid-bluetooth");

static const char description[] = I18N_NOOP("KDE tool for querying and controlling your hardware from the command line");

static const char version[] = "0.1";

std::ostream &operator<<(std::ostream &out, const QString &msg)
{
    return (out << msg.toLocal8Bit().constData());
}

std::ostream &operator<<(std::ostream &out, const QVariant &value)
{
    switch (value.type())
    {
    case QVariant::StringList:
    {
        out << "{";

        QStringList list = value.toStringList();

        QStringList::ConstIterator it = list.begin();
        QStringList::ConstIterator end = list.end();

        for (; it!=end; ++it)
        {
            out << "'" << *it << "'";

            if (it+1!=end)
            {
                out << ", ";
            }
        }

        out << "}  (string list)";
        break;
    }
    case QVariant::Bool:
        out << (value.toBool()?"true":"false") << "  (bool)";
        break;
    case QVariant::Int:
        out << value.toString()
            << "  (0x" << QString::number(value.toInt(), 16) << ")  (int)";
        break;
    default:
        out << "'" << value.toString() << "'  (string)";
        break;
    }

    return out;
}

std::ostream &operator<<(std::ostream &out, const Solid::Device &device)
{
    out << "  parent = " << QVariant(device.parentUdi()) << endl;
    out << "  vendor = " << QVariant(device.vendor()) << endl;
    out << "  product = " << QVariant(device.product()) << endl;

    int index = Solid::DeviceInterface::staticMetaObject.indexOfEnumerator("Type");
    QMetaEnum typeEnum = Solid::DeviceInterface::staticMetaObject.enumerator(index);

    for (int i=0; i<typeEnum.keyCount(); i++)
    {
        Solid::DeviceInterface::Type type = (Solid::DeviceInterface::Type)typeEnum.value(i);
        const Solid::DeviceInterface *interface = device.asDeviceInterface(type);

        if (interface)
        {
            const QMetaObject *meta = interface->metaObject();

            for (int i=meta->propertyOffset(); i<meta->propertyCount(); i++)
            {
                QMetaProperty property = meta->property(i);
                out << "  " << QString(meta->className()).mid(7) << "." << property.name()
                    << " = ";

                QVariant value = property.read(interface);

                if (property.isEnumType()) {
                    QMetaEnum metaEnum = property.enumerator();
                    out << "'" << metaEnum.valueToKeys(value.toInt()).constData() << "'"
                        << "  (0x" << QString::number(value.toInt(), 16) << ")  ";
                    if (metaEnum.isFlag()) {
                        out << "(flag)";
                    } else {
                        out << "(enum)";
                    }
                    out << endl;
                } else {
                    out << value << endl;
                }
            }
        }
    }

    return out;
}

std::ostream &operator<<(std::ostream &out, const QMap<QString,QVariant> &properties)
{
    foreach (QString key, properties.keys())
    {
        out << "  " << key << " = " << properties[key] << endl;
    }

    return out;
}

void checkArgumentCount(int min, int max)
{
    int count = KCmdLineArgs::parsedArgs()->count();

    if (count < min)
    {
        cerr << i18n("Syntax Error: Not enough arguments") << endl;
        ::exit(1);
    }

    if ((max > 0) && (count > max))
    {
        cerr << i18n("Syntax Error: Too many arguments") << endl;
        ::exit(1);
    }
}

int main(int argc, char **argv)
{
  KCmdLineArgs::init(argc, argv, appName, 0, ki18n(programName), version, ki18n(description), false);


  KCmdLineOptions options;

  options.add("commands", ki18n("Show available commands by domains"));

  options.add("+command", ki18n("Command (see --commands)"));

  options.add("+[arg(s)]", ki18n("Arguments for command"));

  KCmdLineArgs::addCmdLineOptions(options);

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  KComponentData componentData(appName);

  if (args->isSet("commands"))
  {
      KCmdLineArgs::enable_i18n();

      cout << endl << i18n("Syntax:") << endl << endl;

      cout << "  solid-bluetooth listadapters" << endl;
      cout << i18n("             # List bluetooth adapters/interfaces\n") << endl;

      cout << "  solid-bluetooth defaultadapter" << endl;
      cout << i18n("             # List bluetooth default adapter/interface\n") << endl;

      cout << "  solid-bluetooth query (address|bondings|connections|name) (interface 'ubi')" << endl;
      cout << i18n("             # Query information about the bluetooth adapter/interface with 'ubi'\n") << endl;

      cout << "  solid-bluetooth set (mode|name) (interface 'ubi') 'value'" << endl;
      cout << i18n("             # Set the bluetooth adapter name.\n"
                    "             # Set the bluetooth adapter mode. Where 'value' is one of:\n"
                    "             # off|connectable|discoverable\n") << endl;

      cout << "  solid-bluetooth scan (interface 'ubi')" << endl;
      cout << i18n("             # Scan for bluetooth remote devices.\n") << endl;

      cout << "  solid-bluetooth input listdevices" << endl;
      cout << i18n("             # List configured input devices.\n") << endl;

      cout << "  solid-bluetooth input (setup|remove|connect|disconnect) (device 'ubi')" << endl;
      cout << i18n("             # Setup bluetooth input device.\n"
                    "             # Remove configuration of remote input device.\n"
                    "             # Connect or disconnect bluetooth input device.\n") << endl;

      cout << "  solid-bluetooth remote (createbonding|removebonding|hasbonding) (device 'ubi')" << endl;
      cout << i18n("             # Create bonding (pairing) with bluetooth remote device.\n"
                    "             # Remove bonding of bluetooth remote device.\n"
                    "             # Check for bonding of bluetooth remote device.\n") << endl;

      return 0;
  }

  return SolidBluetooth::doIt() ? 0 : 1;
}

bool SolidBluetooth::doIt()
{
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    checkArgumentCount(1, 0);

    QString command(args->arg(0));

    int fake_argc = 0;
    char **fake_argv = 0;
    SolidBluetooth shell(fake_argc, fake_argv);

    if (command == "listadapters")
    {
        return shell.bluetoothListAdapters();
    }
    else if (command == "defaultadapter")
    {
        return shell.bluetoothDefaultAdapter();
    }
    else if (command == "set")
    {
        checkArgumentCount(4, 4);
        QString what(args->arg(1));
        QString ubi(args->arg(2));
        QString value(args->arg(3));

        if (what == "name")
        {
            return shell.bluetoothAdapterSetName(ubi, value);
        }
        else if (what == "mode")
        {
            return shell.bluetoothAdapterSetMode(ubi, value);
        }

    }
    else if (command == "query")
    {
        checkArgumentCount(3, 3);
        QString what(args->arg(1));
        QString ubi(args->arg(2));

        if (what == "mode")
        {
            return shell.bluetoothAdapterMode(ubi);
        }
        else if (what == "address")
        {
            return shell.bluetoothAdapterAddress(ubi);
        }
        else if (what == "name")
        {
            return shell.bluetoothAdapterName(ubi);
        }
        else if (what == "connections")
        {
            return shell.bluetoothAdapterListConnections(ubi);
        }
        else if (what == "bondings")
        {
            return shell.bluetoothAdapterListBondings(ubi);
        }

    }
    else if (command == "scan")
    {
        checkArgumentCount(2, 2);
        QString ubi (args->arg(1));
        return shell.bluetoothAdapterScan(ubi);
    }
    else if (command == "input")
    {
        checkArgumentCount(2, 3);
        QString what (args->arg(1));

        if (what == "listdevices")
        {
            return shell.bluetoothInputListDevices();
        }

        checkArgumentCount(3, 3);
        QString ubi (args->arg(2));

        if (what == "setup")
        {
            return shell.bluetoothInputSetup(ubi);
        }
        else if (what == "remove")
        {
            return shell.bluetoothInputRemoveSetup(ubi);
        }
        else if (what == "connect")
        {
            return shell.bluetoothInputConnect(ubi);
        }
        else if (what == "disconnect")
        {
            return shell.bluetoothInputDisconnect(ubi);
        }
    }
    else if (command == "remote" && args->count() >= 3)
    {
        checkArgumentCount(4, 4);
        QString what (args->arg(1));
        QString adapter (args->arg(2));
        QString remote (args->arg(3));

        if (what == "createbonding")
        {
            return shell.bluetoothRemoteCreateBonding(adapter, remote);
        }
        else if (what == "removebonding")
        {
            return shell.bluetoothRemoteRemoveBonding(adapter, remote);
        }
        else if (what == "hasbonding")
        {
            return shell.bluetoothRemoteHasBonding(adapter, remote);
        }

    }
    else
    {
        cerr << i18n("Syntax Error: Unknown command '%1'" , command) << endl;
    }

    return false;
}

bool SolidBluetooth::bluetoothListAdapters()
{
    Solid::Control::BluetoothManager &manager = Solid::Control::BluetoothManager::self();

    const Solid::Control::BluetoothInterfaceList all = manager.bluetoothInterfaces();

    foreach (const Solid::Control::BluetoothInterface device, all)
    {
        cout << "UBI = '" << device.ubi() << "'" << endl;
    }
    return true;
}

bool SolidBluetooth::bluetoothDefaultAdapter()
{
    Solid::Control::BluetoothManager &manager = Solid::Control::BluetoothManager::self();

    cout << "UBI = '" <<  manager.defaultInterface() << "'" << endl;

    return true;
}

bool SolidBluetooth::bluetoothAdapterAddress(const QString &ubi)
{
    Solid::Control::BluetoothManager &manager = Solid::Control::BluetoothManager::self();
    Solid::Control::BluetoothInterface adapter = manager.findBluetoothInterface(ubi);

    cout << "'" <<  adapter.address() << "'" << endl;

    return true;
}

bool SolidBluetooth::bluetoothAdapterName(const QString &ubi)
{
    Solid::Control::BluetoothManager &manager = Solid::Control::BluetoothManager::self();
    Solid::Control::BluetoothInterface adapter = manager.findBluetoothInterface(ubi);

    cout << "'" <<  adapter.name() << "'" << endl;

    return true;
}

bool SolidBluetooth::bluetoothAdapterSetName(const QString &ubi, const QString &name)
{
    Solid::Control::BluetoothManager &manager = Solid::Control::BluetoothManager::self();
    Solid::Control::BluetoothInterface adapter = manager.findBluetoothInterface(ubi);

    adapter.setName(name);

    return true;
}

bool SolidBluetooth::bluetoothAdapterMode(const QString &ubi)
{
    Solid::Control::BluetoothManager &manager = Solid::Control::BluetoothManager::self();
    Solid::Control::BluetoothInterface adapter = manager.findBluetoothInterface(ubi);

    cout << "'" <<  adapter.mode() << "'" << endl;

    return true;
}

bool SolidBluetooth::bluetoothAdapterSetMode(const QString &ubi, const QString &mode)
{
    Solid::Control::BluetoothManager &manager = Solid::Control::BluetoothManager::self();
    Solid::Control::BluetoothInterface adapter = manager.findBluetoothInterface(ubi);
    Solid::Control::BluetoothInterface::Mode modeEnum(Solid::Control::BluetoothInterface::Off);
    if (mode == "off")
    {
        modeEnum = Solid::Control::BluetoothInterface::Off;
    }
    else if (mode == "connectable")
    {
        modeEnum = Solid::Control::BluetoothInterface::Connectable;
    }
    else if (mode == "discoverable")
    {
        modeEnum = Solid::Control::BluetoothInterface::Discoverable;
    }
    adapter.setMode(modeEnum);

    return true;
}

bool SolidBluetooth::bluetoothAdapterListConnections(const QString &ubi)
{
    Solid::Control::BluetoothManager &manager = Solid::Control::BluetoothManager::self();
    Solid::Control::BluetoothInterface adapter = manager.findBluetoothInterface(ubi);

    const Solid::Control::BluetoothRemoteDeviceList all = adapter.listConnections();

    cout << "Current connections of Bluetooth Adapter: " << ubi << endl;
    foreach (const Solid::Control::BluetoothRemoteDevice device, all)
    {
        cout << "UBI = '" << device.ubi() << "'" << endl;
    }
    return true;
}

bool SolidBluetooth::bluetoothAdapterListBondings(const QString &ubi)
{
    Solid::Control::BluetoothManager &manager = Solid::Control::BluetoothManager::self();
    Solid::Control::BluetoothInterface adapter = manager.findBluetoothInterface(ubi);

    const QStringList all = adapter.listBondings();

    cout << "Current bonded/paired remote bluetooth devices of Bluetooth Adapter: " << ubi << endl;
    foreach (const QString device, all)
    {
        cout << "UBI = '" << device << "'" << endl;
    }
    return true;
}

bool SolidBluetooth::bluetoothAdapterScan(const QString &ubi)
{
    Solid::Control::BluetoothManager &manager = Solid::Control::BluetoothManager::self();
    Solid::Control::BluetoothInterface adapter = manager.findBluetoothInterface(ubi);

    connect(&adapter, SIGNAL(remoteDeviceFound(const QString &, int, int)),
           this, SLOT(slotBluetoothDeviceFound(const QString &, int, int)));
    connect(&adapter, SIGNAL(discoveryCompleted()),
           this, SLOT(slotBluetoothDiscoveryCompleted()));

    adapter.discoverDevices();
    // Workaround for the fakebluetooth backend... quit the discovery after 30 seconds
    QTimer::singleShot(30000, this, SLOT(slotBluetoothDiscoveryCompleted()));
    cout << "Searching ..." << endl;
    m_loop.exec();

    return true;
}

void SolidBluetooth::slotBluetoothDeviceFound(const QString &ubi, int deviceClass, int rssi)
{
    cout << QString("['%1','%2','%3']").arg(ubi).arg(deviceClass).arg(rssi) << endl;
}

void SolidBluetooth::slotBluetoothDiscoveryCompleted()
{
    kDebug() ;
    m_loop.exit();
}

bool SolidBluetooth::bluetoothInputListDevices()
{
    Solid::Control::BluetoothManager &manager = Solid::Control::BluetoothManager::self();
    const Solid::Control::BluetoothInputDeviceList all = manager.bluetoothInputDevices();

    foreach (const Solid::Control::BluetoothInputDevice device, all)
    {
        cout << "UBI = '" << device.ubi() << "'" << endl;
    }

    return true;
}

bool SolidBluetooth::bluetoothInputSetup(const QString &deviceUbi)
{
    Solid::Control::BluetoothManager &manager = Solid::Control::BluetoothManager::self();
    KJob *job = manager.setupInputDevice(deviceUbi);

    if (job==0)
    {
        cerr << i18n("Error: unsupported operation!") << endl;
        return false;
    }

    connectJob(job);

    job->start();
    m_loop.exec();

    if (m_error)
    {
        cerr << i18n("Error: %1" , m_errorString) << endl;
        return false;
    }

    return true;
}

bool SolidBluetooth::bluetoothInputRemoveSetup(const QString &deviceUbi)
{
    Solid::Control::BluetoothManager &manager = Solid::Control::BluetoothManager::self();

    manager.removeInputDevice(deviceUbi);

    return true;
}

bool SolidBluetooth::bluetoothInputConnect(const QString &deviceUbi)
{
    Solid::Control::BluetoothManager &manager = Solid::Control::BluetoothManager::self();
    Solid::Control::BluetoothInputDevice device = manager.findBluetoothInputDevice(deviceUbi);

    device.slotConnect();

    return true;
}

bool SolidBluetooth::bluetoothInputDisconnect(const QString &deviceUbi)
{
    Solid::Control::BluetoothManager &manager = Solid::Control::BluetoothManager::self();
    Solid::Control::BluetoothInputDevice device = manager.findBluetoothInputDevice(deviceUbi);

    device.slotDisconnect();

    return true;
}

bool SolidBluetooth::bluetoothRemoteCreateBonding(const QString &adapterUbi, const QString &deviceUbi)
{
    Solid::Control::BluetoothManager &manager = Solid::Control::BluetoothManager::self();
    Solid::Control::BluetoothInterface adapter = manager.findBluetoothInterface(adapterUbi);
    Solid::Control::BluetoothRemoteDevice device = adapter.findBluetoothRemoteDevice(deviceUbi);

    KJob *job = device.createBonding();

    connectJob(job);

    job->start();
    m_loop.exec();

    if (m_error)
    {
        cerr << i18n("Error: %1" , m_errorString) << endl;
        return false;
    }

    return true;
}

bool SolidBluetooth::bluetoothRemoteRemoveBonding(const QString &adapterUbi, const QString &deviceUbi)
{
    Solid::Control::BluetoothManager &manager = Solid::Control::BluetoothManager::self();
    Solid::Control::BluetoothInterface adapter = manager.findBluetoothInterface(adapterUbi);
    Solid::Control::BluetoothRemoteDevice device = adapter.findBluetoothRemoteDevice(deviceUbi);

    device.removeBonding();

    return true;
}

bool SolidBluetooth::bluetoothRemoteHasBonding(const QString &adapterUbi, const QString &deviceUbi)
{
    Solid::Control::BluetoothManager &manager = Solid::Control::BluetoothManager::self();
    Solid::Control::BluetoothInterface adapter = manager.findBluetoothInterface(adapterUbi);
    Solid::Control::BluetoothRemoteDevice device = adapter.findBluetoothRemoteDevice(deviceUbi);

    if (device.hasBonding())
    {
        cout << "'" << deviceUbi << "' is bonded/paired." << endl;
    } else {
        cout << "'" << deviceUbi << "' is not bonded/paired." << endl;
    }

    return true;
}

void SolidBluetooth::connectJob(KJob *job)
{
    connect(job, SIGNAL(result(KJob *)),
             this, SLOT(slotResult(KJob *)));
    connect(job, SIGNAL(percent(KJob *, unsigned long)),
             this, SLOT(slotPercent(KJob *, unsigned long)));
    connect(job, SIGNAL(infoMessage(KJob *, const QString &, const QString &)),
             this, SLOT(slotInfoMessage(KJob *, const QString &)));
}

void SolidBluetooth::slotPercent(KJob */*job */, unsigned long percent)
{
    cout << i18n("Progress: %1%" , percent) << endl;
}

void SolidBluetooth::slotInfoMessage(KJob */*job */, const QString &message)
{
    cout << i18n("Info: %1" , message) << endl;
}

void SolidBluetooth::slotResult(KJob *job)
{
    m_error = 0;

    if (job->error())
    {
        m_error = job->error();
        m_errorString = job->errorString();
    }

    m_loop.exit();
}

void SolidBluetooth::slotStorageResult(Solid::ErrorType error, const QVariant &errorData)
{
    if (error) {
        m_error = 1;
        m_errorString = errorData.toString();
    }
    m_loop.exit();
}

#include "solid-bluetooth.moc"
