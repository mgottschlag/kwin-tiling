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

#ifndef BLUEZ_BLUETOOTHREMOTEDEVICE_H
#define BLUEZ_BLUETOOTHREMOTEDEVICE_H

#include <kdemacros.h>

#include <solid/control/ifaces/bluetoothremotedevice.h>
#include <QtDBus>


class KDE_EXPORT BluezBluetoothRemoteDevice : public Solid::Control::Ifaces::BluetoothRemoteDevice
{
    Q_OBJECT
    Q_INTERFACES(Solid::Control::Ifaces::BluetoothRemoteDevice)
public:
    BluezBluetoothRemoteDevice(const QString &objectPath);
    virtual ~BluezBluetoothRemoteDevice();

    QString ubi();
    QMap<QString,QVariant> getProperties();
    void discoverServices(const QString &pattern);
    QStringList listNodes();

private Q_SLOTS:

    void slotPropertyChanged(const QString &, const QDBusVariant &);
    void slotDisconnectRequested();
    void slotNodeCreated(const QDBusObjectPath &); 
    void slotNodeRemoved(const QDBusObjectPath &);

    void slotServiceDiscover(const QMap<uint,QString> &handles);
    void dbusErrorServiceDiscover(const QDBusError &error);

public Q_SLOTS:

    void setProperty(const QString &, const QVariant &);
    void cancelDiscovery();
    void disconnect();

Q_SIGNALS:

    void serviceDiscoverAvailable(const QString &state, const QMap<uint,QString> &handles);
    void propertyChanged(const QString &, const QVariant &);
    void disconnectRequested();
    void nodeCreated(const QString &);
    void nodeRemoved(const QString &);

private:
    QString m_objectPath;
    QDBusInterface *device;
    QString m_address;
    QString m_adapter;

    QStringList listReply(const QString &method) const;
    QString stringReply(const QString &method) const;
    bool boolReply(const QString &method) const;
};

#endif
