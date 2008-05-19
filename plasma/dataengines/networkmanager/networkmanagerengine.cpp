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

#include "networkmanagerengine.h"

#include <QMap>
#include <QDir>
#include <QStringList>
#include <QNetworkAddressEntry>

#include <KDebug>
#include <KLocale>
#include <plasma/datacontainer.h>
#include <solid/networking.h>
#include <solid/control/network.h>
#include <solid/control/networkmanager.h>
#include <solid/control/wirelessnetwork.h>

NetworkManagerEngine::NetworkManagerEngine(QObject* parent, const QVariantList& args)
    : Plasma::DataEngine(parent),m_activeNetwork(parent)
{
    Q_UNUSED(args)

    signalmanager = new NetworkSignalMapManager(this);

    //connect network managment signals
    connect(Solid::Control::NetworkManager::notifier(), SIGNAL(statusChanged(Solid::Networking::Status)), this, SLOT(onStatusChanged(Solid::Networking::Status)));
    connect(Solid::Control::NetworkManager::notifier(), SIGNAL(networkInterfaceAdded(const QString&)), this, SLOT(onNetworkInterfaceAdded(const QString&)));
    connect(Solid::Control::NetworkManager::notifier(), SIGNAL(networkInterfaceRemoved(const QString&)), this, SLOT(onNetworkInterfaceRemoved(const QString&)));
}

NetworkManagerEngine::~NetworkManagerEngine()
{
    delete signalmanager;

    disconnect(Solid::Control::NetworkManager::notifier(), SIGNAL(statusChanged(Solid::Networking::Status)), this, SLOT(onStatusChanged(Solid::Networking::Status)));
    disconnect(Solid::Control::NetworkManager::notifier(), SIGNAL(networkInterfaceAdded(const QString&)), this, SLOT(onNetworkInterfaceAdded(const QString&)));
    disconnect(Solid::Control::NetworkManager::notifier(), SIGNAL(networkInterfaceRemoved(const QString&)), this, SLOT(onNetworkInterfaceRemoved(const QString&)));
}

bool NetworkManagerEngine::sourceRequestEvent(const QString &name)
{
    if (name == "Network Management") {
        return updateNetworkManagement();
    }

    if (ifaceMap.contains(name) ) {
        return true;
    } else if(networkMap.contains(name)) {
        return updateNetworkData(networkMap[name]);
    } else {
        Solid::Control::NetworkInterfaceList iflist = Solid::Control::NetworkManager::networkInterfaces();
        foreach (Solid::Control::NetworkInterface *iface, iflist) {
            if (name == iface->uni()) {
                ifaceMap[name] = iface;
                return updateInterfaceData(ifaceMap[name]);
            }
        }
    }

    return false;
}

bool NetworkManagerEngine::updateNetworkManagement()
{
    QString name("Network Management");
    m_status = Solid::Control::NetworkManager::status();
    setData(name, I18N_NOOP("Status"), statusToString(m_status));
    setData(name, I18N_NOOP("Networking Enabled"), Solid::Control::NetworkManager::isNetworkingEnabled());
    setData(name, I18N_NOOP("Wireless Enabled"), Solid::Control::NetworkManager::isWirelessEnabled());

    Solid::Control::NetworkInterfaceList iflist = Solid::Control::NetworkManager::networkInterfaces();
    QStringList ifaceList;
    foreach (const Solid::Control::NetworkInterface &iface, iflist) {
        ifaceList << iface.uni();
        if (iface.isActive()) {
            updateActiveIface(iface);
        }
    }
    setData(name, I18N_NOOP("Network Interfaces"), ifaceList);
    setData(name, I18N_NOOP("Active NetworkInterface"), m_activeIface.uni());
    determineIcon();

    return true;
}

