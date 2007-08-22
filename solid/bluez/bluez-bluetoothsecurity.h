/*  This file is part of the KDE project
    Copyright (C) 2007 Juan González <jaguilera@opsiland.info>


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
#ifndef BLUEZ_BLUETOOTH_SECURITY_H
#define BLUEZ_BLUETOOTH_SECURITY_H

#include <solid/control/ifaces/bluetoothsecurity.h>
#include <solid/control/bluetoothsecurity.h>

#include <QStringList>

/**
 * Implementation of the bluez security stuff. This is used to handle remote device pairing/authorization
 * using the BlueZ stack.
 */
class KDE_EXPORT BluezBluetoothSecurity : public Solid::Control::Ifaces::BluetoothSecurity
{
    Q_OBJECT
    Q_INTERFACES(Solid::Control::Ifaces::BluetoothSecurity)
    public:
        explicit BluezBluetoothSecurity(QObject *parent = 0);
        BluezBluetoothSecurity(const QString &interface,QObject *parent = 0);
        ~BluezBluetoothSecurity();

        //No need to make this private as it's not exposed in the super.
        //For the passkey agents
        QString request(const QString & address, bool numeric);
        bool confirm(const QString & address, const QString & value);
        void display(const QString & address, const QString & value);
        void keypress(const QString & address);
        void complete(const QString & address);
        void cancel(const QString & address);

        //For the authorization agent
        bool authorize(const QString &localUbi,const QString &remoteAddress,const QString& serviceUuid);
        void cancel(const QString &localUbi,const QString &remoteAddress,const QString& serviceUuid);

    public Q_SLOTS:
        void setPasskeyAgent(Solid::Control::BluetoothPasskeyAgent *agent);
        void setAuthorizationAgent(Solid::Control::BluetoothAuthorizationAgent *agent);

    private:
        Solid::Control::BluetoothPasskeyAgent * passkeyAgent;
        Solid::Control::BluetoothAuthorizationAgent *authAgent;
};

#endif
