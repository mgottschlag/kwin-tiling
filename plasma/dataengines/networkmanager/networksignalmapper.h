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

#ifndef NETWORK_SIGNAL_MAPPER_H
#define NETWORK_SIGNAL_MAPPER_H

#include <QObject>
#include <QSignalMapper>
#include <QMap>
#include <QVariant>

#include <KDebug>

#include <solid/networking.h>
#include <solid/control/networkinterface.h>
#include <solid/control/wirednetworkinterface.h>
#include <solid/control/wirelessnetworkinterface.h>
#include <solid/control/wirelessaccesspoint.h>

class NetworkSignalMapper : public QSignalMapper
{
    Q_OBJECT

    public:
        NetworkSignalMapper(QObject *parent=0);
        ~NetworkSignalMapper();
        
        void setMapping(QObject* network, const QString &uni);

    Q_SIGNALS:
        void networkChanged(const QString& uni, const QString &property, QVariant value);
        
    protected:
        QMap<QObject*, QString> signalmap;
};

class NetworkInterfaceControlSignalMapper : public NetworkSignalMapper
{
    Q_OBJECT

    public:
        NetworkInterfaceControlSignalMapper(QObject *parent=0);
        ~NetworkInterfaceControlSignalMapper();

    public Q_SLOTS:
        void ipDetailsChanged();
        void linkUpChanged(bool linkActivated);
        void connectionStateChanged(int state);
        void bitRateChanged(int bitRate);
        void carrierChanged(bool plugged);
        void bitRateChanged(int);
        void activeAccessPointChanged(const QString &);
        void modeChanged(Solid::Control::WirelessNetworkInterface::OperationMode);
        void accessPointAppeared(const QString &);
        void accessPointDisappeared(const QString &);
};

class AccessPointControlSignalMapper : public NetworkSignalMapper
{
    Q_OBJECT

    public:
        NetworkControlSignalMapper(QObject *parent=0);
        ~NetworkControlSignalMapper();

    public Q_SLOTS:
        void signalStrengthChanged(int strength);
        void bitRateChanged(int bitrate);
        void wpaFlagsChanged(Solid::Control::AccessPoint::WpaFlags flags) const;
        void rsnFlagsChanged(Solid::Control::AccessPoint::WpaFlags flags) const;
        void ssidChanged(const QString & ssid) const;
        void frequencyChanged(double frequency) const;
};

#endif
