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

#ifndef BLUEZ_BLUETOOTHINTERFACE_H
#define BLUEZ_BLUETOOTHINTERFACE_H

#include <kdemacros.h>
#include <QtDBus>
#include <QDBusVariant>
#include <QDBusObjectPath>
#include <QString>

#include <solid/control/ifaces/bluetoothinterface.h>

class BluezBluetoothInterfacePrivate;


class KDE_EXPORT BluezBluetoothInterface : public Solid::Control::Ifaces::BluetoothInterface
{
    Q_OBJECT
    Q_INTERFACES(Solid::Control::Ifaces::BluetoothInterface)
    
public:
    BluezBluetoothInterface(const QString  & objectPath);
    virtual ~BluezBluetoothInterface();
    QString ubi() const;
    
    QObject *createBluetoothRemoteDevice(const QString &);
    QObject *createBluetoothInputDevice(const QString &);
    void createDevice(const QString &) const;
    void createPairedDevice(const QString &,const QString &,const QString &) const;
    QString findDevice(const QString &) const;
    QMap< QString, QVariant > getProperties() const;
    QStringList listDevices() const;


public Q_SLOTS:

    void cancelDeviceCreation(const QString &);
    void registerAgent(const QString &,const QString &);
    void releaseSession();
    void removeDevice(const QString &);
    void requestSession();
    void setProperty(const QString &,const QVariant&);
    void startDiscovery();
    void stopDiscovery();
    void unregisterAgent(const QString &);

    void slotDeviceCreated(const QDBusObjectPath &);
    void slotDeviceDisappeared(const QString &);
    void slotDeviceFound(const QString &, const QMap< QString, QVariant > &);
    void slotDeviceRemoved(const QDBusObjectPath &);
    void slotPropertyChanged(const QString &,const QVariant &);

private:
    BluezBluetoothInterfacePrivate * d;

    QStringList listReply(const QString &method) const;
    QString stringReply(const QString &method, const QString &param = "") const;
    bool boolReply(const QString &method, const QString &param = "") const;
    QDBusObjectPath objectReply(const QString &method, const QString &param = "" ) const;


};

#endif
