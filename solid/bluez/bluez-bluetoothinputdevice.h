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

#ifndef BLUEZ_BLUETOOTHINPUTDEVICE_H
#define BLUEZ_BLUETOOTHINPUTDEVICE_H

#include <kdemacros.h>

#include <solid/ifaces/bluetoothinputdevice.h>

class KDE_EXPORT BluezBluetoothInputDevice : public Solid::Ifaces::BluetoothInputDevice
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::BluetoothInputDevice)
public:
    BluezBluetoothInputDevice(const QString &objectPath, const QString &dest);
    virtual ~BluezBluetoothInputDevice();
    QString ubi() const;
    QString address() const;
    bool isConnected() const;
    QString name() const;
    QString productID() const;
    QString vendorID() const;

public Q_SLOTS:
    void slotConnect();
    void slotDisconnect();
Q_SIGNALS:
    void connected();
    void disconnected();

private:
    QString m_objectPath;
    QDBusInterface *device;

};

#endif
