/*  This file is part of the KDE project
    Copyright (C) 2006 Will Stephenson <wstephenson@kde.org>
    Copyright (C) 2007 Daniel Gollub <dgollub@suse.de>
    Copyright (C) 2007 Juan González Aguilera <jaguilera@opsiland.info>

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
#include "bluez-bluetoothsecurityadaptor.h"
#include <kdebug.h>
#include <ctime>

BluezBluetoothSecurityAdaptor::BluezBluetoothSecurityAdaptor(BluezBluetoothSecurity * security)
    :QDBusAbstractAdaptor(security),security(security),conn(QDBusConnection::systemBus())
{
    serviceName = QString("/org/kde/solid/BluezBluetoothSecurityAdaptor%1").arg(time(NULL));
    bool done = conn.registerObject(
            serviceName,security,QDBusConnection::ExportAdaptors);
    if (!done) {
        kDebug() << "Failed to register the object: " << conn.lastError().name() << " : " << conn.lastError().message();
        serviceName = "";
    } else {
        kDebug() << "DBus service registered at "<< serviceName <<endl;
                //TODO Add support for an specific local device
        QDBusInterface iface("org.bluez", "/org/bluez","org.bluez.Security",conn, this);
        iface.call("RegisterDefaultPasskeyAgent",serviceName);
        if (iface.lastError().isValid()) {
            kDebug() << "RegisterDefaultPasskeyAgent failed :" << iface.lastError().name() << " : " << iface.lastError().message();
            serviceName = "";
        } else {
            kDebug() << "RegisterDefaultPasskeyAgent succesfull!";
        }
    }

}

BluezBluetoothSecurityAdaptor::~ BluezBluetoothSecurityAdaptor()
{
    kDebug() << k_funcinfo;
    if (!serviceName.isEmpty())
    {
        QDBusInterface iface("org.bluez", "/org/bluez","org.bluez.Security",conn, this);
        iface.call("UnregisterDefaultPasskeyAgent",serviceName);
        if (iface.lastError().isValid()) {
            kDebug() << "UnregisterDefaultPasskeyAgent failed :" << iface.lastError().name() << " : " << iface.lastError().message();
            serviceName = "";
        } else {
            kDebug() << "UnregisterDefaultPasskeyAgent Successful!:" << iface.lastError().name() << " : " << iface.lastError().message();
        }
    }
}

QString BluezBluetoothSecurityAdaptor::Request(const QString & path, const QString & address, bool numeric,const QDBusMessage &msg)
{
    kDebug() << k_funcinfo;
    Q_UNUSED(path)
    if (security) {
        QString answer = security->request(address,numeric);
        if (!answer.isEmpty()) {
            return answer;
        } else {
            QDBusMessage error = msg.createErrorReply("org.bluez.Error.Rejected","Pairing request rejected");
            QDBusConnection::systemBus().send(error);
        }
    }
    return "";//To satisfy the compiler, but the answer is already sent
}

void BluezBluetoothSecurityAdaptor::Confirm(const QString & path, const QString & address, const QString & value,const QDBusMessage &msg)
{
    kDebug() << k_funcinfo;
    Q_UNUSED(path)
    if (security) {
        if(security->confirm(address,value)) {
            kDebug() << "Confirmed pin for " << address;
        } else {
            QDBusMessage error = msg.createErrorReply("org.bluez.Error.Rejected","Pairing request rejected");
            QDBusConnection::systemBus().send(error);
        }
    }
}

void BluezBluetoothSecurityAdaptor::Display(const QString & path, const QString & address, const QString & value)
{
    kDebug() << k_funcinfo;
    Q_UNUSED(path)
    if (security) {
        security->display(address,value);
    }
}

void BluezBluetoothSecurityAdaptor::Keypress(const QString & path, const QString & address)
{
    kDebug() << k_funcinfo;
    Q_UNUSED(path)
    if (security) {
        security->keypress(address);
    }
}

void BluezBluetoothSecurityAdaptor::Complete(const QString & path, const QString & address)
{
    kDebug() << k_funcinfo;
    Q_UNUSED(path)
    if (security) {
        security->complete(address);
    }
}

void BluezBluetoothSecurityAdaptor::Cancel(const QString & path, const QString & address)
{
    kDebug() << k_funcinfo;
    Q_UNUSED(path)
    if (security) {
        security->cancel(address);
    }
}

void BluezBluetoothSecurityAdaptor::Release()
{
    kDebug() << k_funcinfo;
}

void BluezBluetoothSecurityAdaptor::Authorize(const QString & adapter_path, const QString & address, const QString & service_path, const QString & uuid,const QDBusMessage &msg)
{
    kDebug() << k_funcinfo;
    Q_UNUSED(service_path)
    if (security) {
        if(security->authorize(adapter_path,address,uuid)) {
            kDebug() << "Service with uuid "<< uuid <<" for " << address << " authorized";
        } else {
            QDBusMessage error = msg.createErrorReply("org.bluez.Error.Rejected","Authorization request rejected");
            QDBusConnection::systemBus().send(error);
        }
    }
}

void BluezBluetoothSecurityAdaptor::Cancel(const QString & adapter_path, const QString & address, const QString & service_path, const QString & uuid)
{
    Q_UNUSED(service_path)
    if (security) {
        security->cancel(adapter_path,address,uuid);
    }
}

#include "bluez-bluetoothsecurityadaptor.moc"
