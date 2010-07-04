/*
Copyright 2008 Will Stephenson <wstephenson@kde.org>
Copyright 2010 Lamarque Souza <lamarque@gmail.com>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of
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

#ifndef MM04_MODEMGSMCONTACTSINTERFACE_H
#define MM04_MODEMGSMCONTACTSINTERFACE_H

#include "modeminterface.h"
#include "solid/control/ifaces/modemgsmcontactsinterface.h"
#include "dbus/generic-types.h"

class MMModemGsmContactsInterfacePrivate;

class KDE_EXPORT MMModemGsmContactsInterface : public MMModemInterface, virtual public Solid::Control::Ifaces::ModemGsmContactsInterface
{
Q_OBJECT
Q_DECLARE_PRIVATE(MMModemGsmContactsInterface)
Q_INTERFACES(Solid::Control::Ifaces::ModemGsmContactsInterface)

public:
    MMModemGsmContactsInterface(const QString & path, MMModemManager * manager, QObject * parent);
    ~MMModemGsmContactsInterface();

    int addContact(const QString & name, const QString & number);
    void deleteContact(const int index);
    ContactType get(const int index);
    Solid::Control::ModemGsmContactsInterface::ContactTypeList list();
    Solid::Control::ModemGsmContactsInterface::ContactTypeList find(const QString & pattern);
    int getCount();
};

#endif

