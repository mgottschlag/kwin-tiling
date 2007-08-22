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
#include "bluez-bluetoothsecurity.h"
#include <kdebug.h>

/*********************** BluezBluetoothSecurity ***********************/
BluezBluetoothSecurity::BluezBluetoothSecurity(QObject * parent)
    :Solid::Control::Ifaces::BluetoothSecurity(parent),passkeyAgent(0),authAgent(0)
{
    kDebug() << k_funcinfo << endl; 
}

BluezBluetoothSecurity::BluezBluetoothSecurity(const QString & interface, QObject * parent)
    :Solid::Control::Ifaces::BluetoothSecurity(interface,parent)
{
    kDebug() << k_funcinfo << " interface: " << interface << endl; 
}

BluezBluetoothSecurity::~ BluezBluetoothSecurity()
{
    kDebug() << k_funcinfo << endl; 
}

/*********************** methods from Solid::Control::BluetoothSecurity ***********************/
void BluezBluetoothSecurity::setPasskeyAgent(Solid::Control::BluetoothPasskeyAgent * agent)
{
    if (passkeyAgent) {
        delete passkeyAgent;
    }
    passkeyAgent = agent;
}
void BluezBluetoothSecurity::setAuthorizationAgent(Solid::Control::BluetoothAuthorizationAgent * agent)
{
    if (authAgent) {
        delete authAgent;
    }
    authAgent = agent;
}


/**************************************************************************************/
QString BluezBluetoothSecurity::request(const QString & address, bool numeric)
{
    QString out;
    if (passkeyAgent) {
        out = passkeyAgent->requestPasskey(address,numeric);
    }
    return out;
}

bool BluezBluetoothSecurity::confirm(const QString & address, const QString & value)
{
    bool out = false;
    if (passkeyAgent) {
        out = passkeyAgent->confirmPasskey(address,value);
    }
    return out;
}

void BluezBluetoothSecurity::display(const QString & address, const QString & value)
{
    if (passkeyAgent) {
        passkeyAgent->displayPasskey(address,value);
    }
}

void BluezBluetoothSecurity::complete(const QString & address)
{
    if (passkeyAgent) {
        passkeyAgent->completedAuthentication(address);
    }
}

void BluezBluetoothSecurity::keypress(const QString & address)
{
    if (passkeyAgent) {
        passkeyAgent->keypress(address);
    }
}

void BluezBluetoothSecurity::cancel(const QString & address)
{
    if (passkeyAgent) {
        passkeyAgent->cancelAuthentication(  address);
    }
}

bool BluezBluetoothSecurity::authorize(const QString & localUbi, const QString & remoteAddress, const QString & serviceUuid)
{
    bool out = false;
    if (authAgent) {
        out = authAgent->authorize(localUbi,remoteAddress,serviceUuid);
    }
    return out;
}

void BluezBluetoothSecurity::cancel(const QString & localUbi, const QString & remoteAddress, const QString & serviceUuid)
{
    if (authAgent) {
        authAgent->cancel(localUbi,remoteAddress,serviceUuid);
    }
}

#include "bluez-bluetoothsecurity.moc"
