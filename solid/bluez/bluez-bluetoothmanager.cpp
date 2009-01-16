/*  This file is part of the KDE project
    Copyright (C) 2007 Will Stephenson <wstephenson@kde.org>
    Copyright (C) 2007 Daniel Gollub <dgollub@suse.de>
    Copyright (C) 2008 Tom Patzig <tpatzig@suse.de>


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

#include "bluez-bluetoothmanager.h"

#include <QtDBus>

#include <kdebug.h>

#include "bluez-bluetoothinterface.h"
#include "bluez-bluetoothinputdevice.h"
#include "bluez-bluetoothsecurity.h"
#include "bluez-bluetoothsecurityadaptor.h"
#include "bluezcalljob.h"

class BluezBluetoothManagerPrivate
{
public:


    BluezBluetoothManagerPrivate() : manager("org.bluez", "/", "org.bluez.Manager", QDBusConnection::systemBus())
    {}

    QDBusInterface manager;
//  QDBusInterface *inputManager;

    QMap<QString, BluezBluetoothInterface *> interfaces;
//  QMap<QString, BluezBluetoothInputDevice *> inputDevices;

};

BluezBluetoothManager::BluezBluetoothManager(QObject * parent, const QStringList  & /*args */)
        : BluetoothManager(parent), d(new BluezBluetoothManagerPrivate)
{
#define connectManagerToThis(signal, slot) \
    d->manager.connection().connect("org.bluez", \
                                     "/", \
                                     "org.bluez.Manager", \
                                     signal, this, SLOT(slot));
    connectManagerToThis("AdapterAdded", slotDeviceAdded(const QDBusObjectPath &));
    connectManagerToThis("AdapterRemoved", slotDeviceRemoved(const QDBusObjectPath &));
    connectManagerToThis("DefaultAdapterChanged", slotDefaultDeviceChanged(const QDBusObjectPath &));

// in bluez4 no input Manager needed

/*  QDBusReply< QString > busId = d->manager.call("ActivateService", "input");
    if (busId.isValid()) {
        m_inputManagerDest = busId.value();
    }


    d->inputManager = new QDBusInterface(m_inputManagerDest, "/org/bluez/input",
                                         "org.bluez.input.Manager", QDBusConnection::systemBus());

#define connectInputManagerToThis(signal, slot) \
    d->inputManager->connection().connect(m_inputManagerDest, \
                                           "/org/bluez/input", \
                                           "org.bluez.input.Manager", \
                                           signal, this, SLOT(slot));

    connectInputManagerToThis("DeviceCreated", inputDeviceCreated(const QString &));
    connectInputManagerToThis("DeviceRemoved", inputDeviceRemoved(const QString &));
*/
}

BluezBluetoothManager::~BluezBluetoothManager()
{
//  delete d->inputManager;
    delete d;
}

QStringList BluezBluetoothManager::bluetoothInterfaces() const
{
    QStringList bluetoothInterfaces;

    QDBusReply< QList<QDBusObjectPath> > deviceList = d->manager.call("ListAdapters");
    if (deviceList.isValid()) {
        QList<QDBusObjectPath> devices = deviceList.value();
        foreach (const QDBusObjectPath& path, devices) {
            bluetoothInterfaces.append(path.path());
        }
    }
    return bluetoothInterfaces;
}

QString BluezBluetoothManager::defaultInterface() const
{
    kDebug() << "Calling Backend Default Interface";
    QDBusReply< QDBusObjectPath > path = d->manager.call("DefaultAdapter");
    if (!path.isValid())
        return QString();

    return path.value().path();
}

QString BluezBluetoothManager::findInterface(const QString &adapterName) const
{
    QDBusReply< QDBusObjectPath > devicePath = d->manager.call("FindAdapter",adapterName);
    if (!devicePath.isValid())
        return QString();

    return devicePath.value().path();
}

QObject * BluezBluetoothManager::createInterface(const QString  & ubi)
{
    BluezBluetoothInterface * bluetoothInterface;
    if (d->interfaces.contains(ubi)) {
        bluetoothInterface = d->interfaces[ubi];
    } else {
        bluetoothInterface = new BluezBluetoothInterface(ubi);
        d->interfaces.insert(ubi, bluetoothInterface);
    }
    return bluetoothInterface;
}

void BluezBluetoothManager::removeInterface(const QString& ubi)
{

    if (d->interfaces.contains(ubi)) {
        kDebug() << "Removing Interface" << ubi;
        BluezBluetoothInterface * bluetoothInterface = d->interfaces.take(ubi);
        bluetoothInterface = 0;
    }
}

/*
KJob *BluezBluetoothManager::setupInputDevice(const QString &ubi)
{
    QString address = ubi.right(17);

    QList<QVariant> params;
    params << address;

    return new BluezCallJob(QDBusConnection::systemBus(), m_inputManagerDest, "/org/bluez/input", "org.bluez.input.Manager",
                            "CreateDevice", params);
}
*/

/*
QStringList BluezBluetoothManager::bluetoothInputDevices() const
{
    QStringList bluetoothInputDevices;

    QDBusReply< QStringList > deviceList = d->inputManager->call("ListDevices");
    if (deviceList.isValid()) {
        QStringList devices = deviceList.value();
        foreach (const QString& path, devices) {
            bluetoothInputDevices.append(path);
        }
    }
    return bluetoothInputDevices;
}

void BluezBluetoothManager::removeInputDevice(const QString &ubi)
{
    d->inputManager->call("RemoveDevice", ubi);
}

QObject *BluezBluetoothManager::createBluetoothInputDevice(QString const &ubi)
{
    BluezBluetoothInputDevice *bluetoothInputDevice;
    if (d->inputDevices.contains(ubi)) {
        bluetoothInputDevice = d->inputDevices[ubi];
    } else {
        bluetoothInputDevice = new BluezBluetoothInputDevice(ubi, m_inputManagerDest);
        d->inputDevices.insert(ubi, bluetoothInputDevice);
    }
    return bluetoothInputDevice;
}
*/

void BluezBluetoothManager::slotDeviceAdded(const QDBusObjectPath &adapter)
{
    // TODO free the adapter device...
    kDebug() << "interfaceAdded " << adapter.path();
    emit interfaceAdded(adapter.path());
}

void BluezBluetoothManager::slotDeviceRemoved(const QDBusObjectPath &adapter)
{
    kDebug() << "interfaceRemoved " << adapter.path();
    emit interfaceRemoved(adapter.path());
}

void BluezBluetoothManager::slotDefaultDeviceChanged(const QDBusObjectPath &adapter)
{
    kDebug() << "defaultDeviceChanged " << adapter.path();
    emit defaultInterfaceChanged(adapter.path());
}

/*
void BluezBluetoothManager::slotInputDeviceCreated(const QString &path)
{
    emit inputDeviceCreated(path);
}

void BluezBluetoothManager::slotInputDeviceRemoved(const QString &path)
{
    // TODO free the input device...
    emit inputDeviceRemoved(path);
}
*/

/*
Solid::Control::Ifaces::BluetoothSecurity *BluezBluetoothManager::security(const QString &interface)
{
    BluezBluetoothSecurity *out;
    if (interface.isEmpty()) {
        out = new BluezBluetoothSecurity(this);
    } else {
        out = new BluezBluetoothSecurity(interface,this);
    }
    new BluezBluetoothSecurityPasskeyAgentAdaptor(out);
    new BluezBluetoothSecurityAuthorizationAgentAdaptor(out);
    return out;
}
*/
#include "bluez-bluetoothmanager.moc"


