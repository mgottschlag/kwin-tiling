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

#ifndef SOLID_CONTROL_MODEMMANAGERINTERFACE_H
#define SOLID_CONTROL_MODEMMANAGERINTERFACE_H

#include <QtCore/QObject>
#include <QtCore/QVariant>

#include <solid/networking.h>
#include "solid_control_export.h"
#include "networkipv4config.h"

namespace Solid
{
namespace Control
{
    class ModemManagerInterfacePrivate;

    class SOLIDCONTROL_EXPORT ModemManagerInterface: public QObject
    {
    Q_OBJECT
    Q_DECLARE_PRIVATE(ModemManagerInterface)

    public:
        /**
         * Creates a new ModemManagerInterface object.
         *
         * @param backendObject the modem object provided by the backend
         */
        explicit ModemManagerInterface(QObject *backendObject);

        /**
         * Constructs a copy of a modem.
         *
         * @param modem the modem to copy
         */
        ModemManagerInterface(const ModemManagerInterface &modem);

        /**
         * Destroys a ModemManagerInterface object.
         */
        virtual ~ModemManagerInterface();

    Q_SIGNALS:
        /**
         * This signal is emitted when a device was added to the system.
         *
         * @param device object path of the newly added device.
         */
        void deviceAdded(QString &device);
        /**
         * This signal is emitted when a device was removed from the system.
         *
         * @param device object path of the device that was just removed.
         */
        void deviceRemoved(QString &device);

    protected:
        /**
         * @internal
         */
        ModemManagerInterface(ModemManagerInterfacePrivate &dd, QObject *backendObject);

        /**
         * @internal
         */
        ModemManagerInterface(ModemManagerInterfacePrivate &dd, const ModemManagerInterface &modem);

        ModemManagerInterfacePrivate *d_ptr;
    };

} // Control
} // Solid

#endif