bool NetworkManagerEngine::updateInterfaceData(Solid::Control::NetworkInterface *iface)
{
    QString name = iface->uni();
    //build a list of available networks

    switch (iface.type()) {
        case Solid::Control::NetworkInterface::Ieee8023:
            setData(name, I18N_NOOP("Type"), "Ieee8023");
            break;
        case Solid::Control::NetworkInterface::Ieee80211:
            setData(name, I18N_NOOP("Type"), "Ieee80211");
            updateWirelessData(iface);
            break;
        default:
            setData(name, I18N_NOOP("Type"), "UnknownType");
            break;
    }
    
    setData(name, I18N_NOOP("Active Connection"), iface->activeConnection());
    setData(name, I18N_NOOP("Interface Name"), iface->interfaceName());
    setData(name, I18N_NOOP("Driver"), iface->driver());
    setData(name, I18N_NOOP("Connection State"), connectionStateToString(iface->connectionState()));

    Solid::Control::NetworkInterface::Capabilities capabilities = iface->capabilities();
    QStringList capabilityList;
    if (capabilities & Solid::Control::NetworkInterface::IsManageable) {
        capabilityList << "IsManageable";
    }
    if (capabilities & Solid::Control::NetworkInterface::SupportsCarrierDetect) {
        capabilityList << "SupportsCarrierDetect";
    }
    setData(name, I18N_NOOP("Capabilities"), capabilityList);

    setData(name, I18N_NOOP("Active"), iface->isActive());
    setData(name, I18N_NOOP("Design Speed"), iface->designSpeed());
    setData(name, I18N_NOOP("Link Up"), iface.isLinkUp());

    signalmanager->mapNetwork(&iface,iface.uni());

    return true;
}

bool NetworkManagerEngine::updateWiredData(Solid::Control::WiredNetworkInterface *iface)
{
    setData(name, I18N_NOOP("Network Type"), iface->type());
    setData(name, I18N_NOOP("Hardware Address"), iface->hardwareAddress());
    setData(name, I18N_NOOP("Bit Rate"), iface->bitRate());
    setData(name, I18N_NOOP("Carrier"), iface->carrier());
}

bool NetworkManagerEngine::updateWirelessData(Solid::Control::WirelessNetworkInterface *iface)
{
    QStringList availableNetworks;
    foreach (const QString &network, iface->accessPoints()) {
        availableNetworks << network;
        accessPointMap[network] = iface->findAccessPoint(network);
    }

    setData(name, I18N_NOOP("Network Type"), iface->type());
    setData(name, I18N_NOOP("Active Access Point"), iface->activeAccessPoint());
    setData(name, I18N_NOOP("Hardware Address"), iface->hardwareAddress());

    switch (wlan->mode()) {
        case Solid::Control::WirelessNetwork::Unassociated:
            setData(name, I18N_NOOP("Mode"), "Unassociated");
            break;
        case Solid::Control::WirelessNetwork::Adhoc:
            setData(name, I18N_NOOP("Mode"), "Adhoc");
            break;
        case Solid::Control::WirelessNetwork::Managed:
            setData(name, I18N_NOOP("Mode"), "Managed");
            break;
        case Solid::Control::WirelessNetwork::Master:
            setData(name, I18N_NOOP("Mode"), "Master");
            break;
        case Solid::Control::WirelessNetwork::Repeater:
            setData(name, I18N_NOOP("Mode"), "Repeater");
            break;
        default:
            setData(name, I18N_NOOP("Mode"), i18n("Unknown"));
            break;
    }
    
    setData(name, I18N_NOOP("Bit Rate"), iface->bitRate());
    setData(name, I18N_NOOP("Wireless Capabilities"), wirelessCapabilitiesToStringList(wirelessCapabilities()));

    setData(name, I18N_NOOP("Encryption"), wlan->isEncrypted());

    return true;
}

