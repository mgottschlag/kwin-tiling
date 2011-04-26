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

#ifndef SOLID_IFACES_BTNETWORKINTERFACE_H
#define SOLID_IFACES_BTNETWORKINTERFACE_H

#include "../solid_control_export.h"

#include "../networkbtinterface.h"
#include "networkgsminterface.h"


namespace Solid
{
namespace Control
{
namespace Ifaces
{
    /**
     * Represents a bluetooth network interface
     */
    class SOLIDCONTROLIFACES_EXPORT BtNetworkInterface : virtual public GsmNetworkInterface
    {
    public:
        /**
         * Destroys a BtNetworkInterface object
         */
        virtual ~BtNetworkInterface();

        /**
         * Retrieves the capabilities supported by this device.
         *
         * @return the capabilities of the device
         * @see Solid::Control::BtNetworkInterface::Capabilities
         */
        virtual Solid::Control::BtNetworkInterface::Capabilities btCapabilities() const = 0;

        /**
         * The hardware address assigned to the bluetooth interface
         */
        virtual QString hardwareAddress() const = 0;

        /**
         * Name of the bluetooth interface
         */
        virtual QString name() const = 0;

    protected:
    //Q_SIGNALS:
        /**
         * This signal is emitted when the network name of this network changes
         *
         * @param networkName the new network name
         */
        void networkNameChanged(const QString & networkName);
    };
} //Ifaces
} //Control
} //Solid

Q_DECLARE_INTERFACE(Solid::Control::Ifaces::BtNetworkInterface, "org.kde.Solid.Control.Ifaces.BtNetworkInterface/0.1")

#endif //SOLID_IFACES_BTNETWORKINTERFACE_H

