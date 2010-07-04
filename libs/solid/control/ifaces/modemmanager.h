/*  This file is part of the KDE project
    Copyright (C) 2006 Will Stephenson <wstephenson@kde.org>
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

#ifndef SOLID_IFACES_MODEMMANAGER
#define SOLID_IFACES_MODEMMANAGER

#include <QtCore/QObject>
#include <QtCore/QStringList>
#include <solid/networking.h>
#include "../solid_control_export.h"
#include "../modemmanager.h"

namespace Solid
{
namespace Control
{
namespace Ifaces
{
    /**
     * This class specifies the interface a backend will have to implement in
     * order to be used in the system.
     *
     */
    class SOLIDCONTROLIFACES_EXPORT ModemManager : public QObject
    {
        Q_OBJECT
    public:
        /**
         * Constructs a ModemManager.
         *
         * @param parent the parent object
         */
        ModemManager(QObject * parent = 0);
        /**
         * Destructs a ModemManager object.
         */
        virtual ~ModemManager();

        /**
         * Get the manager connection state
         */
        virtual Solid::Networking::Status status() const = 0;

        /**
         * Retrieves the list of all modem interfaces Unique Device Identifiers (UDIs)
         * in the system. This method is the equivalent of enumerateDevices described
         * in Modem Manager specification.
         *
         * @return the list of modem interfaces available in this system
         */
        virtual QStringList modemInterfaces() const = 0;

        /**
         * Instantiates a new ModemInterface object from this backend given its UDI.
         *
         * @param udi the identifier of the modem instantiated
         * @returns a new ModemInterface object if there's a device having the given UDI, 0 otherwise
         */
        virtual QObject *createModemInterface(const QString & udi, const Solid::Control::ModemInterface::GsmInterfaceType ifaceType) = 0;

    Q_SIGNALS:
        /**
         * This signal is emitted when the system's connection state changes
         */
        void statusChanged(Solid::Networking::Status status);

        /**
         * This signal is emitted when a device was added to the system.
         *
         * @param device object path of the newly added device.
         */
        void deviceAdded(const QString & device);

        /**
         * This signal is emitted when a device was removed from the system.
         *
         * @param device object path of the device that was just removed.
         */
        void deviceRemoved(const QString & device);
    };

} // Ifaces

} // Control

} // Solid

Q_DECLARE_INTERFACE(Solid::Control::Ifaces::ModemManager, "org.kde.Solid.Control.Ifaces.ModemManager/0.1")

#endif
