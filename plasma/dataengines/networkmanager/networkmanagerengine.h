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

#ifndef NETWORKMANAGERENGINE_H
#define NETWORKMANAGERENGINE_H

#include <plasma/dataengine.h>
#include <solid/control/networkinterface.h>
#include <solid/control/wirednetworkinterface.h>
#include <solid/control/wirelessnetworkinterface.h>
#include <solid/control/wirelessaccesspoint.h>
#include <solid/control/networkipv4config.h>
#include "networksignalmapmanager.h"
#include "networksignalmapper.h"

class NetworkManagerEngine : public Plasma::DataEngine
{
    Q_OBJECT

public:
    NetworkManagerEngine(QObject* parent, const QVariantList& args);
    ~NetworkManagerEngine();

public Q_SLOTS:
    void networkChanged(const QString& uni, const QString &property, const QVariant &value);

    //These slots are implemented here because "Network Mangement" source is a singleton
    void onStatusChanged(Solid::Networking::Status status);
    void onNetworkInterfaceAdded(const QString &uni);
    void onNetworkInterfaceRemoved(const QString &uni);

protected:
    bool sourceRequestEvent(const QString &name);

private Q_SLOTS:
    void determineIcon();
    
private:
    bool updateNetworkManagement();
    bool updateWirelessData(const Solid::Control::NetworkInterface &iface);
    bool updateInterfaceData(Solid::Control::NetworkInterface &iface);
    bool updateNetworkData(Solid::Control::Network *network);
    QString determineStageOfConnection(const int connectionState);
    QString determineSignalIcon(int strength);
    inline QString connectionStateToString(int state);
    inline QString statusToString(int status);
    inline QStringList NetworkManageEngine::wirelessCapabilitiesToStringList(int capabilities);
    inline void updateActiveIface(Solid::Control::NetworkInterface iface);
    inline void updateConnectingIface(Solid::Control::NetworkInterface iface);

    NetworkSignalMapManager *signalmanager;
    QHash<QString,Solid::Control::NetworkInterface*> ifaceMap;
    QHash<QString,Solid::Control::AccessPoint*> accessPointMap;

    Solid::Networking::Status m_status;
    Solid::Control::NetworkInterface *m_activeIface;
    QString m_activeNetwork;
    Solid::Control::NetworkInterface *m_connectingIface;
    int m_lastSignalStrength;

    static const int signalStrengthResolution = 25;
    static const int hysteresis = 5;
};

inline QString NetworkManagerEngine::connectionStateToString(int state)
{
    switch (state) {
        case Solid::Control::NetworkInterface::UnknownState:
            return "UnknownState";
        case Solid::Control::NetworkInterface::Down:
            return "Down";
        case Solid::Control::NetworkInterface::Disconnected:
            return "Disconnected";
        case Solid::Control::NetworkInterface::Preparing:
            return "Preparing";
        case Solid::Control::NetworkInterface::Configuring:
            return "Configuring";
        case Solid::Control::NetworkInterface::NeedAuth:
            return "NeedAuth";
        case Solid::Control::NetworkInterface::IPConfig:
            return "IPConfig";
        case Solid::Control::NetworkInterface::Activated:
            return "Activated";
        case Solid::Control::NetworkInterface::Failed:
            return "Failed";
        case Solid::Control::NetworkInterface::Cancelled:
            return "Cancelled";
        default:
            return "UnknownState";
    }
}

inline QString NetworkManagerEngine::statusToString(int status)
{
    switch (status) {
        case Solid::Networking::Unknown:
            return "Unknown";
        case Solid::Networking::Unconnected:
            return "Unconnected";
        case Solid::Networking::Disconnecting:
            return "Disconnecting";
        case Solid::Networking::Connecting:
            return "Connecting";
        case Solid::Networking::Connected:
            return "Connected";
    }
    return QString();
}

inline QStringList NetworkManageEngine::wirelessCapabilitiesToStringList(int capabilities)
{
    QStringList capabilityList;
    if(capabilities & NoCapability) {
        capabilityList << "No Capability";
    }
    if(capabilities & Wep40){
        capabilityList << "WEP 40";
    }
    if(capabilities & Wep104) {
        capabilityList << "WEP 104";
    }
    if(capabilities & Tkip) {
        capabilityList << "TKIP";
    }
    if(capabilities & Ccmp) {
        capabilityList << "CCMP"
    }
    if(capabilities & Wpa) {
        capabilityList << "WPA";
    }
    if(capabilities & Rsn) {
        capabilityList << "RSN";
    }
}

inline void NetworkManagerEngine::updateActiveIface(Solid::Control::NetworkInterface iface)
{
    if (iface.uni() == m_activeIface.uni()) {
        return;
    }
    disconnect(&m_activeIface, SIGNAL(signalStrengthChanged(int)), this, SLOT(determineIcon()));
    disconnect(&m_activeIface, SIGNAL(linkUpChanged(bool)), this, SLOT(determineIcon()));

    m_activeIface = iface;
    if (m_activeIface.isValid()) {
        connect(&m_activeIface, SIGNAL(signalStrengthChanged(int)), this, SLOT(determineIcon()));
        connect(&m_activeIface, SIGNAL(linkUpChanged(bool)), this, SLOT(determineIcon()));
    }
}

inline void NetworkManagerEngine::updateConnectingIface(Solid::Control::NetworkInterface iface)
{
    if (iface.uni() == m_connectingIface.uni()) {
        return;
    }
    disconnect(&m_connectingIface, SIGNAL(connectionStateChanged(int)), this, SLOT(determineIcon()));
    
    m_connectingIface = iface;
    if(m_connectingIface.isValid()) {
        connect(&m_connectingIface, SIGNAL(connectionStateChanged(int)), this, SLOT(determineIcon()));
    }
}

K_EXPORT_PLASMA_DATAENGINE(networkmanager, NetworkManagerEngine)

#endif
