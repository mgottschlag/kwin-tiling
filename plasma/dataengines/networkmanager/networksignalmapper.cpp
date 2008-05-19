/*
 *   Copyright (C) 2007 Christopher Blauvelt <cblauvelt@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "networksignalmapper.h"

NetworkSignalMapper::NetworkSignalMapper(QObject *parent) : QSignalMapper(parent)
{
}

NetworkSignalMapper::~NetworkSignalMapper()
{
}

void NetworkSignalMapper::setMapping(QObject* network, const QString &uni)
{
    signalmap[network] = uni;
}

NetworkInterfaceControlSignalMapper::NetworkInterfaceControlSignalMapper(QObject *parent) : NetworkSignalMapper(parent)
{
}

NetworkInterfaceControlSignalMapper::~NetworkInterfaceControlSignalMapper()
{
}

void NetworkInterfaceControlSignalMapper::ipDetailsChanged()
{
}

void NetworkInterfaceControlSignalMapper::linkUpChanged(bool linkActivated)
{
}

void NetworkInterfaceControlSignalMapper::connectionStateChanged(int state)
{
}

void NetworkInterfaceControlSignalMapper::bitRateChanged(int bitRate)
{
}

void NetworkInterfaceControlSignalMapper::carrierChanged(bool plugged)
{
}

void NetworkInterfaceControlSignalMapper::bitRateChanged(int)
{
}

void NetworkInterfaceControlSignalMapper::activeAccessPointChanged(const QString &)
{
}

void NetworkInterfaceControlSignalMapper::modeChanged(Solid::Control::WirelessNetworkInterface::OperationMode)
{
}

void NetworkInterfaceControlSignalMapper::accessPointAppeared(const QString &)
{
}

void NetworkInterfaceControlSignalMapper::accessPointDisappeared(const QString &)
{
}

NetworkControlSignalMapper::NetworkControlSignalMapper(QObject *parent) : NetworkSignalMapper(parent)
{
}

NetworkControlSignalMapper::~NetworkControlSignalMapper()
{
}

void NetworkControlSignalMapper::ipDetailsChanged()
{
    emit(networkChanged(signalmap[sender()], "IP Details Changed", true));
}

void NetworkControlSignalMapper::activationStateChanged(bool activated)
{
    emit(networkChanged(signalmap[sender()], "Active", activated));
}

#include "networksignalmapper.moc"