bool NetworkManagerEngine::updateNetworkData(Solid::Control::Network *network)
{
    QString uni = network->uni();
    if (!network->isValid()) {
        return false;
    }
    QList<QNetworkAddressEntry> addressList = network->addressEntries();
    QStringList ips, netmasks, broadcasts;
    foreach (const QNetworkAddressEntry &address, addressList) {
        ips << address.ip().toString();
        netmasks << address.netmask().toString();
        broadcasts << address.broadcast().toString();
    }
    setData(uni, I18N_NOOP("IP Addresses"), ips);
    setData(uni, I18N_NOOP("Netmask Addresses"), netmasks);
    setData(uni, I18N_NOOP("Broadcast Addresses"), broadcasts);

    QStringList dnsAddresses;
    foreach (const QHostAddress &address, network->dnsServers()) {
        dnsAddresses << address.toString();
    }

    setData(uni, I18N_NOOP("Gateway"), network->route());
    setData(uni, I18N_NOOP("Active"), network->isActive());
    setData(uni, I18N_NOOP("NetworkType"), "Network");

    signalmanager->mapNetwork(network, uni);

    return true;
}

void NetworkManagerEngine::networkChanged(const QString& uni, const QString &property, const QVariant &value)
{
    if (property == "Network Added") {
        Data data = this->query(uni);
        QStringList availableNetworks = (data.contains("Available Networks")) ? data["Available Networks"].toStringList() : QStringList();
        availableNetworks << value.toString();
        setData(uni, I18N_NOOP("Available Networks"), availableNetworks);
    } else if (property == "Network Deleted") {
        Data data = this->query(uni);
        QStringList availableNetworks = (data.contains("Available Networks")) ? data["Available Networks"].toStringList() : QStringList();
        availableNetworks.removeAll(value.toString());
        setData(uni, I18N_NOOP("Available Networks"), availableNetworks);
        removeSource(value.toString());
    } else if (property == "IP Details Changed") {
        Solid::Control::NetworkInterfaceList iflist = Solid::Control::NetworkManager::networkInterfaces();
        foreach (const Solid::Control::NetworkInterface &iface, iflist) {
            Solid::Control::Network* network = iface.findNetwork(uni);
            if (network->isValid()) {
                updateNetworkData(network);
            }
        }
    } else if (property == "Connection State") {
       setData(uni, property, connectionStateToString(value.toInt()));
    } else {
        setData(uni, property, value);
    }
}

void NetworkManagerEngine::onStatusChanged(Solid::Networking::Status status)
{
    QString name = "Network Management";
    m_status = status;
    setData(name, I18N_NOOP("Status"), statusToString(m_status));

    switch (m_status) {
        case Solid::Networking::Connected:
                //Connecting Network is no longer connecting
                updateActiveIface(m_connectingIface);
                updateConnectingIface(Solid::Control::NetworkInterface());
                removeData("Network Management", I18N_NOOP("Connecting Network"));

                //kDebug() << "Network Manager Connected.  Active Iface: " << m_activeIface.uni();
                setData("Network Management", I18N_NOOP("Active NetworkInterface"), m_activeIface.uni());
            break;
        case Solid::Networking::Connecting:
            //find the connecting network
            foreach (const Solid::Control::NetworkInterface &iface, Solid::Control::NetworkManager::networkInterfaces()) {
                Solid::Control::NetworkInterface::ConnectionState state = iface.connectionState();
                if (state != Solid::Control::NetworkInterface::Activated &&
                    state != Solid::Control::NetworkInterface::Failed &&
                    state != Solid::Control::NetworkInterface::Cancelled &&
                    state != Solid::Control::NetworkInterface::UnknownState) {
                    updateConnectingIface(iface);
                }
            }
            setData("Network Management", I18N_NOOP("Connecting Network"), m_connectingIface.uni());
            break;
        default:
            updateActiveIface(Solid::Control::NetworkInterface());
            removeData("Network Management", I18N_NOOP("Active NetworkInterface"));
            updateConnectingIface(Solid::Control::NetworkInterface());
            removeData("Network Management", I18N_NOOP("Connecting Network"));
    }
    determineIcon();
}

void NetworkManagerEngine::onNetworkInterfaceAdded(const QString &uni)
{
    QStringList ifaces = query("Network Management")["Network Interfaces"].toStringList();
    if(!ifaces.contains(uni)) {
        ifaces << uni;
        setData("Network Management", I18N_NOOP("Network Interfaces"), ifaces);
    }
}

