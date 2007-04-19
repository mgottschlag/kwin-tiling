/*  This file is part of the KDE project
    Copyright (C) 2007 Will Stephenson <wstephenson@kde.org>
    Copyright (C) 2007 Daniel Gollub <dgollub@suse.de>


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

#ifndef BLUEZ_BLUETOOTHINTERFACE_H
#define BLUEZ_BLUETOOTHINTERFACE_H

#include <kdemacros.h>

#include <solid/experimental/ifaces/bluetoothinterface.h>

class BluezBluetoothInterfacePrivate;

class KDE_EXPORT BluezBluetoothInterface : public SolidExperimental::Ifaces::BluetoothInterface
{
    Q_OBJECT
    Q_INTERFACES(SolidExperimental::Ifaces::BluetoothInterface)
public:
    BluezBluetoothInterface(const QString & objectPath);
    virtual ~BluezBluetoothInterface();
    QString ubi() const;
    QString address() const;
    QString version() const;
    QString revision() const;
    QString manufacturer() const;
    QString company() const;
    SolidExperimental::BluetoothInterface::Mode mode() const;
    int discoverableTimeout() const;
    bool isDiscoverable() const;
    QStringList listConnections() const;
    QString majorClass() const;
    QStringList listAvailableMinorClasses() const;
    QString minorClass() const;
    QStringList serviceClasses() const;
    QString name() const;
    QStringList listBondings() const;
    bool isPeriodicDiscoveryActive() const;
    bool isPeriodicDiscoveryNameResolvingActive() const;
    QStringList listRemoteDevices() const;
    QStringList listRecentRemoteDevices(const QDateTime&) const;

    QObject *createBluetoothRemoteDevice(const QString&);

public Q_SLOTS:
    void setMode(const SolidExperimental::BluetoothInterface::Mode);
    void setDiscoverableTimeout(int);
    void setMinorClass(const QString&);
    void setName(const QString&);
    void discoverDevices();
    void discoverDevicesWithoutNameResolving();
    void cancelDiscovery();
    void startPeriodicDiscovery();
    void stopPeriodicDiscovery();
    void setPeriodicDiscoveryNameResolving(bool);

    void slotModeChanged(const SolidExperimental::BluetoothInterface::Mode mode);
    void slotDiscoverableTimeoutChanged(int timeout);
    void slotMinorClassChanged(const QString &minor);
    void slotNameChanged(const QString &name);
    void slotDiscoveryStarted();
    void slotDiscoveryCompleted();
    void slotRemoteDeviceFound(const QString &ubi, uint deviceClass, short rssi);
    void slotRemoteDeviceDisappeared(const QString &ubi);

private:
    BluezBluetoothInterfacePrivate * d;

    QStringList listReply(const QString &method) const;
    QString stringReply(const QString &method) const;
    bool boolReply(const QString &method) const;
};

#endif
