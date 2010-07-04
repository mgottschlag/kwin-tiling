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

#ifndef SOLID_CONTROL_MODEMMANAGER
#define SOLID_CONTROL_MODEMMANAGER

#include <QtCore/QObject>
#include <QtCore/QVariant>

#include <solid/networking.h>
#include "solid_control_export.h"
#include "modemmanagerinterface.h"
#include "modeminterface.h"

namespace Solid
{
namespace Control
{
    namespace Ifaces
    {
        class ModemManager;
    }
    class Network;
    typedef QList<ModemInterface*> ModemInterfaceList;

    /**
     * This class allow to query the underlying system to discover the available
     * modem interfaces and reachable network. It has also the
     * responsibility to notify when a modem interface or a modem appear or disappear.
     *
     * Note that it's implemented as a singleton and encapsulates the backend logic.
     */
    namespace ModemManager
    {
        /**
         * Get the manager connection state
         */
        SOLIDCONTROL_EXPORT Solid::Networking::Status status();

        /**
         * Retrieves the list of all modem interfaces Unique Device Identifiers (UDIs)
         * in the system. This method is the equivalent of enumerateDevices described
         * in Modem Manager specification.
         * @return the list of modem interfaces available in this system
         */
        SOLIDCONTROL_EXPORT ModemInterfaceList modemInterfaces();

        /**
         * Find a new ModemManagerInterface object given its UDNI.  This pointer is owned by the Solid
         * infrastructure.
         *
         * @param udi the identifier of the modem interface to find
         * @returns a valid ModemManagerInterface object if there's a device having the given UDI, an invalid one otherwise
         */
        SOLIDCONTROL_EXPORT ModemInterface * findModemInterface(const QString &udi, const ModemInterface::GsmInterfaceType ifaceType);

        /**
         * Retrieves the interface types supported by this modem manager.
         *
         * @return the interface types supported by the modem manager
         *
         * @since 4.5
         */
        SOLIDCONTROL_EXPORT Solid::Control::ModemInterface::Type supportedInterfaceTypes();

        class SOLIDCONTROL_EXPORT Notifier : public QObject
        {
            Q_OBJECT
        Q_SIGNALS:
            /**
             * This signal is emitted when the system's connection state changes
             */
            void statusChanged(Solid::Networking::Status status);

            /**
             * This signal is emitted when a new modem interface is available.
             *
             * @param udi the network interface identifier
             */
            void modemInterfaceAdded(const QString &udi);

            /**
             * This signal is emitted when a network interface is not available anymore.
             *
             * @param udi the network interface identifier
             */
            void modemInterfaceRemoved(const QString &udi);
        };

        SOLIDCONTROL_EXPORT Notifier *notifier();
    }

} // Control
} // Solid

#endif
