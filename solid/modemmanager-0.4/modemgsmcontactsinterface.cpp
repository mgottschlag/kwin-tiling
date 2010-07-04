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
#include "modemgsmcontactsinterface.h"
#include "modemgsmcontactsinterface_p.h"

#include <KDebug>

MMModemGsmContactsInterfacePrivate::MMModemGsmContactsInterfacePrivate(const QString &path, QObject *owner)
    : MMModemInterfacePrivate(path, owner), modemGsmContactsIface(MMModemManager::DBUS_SERVICE, path, QDBusConnection::systemBus())
{
}

MMModemGsmContactsInterface::MMModemGsmContactsInterface(const QString & path, MMModemManager * manager, QObject * parent)
    : MMModemInterface(*new MMModemGsmContactsInterfacePrivate(path, this), manager, parent)
{
}

MMModemGsmContactsInterface::~MMModemGsmContactsInterface()
{
}

int MMModemGsmContactsInterface::addContact(const QString & name, const QString & number)
{
    Q_D(MMModemGsmContactsInterface);
    QDBusReply<uint> index = d->modemGsmContactsIface.Add(name, number);

    if (index.isValid()) {
        return index.value();
    }
    return -1;
}

void MMModemGsmContactsInterface::deleteContact(const int index)
{
    Q_D(MMModemGsmContactsInterface);
    d->modemGsmContactsIface.Delete(index);
}

ContactType MMModemGsmContactsInterface::get(const int index)
{
    Q_D(MMModemGsmContactsInterface);
    QDBusReply<ContactType> contact = d->modemGsmContactsIface.Get(index);

    if (contact.isValid()) {
        return contact.value();
    }

    return ContactType();
}

Solid::Control::ModemGsmContactsInterface::ContactTypeList MMModemGsmContactsInterface::list()
{
    Q_D(MMModemGsmContactsInterface);
    QDBusReply<ContactTypeList> contacts = d->modemGsmContactsIface.List();

    if (contacts.isValid()) {
        return contacts.value();
    }

    return ContactTypeList();
}

Solid::Control::ModemGsmContactsInterface::ContactTypeList MMModemGsmContactsInterface::find(const QString & pattern)
{
    Q_D(MMModemGsmContactsInterface);
    QDBusReply<ContactTypeList > contacts = d->modemGsmContactsIface.Find(pattern);

    if (contacts.isValid()) {
        return contacts.value();
    }

    return ContactTypeList();
}

int MMModemGsmContactsInterface::getCount()
{
    Q_D(MMModemGsmContactsInterface);
    QDBusReply<uint> count = d->modemGsmContactsIface.GetCount();

    if (count.isValid()) {
        return count.value();
    }

    return 0;
}

#include "modemgsmcontactsinterface.moc"

