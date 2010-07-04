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

#ifndef SOLID_CONTROL_MODEMGSMSMSINTERFACE_H
#define SOLID_CONTROL_MODEMGSMSMSINTERFACE_H

#include "modeminterface.h"

namespace Solid
{
namespace Control
{
    class ModemGsmSmsInterfacePrivate;

    class SOLIDCONTROL_EXPORT ModemGsmSmsInterface: public ModemInterface
    {
        Q_OBJECT
        Q_DECLARE_PRIVATE(ModemGsmSmsInterface)

    public:

        ModemGsmSmsInterface(QObject *backendObject = 0);

        ModemGsmSmsInterface(const ModemGsmSmsInterface &smsIface);

        virtual ~ModemGsmSmsInterface();

        void deleteSms(const int index) const;

        QVariantMap get(const int index) const;

        int getFormat() const;

        void setFormat(const int format) const;

        QString getSmsc() const;

        QList<QVariantMap> list() const;

        void save(const QVariantMap & properties) const;

        void send(const QVariantMap & properties) const;

        void sendFromStorage(const int index) const;

        void setIndication(const int mode, const int mt, const int bm, const int ds, const int brf) const;

    Q_SIGNALS:
        void smsReceived(int index, bool complete);

        void completed(int index, bool completed);

    protected:
        /**
         * @internal
         */
        ModemGsmSmsInterface(ModemGsmSmsInterfacePrivate &dd, QObject *backendObject);

        /**
         * @internal
         */
        ModemGsmSmsInterface(ModemGsmSmsInterfacePrivate &dd, const ModemGsmSmsInterface &smsinterface);

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
