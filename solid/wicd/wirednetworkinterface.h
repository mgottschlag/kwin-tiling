/*  This file is part of the KDE project
    Copyright (C) 2008 Dario Freddi <drf54321@gmail.com>

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

#ifndef WICD_WIREDNETWORKINTERFACE_H
#define WICD_WIREDNETWORKINTERFACE_H

#include <solid/control/ifaces/wirednetworkinterface.h>
#include <solid/control/wirednetworkinterface.h>

#include "networkinterface.h"

class WicdWiredNetworkInterface : public WicdNetworkInterface, virtual public Solid::Control::Ifaces::WiredNetworkInterface
{
    Q_OBJECT
    Q_INTERFACES(Solid::Control::Ifaces::WiredNetworkInterface)

    public:
        WicdWiredNetworkInterface(const QString  & objectPath);
        virtual ~WicdWiredNetworkInterface();

        QString hardwareAddress() const;
        int bitRate() const;
        bool carrier() const;

    Q_SIGNALS:
        void bitRateChanged(int bitRate);
        void carrierChanged(bool plugged);
};

#endif // WICD_WIRELESSNETWORKINTERFACE_H

