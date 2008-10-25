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

#ifndef BLUEZ_BLUETOOTHINPUTDEVICE_H
#define BLUEZ_BLUETOOTHINPUTDEVICE_H

#include <kdemacros.h>
#include <QMap>
#include <QtDBus>

#include <solid/control/ifaces/bluetoothinputdevice.h>

class QDBusInterface;

class KDE_EXPORT BluezBluetoothInputDevice : public Solid::Control::Ifaces::BluetoothInputDevice
{
    Q_OBJECT
    Q_INTERFACES(Solid::Control::Ifaces::BluetoothInputDevice)
public:
    BluezBluetoothInputDevice(const QString &objectPath);
    virtual ~BluezBluetoothInputDevice();
    QString ubi() const;
    QMap<QString,QVariant> getProperties() const;

private Q_SLOTS:
    void slotPropertyChanged(const QString &, const QDBusVariant &);


public Q_SLOTS:
    void disconnect();
    void connect();

Q_SIGNALS:

    void propertyChanged(const QString &, const QVariant &);

private:
    QString m_objectPath;
    QDBusInterface *device;

    QString stringReply(const QString &method) const;
    bool boolReply(const QString &method) const;
};

#endif
