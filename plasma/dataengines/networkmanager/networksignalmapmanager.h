/*
 *   Copyright (C) 2008 Christopher Blauvelt <cblauvelt@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef NETWORK_SIGNALMAP_MANAGER_H
#define NETWORK_SIGNALMAP_MANAGER_H

#include <KDebug>

#include "networksignalmapper.h"

class NetworkSignalMapManager : public QObject
{
    Q_OBJECT

    public:
        enum ControlType { NetworkInterfaceControl, AccessPointControl };
        
        NetworkSignalMapManager(QObject *parent=0);
        ~NetworkSignalMapManager();

        void mapNetwork(Solid::Control::NetworkInterface *iface, const QString &uni);
        void mapNetwork(Solid::Control::Network *network, const QString &uni);
        
        void unmapNetwork(Solid::Control::NetworkInterface *iface);
        void unmapNetwork(Solid::Control::Network *network);
        
    private:
        QMap<ControlType, NetworkSignalMapper*> signalmap;
        QObject *user;
};

#endif
