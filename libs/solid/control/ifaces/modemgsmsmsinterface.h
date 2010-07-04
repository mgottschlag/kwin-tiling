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

#ifndef SOLID_IFACES_MODEMGSMSMSINTERFACE_H
#define SOLID_IFACES_MODEMGSMSMSINTERFACE_H

#include "../solid_control_export.h"
#include "../modemgsmhsointerface.h"
#include "modemmanagerinterface.h"

namespace Solid
{
namespace Control
{
namespace Ifaces
{
    class SOLIDCONTROLIFACES_EXPORT ModemGsmSmsInterface: virtual public ModemManagerInterface
    {
    public:
        virtual ~ModemGsmSmsInterface();

        virtual void deleteSms(const int index) = 0;

        virtual QVariantMap get(const int index) = 0;

        virtual int getFormat() = 0;

        virtual void setFormat(const int format) = 0;

        virtual QString getSmsc() = 0;

        virtual QList<QVariantMap> list() = 0;

        virtual void save(const QVariantMap & properties) = 0;

        virtual void send(const QVariantMap & properties) = 0;

        virtual void sendFromStorage(const int index) = 0;

        virtual void setIndication(const int mode, const int mt, const int bm, const int ds, const int brf) = 0;

    protected:
    //Q_SIGNALS:
        void smsReceived(int index, bool complete);

        void completed(int index, bool completed);
    };
} // Ifaces
} // Control
} // Solid

Q_DECLARE_INTERFACE(Solid::Control::Ifaces::ModemGsmSmsInterface, "org.kde.Solid.Control.Ifaces.ModemGsmSmsInterface/0.1")
#endif
