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

#ifndef BLUEZ_BLUETOOTHREMOTEDEVICE_H
#define BLUEZ_BLUETOOTHREMOTEDEVICE_H

#include <kdemacros.h>

#include <solid/ifaces/bluetoothremotedevice.h>

class KDE_EXPORT BluezBluetoothRemoteDevice : public Solid::Ifaces::BluetoothRemoteDevice
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::BluetoothRemoteDevice)
public:
    BluezBluetoothRemoteDevice(const QString &objectPath);
    virtual ~BluezBluetoothRemoteDevice();
    QString ubi() const;
    QString address() const;
    bool isConnected() const;
    QString version() const;
    QString revision() const;
    QString manufacturer() const;
    QString company() const;
    QString majorClass() const;
    QString minorClass() const;
    QStringList serviceClasses() const;
    QString name() const;
    QString alias() const;
    QString lastSeen() const;
    QString lastUsed() const;
    bool hasBonding() const;
    int pinCodeLength() const;
    int encryptionKeySize() const;

public Q_SLOTS:
    void setAlias(const QString &alias);
    void clearAlias();
    void disconnect();
    void createBonding();
    void cancelBondingProcess();
    void removeBonding();

Q_SIGNALS:
    void classChanged(uint deviceClass);
    void nameChanged(const QString &name);
    void nameResolvingFailed();
    void aliasChanged(const QString &alias);
    void aliasCleared();
    void connected();
    void requestDisconnection();
    void disconnected();
    void bondingCreated();
    void bondingRemoved();

private:
    QString m_objectPath;
    QDBusInterface *device;
    QString m_address;
    QString m_adapter;

};

#endif
