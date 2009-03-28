/*  This file is part of the KDE project
    Copyright (C) 2008 Dario Freddi <drf54321@gmail.com>

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

#include "wirednetworkinterface.h"

#include "wicddbusinterface.h"

#include <QtDBus/QDBusInterface>
#include <QProcess>

WicdWiredNetworkInterface::WicdWiredNetworkInterface(const QString &name)
        : WicdNetworkInterface(name)
{

}

WicdWiredNetworkInterface::~WicdWiredNetworkInterface()
{

}

QString WicdWiredNetworkInterface::hardwareAddress() const
{
    // Let's parse ifconfig here

    QProcess ifconfig;

    ifconfig.start(QString("ifconfig %1").arg(uni()));
    ifconfig.waitForFinished();

    QString result = ifconfig.readAllStandardOutput();

    QStringList lines = result.split('\n');

    return lines.at(0).split("HWaddr ").at(1);
}

int WicdWiredNetworkInterface::bitRate() const
{

}

bool WicdWiredNetworkInterface::carrier() const
{

}

bool WicdWiredNetworkInterface::activateConnection(const QString & connectionUni, const QVariantMap & connectionParameters)
{
    Q_UNUSED(connectionUni)
    Q_UNUSED(connectionParameters)
    WicdDbusInterface::instance()->daemon().call("SetWiredInterface", uni());
    WicdDbusInterface::instance()->wired().call("ConnectWired");
}

bool WicdWiredNetworkInterface::deactivateConnection()
{
    WicdDbusInterface::instance()->wired().call("DisconnectWired");
    return true;
}

#include "wirednetworkinterface.moc"
