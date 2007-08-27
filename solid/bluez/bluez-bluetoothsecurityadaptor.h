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

#ifndef __BLUEZ_BLUETOOTHSECURITYADAPTOR_H
#define __BLUEZ_BLUETOOTHSECURITYADAPTOR_H

#include "bluez-bluetoothsecurity.h"

#include <QtDBus>

/**
 * Acts as a proxy to BluezBluetoothSecurity to expose it's functionalities to the D-Bus, which is needed
 * by the BlueZ system.
 */
class BluezBluetoothSecurityAdaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface","org.bluez.PasskeyAgent")
    Q_CLASSINFO("D-Bus Interface","org.bluez.AuthorizationAgent")
    public:
        BluezBluetoothSecurityAdaptor(BluezBluetoothSecurity *security);
        ~BluezBluetoothSecurityAdaptor();
    public Q_SLOTS:
        //org.bluez.PasskeyAgent
        QString Request(const QString & path, const QString & address, bool numeric,const QDBusMessage &msg);
        void Confirm(const QString & path, const QString & address, const QString & value,const QDBusMessage &msg);
        void Display(const QString & path, const QString & address, const QString & value);
        void Keypress(const QString & path, const QString & address);
        void Complete(const QString & path, const QString & address);
        void Cancel(const QString & path, const QString & address);


        //org.bluez.AuthorizationAgent
        void Authorize(const QString & adapter_path, const QString & address,
                       const QString & service_path, const QString & uuid,const QDBusMessage &msg);
        void Cancel(const QString & adapter_path, const QString & address,
                            const QString & service_path, const QString & uuid);
        //common
        void Release();
    private:
        QString serviceName;
        BluezBluetoothSecurity *security;
        QDBusInterface *manager;
        QDBusConnection conn;
};

#endif // __BLUEZ_BLUETOOTHSECURITYADAPTOR_H
