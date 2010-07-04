/*  This file is part of the KDE project
    Copyright (C) 2010 Lamarque Souza <lamarque@gmail.com>

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

#ifndef SOLID_CONTROL_MODEMGSMCONTACTSINTERFACE_H
#define SOLID_CONTROL_MODEMGSMCONTACTSINTERFACE_H

#include "modeminterface.h"

namespace Solid
{
namespace Control
{
    class ModemGsmContactsInterfacePrivate;

    class SOLIDCONTROL_EXPORT ModemGsmContactsInterface: public ModemInterface
    {
        Q_OBJECT
        Q_DECLARE_PRIVATE(ModemGsmContactsInterface)

    public:

        class ContactType
        {
        public:
            int index;
            QString name,
                    number;
        };

        typedef QList <Solid::Control::ModemGsmContactsInterface::ContactType> ContactTypeList;

        ModemGsmContactsInterface(QObject *backendObject = 0);

        ModemGsmContactsInterface(const ModemGsmContactsInterface &contactsIface);

        virtual ~ModemGsmContactsInterface();

        int addContact(const QString & name, const QString & number) const;

        void deleteContact(const int index) const;

        ContactType get(const int index) const;

        QList<ContactType> list() const;

        QList<ContactType> find(const QString & pattern) const;

        int getCount() const;

    Q_SIGNALS:

    protected:
        /**
         * @internal
         */
        ModemGsmContactsInterface(ModemGsmContactsInterfacePrivate &dd, QObject *backendObject);

        /**
         * @internal
         */
        ModemGsmContactsInterface(ModemGsmContactsInterfacePrivate &dd, const ModemGsmContactsInterface &contactsIface);

        void makeConnections(QObject * source);
    private Q_SLOTS:
        void _k_destroyed(QObject *object);
    private:
        friend class ModemManagerInterface;
        friend class ModemManagerInterfacePrivate;
    };
} // Control
} // Solid

#endif
