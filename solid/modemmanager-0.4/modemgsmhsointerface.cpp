/*
Copyright 2008 Will Stephenson <wstephenson@kde.org>
Copyright 2010 Lamarque Souza <lamarque@gmail.com>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of
the License or (at your option) version 3 or any later version
accepted by the membership of KDE e.V. (or its successor approved
by the membership of KDE e.V.), which shall act as a proxy
defined in Section 14 of version 3 of the license.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "manager.h"
#include "modemgsmhsointerface.h"
#include "modemgsmhsointerface_p.h"

#include <KDebug>

MMModemGsmHsoInterfacePrivate::MMModemGsmHsoInterfacePrivate(const QString &path, QObject *owner)
    : MMModemInterfacePrivate(path, owner), modemGsmHsoIface(MMModemManager::DBUS_SERVICE, path, QDBusConnection::systemBus())
{
}

MMModemGsmHsoInterface::MMModemGsmHsoInterface(const QString & path, MMModemManager * manager, QObject * parent)
    : MMModemInterface(*new MMModemGsmHsoInterfacePrivate(path, this), manager, parent)
{
    Q_D(const MMModemGsmHsoInterface);
    connect( &d->modemGsmHsoIface, SIGNAL(smsReceived(uint,bool)),
                this, SIGNAL(smsReceived(int,bool)));
    connect( &d->modemGsmHsoIface, SIGNAL(completed(uint,bool)),
                this, SIGNAL(completed(int,bool)));
}

MMModemGsmHsoInterface::~MMModemGsmHsoInterface()
{
}

void MMModemGsmHsoInterface::authenticate(const QString & username, const QString & password)
{
    Q_D(MMModemGsmHsoInterface);
    d->modemGsmHsoIface.Authenticate(username, password);
}

#include "modemgsmhsointerface.moc"