void NetworkManagerEngine::onNetworkInterfaceRemoved(const QString &uni)
{
    QStringList ifaces = query("Network Management")["Network Interfaces"].toStringList();
    if(ifaces.contains(uni)) {
        ifaces.removeAll(uni);
        setData("Network Management", I18N_NOOP("Network Interfaces"), ifaces);
    }

    //remove the interface and all open networks
    foreach (const QString &network, query(uni)["Available Networks"].toStringList()) {
        removeSource(network);
    }
    removeSource(uni);

}

void NetworkManagerEngine::determineIcon()
{
    //kDebug() << "Determining new icon.";
    //kDebug() << "Connection status: " << statusToString(m_status);
    //kDebug() << "Active network: " << m_activeIface.uni();
    
    if (m_status == Solid::Networking::Connected) {
        //connection status is changed to Connected before the device is declared active
        if (!m_activeIface.isValid()) {
            return;
        }
        if (m_activeIface.type() == Solid::Control::NetworkInterface::Ieee8023) {
                setData("Network Management", I18N_NOOP("icon"), "action-nm_device_wired");
                return;
        } else if (m_activeIface.type() == Solid::Control::NetworkInterface::Ieee80211) {
            Solid::Control::WirelessNetwork wifiNet = m_activeNetwork;
            if (wifiNet.mode() == Solid::Control::WirelessNetwork::Adhoc) {
                setData("Network Management", I18N_NOOP("icon"), "action-nm_adhoc");
            } else {
                setData("Network Management", I18N_NOOP("icon"), determineSignalIcon(m_activeIface.signalStrength()));
            }
            return;
        } else {
            kDebug() << "Device type unknown.";
            setData("Network Management", I18N_NOOP("icon"), "action-nm_device_unknown");
            return;
        }
    } else if (m_status == Solid::Networking::Unconnected || m_status == Solid::Networking::Disconnecting) {
        setData("Network Management", I18N_NOOP("icon"), "action-nm_no_connection");
        return;
    } else if (m_status == Solid::Networking::Connecting) {
        setData("Network Management", I18N_NOOP("icon"), determineStageOfConnection(m_connectingIface.connectionState()));
        return;
    }
    setData("Network Management", I18N_NOOP("icon"), "action-nm_unknown_connection");
}

QString NetworkManagerEngine::determineStageOfConnection(const int connectionState)
{
    if (connectionState == Solid::Control::NetworkInterface::Prepare ||
        connectionState == Solid::Control::NetworkInterface::Configure ||
        connectionState == Solid::Control::NetworkInterface::NeedUserKey ||
        connectionState == Solid::Control::NetworkInterface::IPStart) {
        return "action-stage01";
    } else if(connectionState == Solid::Control::NetworkInterface::IPGet) {
        return "action-stage02";
    } else if(connectionState == Solid::Control::NetworkInterface::IPCommit) {
        return "action-stage03";
    } else {
        return "action-nm_no_connection";
    }
}

QString NetworkManagerEngine::determineSignalIcon(int strength)
{
    if(strength > m_lastSignalStrength-signalStrengthResolution-hysteresis
       && strength <= m_lastSignalStrength+signalStrengthResolution+hysteresis) {
        return QString("action-nm_signal_%1").arg(m_lastSignalStrength, 2, 10, QLatin1Char('0'));
    } else {
        //HACK: this was hardcoded to a resolution of 25 due to the inefficiencies of a variable resolution
        if (strength <= signalStrengthResolution/2) {
            return "action-nm_signal_00";
        } else if (strength <= 25+signalStrengthResolution/2) {
            return "action-nm_signal_25";
        } else if (strength <= 50+signalStrengthResolution/2) {
            return "action-nm_signal_50";
        } else if (strength <= 75+signalStrengthResolution/2) {
            return "action-nm_signal_75";
        } else if (strength <= 100) {
            return "action-nm_signal_100";
        } else {
            return "action-nm_no_connection";
        }
    }
}

#include "networkmanagerengine.moc"
