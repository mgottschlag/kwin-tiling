/*
Copyright 2011 Lamarque Souza <lamarque@gmail.com>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) version 3, or any
later version accepted by the membership of KDE e.V. (or its
successor approved by the membership of KDE e.V.), which shall
act as a proxy defined in Section 6 of version 3 of the license.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SOLID_CONTROL_BTNETWORKINTERFACE_H
#define SOLID_CONTROL_BTNETWORKINTERFACE_H

#include "networkgsminterface.h"

namespace Solid
{
namespace Control
{
    class BtNetworkInterfacePrivate;
    /**
     * This interface represents a GSM cellular network interface
     */
    class SOLIDCONTROL_EXPORT BtNetworkInterface : public GsmNetworkInterface
    {
        Q_OBJECT
        Q_FLAGS(Capabilities)
        Q_DECLARE_PRIVATE(BtNetworkInterface)

    public:
        enum Capability { NoCapability = 0x0, Dun /* Dial up */ = 0x1, Pan /* Personal area network */ = 0x2 };
        Q_DECLARE_FLAGS(Capabilities, Capability)

        /**
         * Creates a new BtNetworkInterface object.
         *
         * @param backendObject the network object provided by the backend
         */
        BtNetworkInterface(QObject *backendObject = 0);

        /**
         * Constructs a copy of a network.
         *
         * @param network the network to copy
         */
        BtNetworkInterface(const BtNetworkInterface &network);

        /**
         * Destroys a BtNetworkInterface object.
         */
        virtual ~BtNetworkInterface();

        /**
         * The NetworkInterface type.
         *
         * @return the NetworkInterface::Type.  This always returns NetworkInterface::Bluetooth
         */
        virtual NetworkInterface::Type type() const;

        /**
         * Retrieves the capabilities supported by this device.
         *
         * @return the capabilities of the device
         * @see Solid::Control::BtNetworkInterface::Capabilities
         */
        Solid::Control::BtNetworkInterface::Capabilities btCapabilities() const;

        /**
         * The hardware address assigned to the bluetooth interface
         */
        QString hardwareAddress() const;

        /**
         * Name of the bluetooth interface
         */
        QString name() const;

    protected:
        /**
         * @internal
         */
        BtNetworkInterface(BtNetworkInterfacePrivate &dd, QObject *backendObject);

        /**
         * @internal
         */
        BtNetworkInterface(BtNetworkInterfacePrivate &dd, const BtNetworkInterface &network);

        void makeConnections(QObject * source);
    private Q_SLOTS:
        void _k_destroyed(QObject *object);
    private:
        friend class NetworkInterface;
        friend class NetworkInterfacePrivate;
    };
} //Control
} //Solid

#endif // SOLID_CONTROL_BTNETWORKINTERFACE_H

