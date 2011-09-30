/*  This file is part of the KDE project
    Copyright (C) 2006 Kevin Ottens <ervin@kde.org>

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

#include "solid-network.h"


#include <QString>
#include <QStringList>
#include <QMetaProperty>
#include <QMetaEnum>
#include <QTimer>

#include <kcomponentdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kdebug.h>

#include <solid/device.h>
#include <solid/genericinterface.h>
#include <solid/storageaccess.h>
#include <solid/opticaldrive.h>

#include <solid/control/ifaces/authentication.h>
#include <solid/control/networkmanager.h>
#include <solid/control/networkinterface.h>
#include <solid/control/wirednetworkinterface.h>
#include <solid/control/wirelessnetworkinterface.h>
#include <solid/control/wirelessaccesspoint.h>

#include <kjob.h>


#include <iostream>
using namespace std;

static const char appName[] = "solid-network";
static const char programName[] = I18N_NOOP("solid-network");

static const char description[] = I18N_NOOP("KDE tool for querying and controlling your network interfaces from the command line");

static const char version[] = "0.1";

std::ostream &operator<<(std::ostream &out, const QString &msg)
{
    return (out << msg.toLocal8Bit().constData());
}

std::ostream &operator<<(std::ostream &out, const QVariant &value)
{
    switch (value.type())
    {
    case QVariant::StringList:
    {
        out << "{";

        QStringList list = value.toStringList();

        QStringList::ConstIterator it = list.constBegin();
        QStringList::ConstIterator end = list.constEnd();

        for (; it!=end; ++it)
        {
            out << "'" << *it << "'";

            if (it+1!=end)
            {
                out << ", ";
            }
        }

        out << "}  (string list)";
        break;
    }
    case QVariant::Bool:
        out << (value.toBool()?"true":"false") << "  (bool)";
        break;
    case QVariant::Int:
        out << value.toString()
            << "  (0x" << QString::number(value.toInt(), 16) << ")  (int)";
        break;
    default:
        out << "'" << value.toString() << "'  (string)";
        break;
    }

    return out;
}

std::ostream &operator<<(std::ostream &out, const Solid::Device &device)
{
    out << "  parent = " << QVariant(device.parentUdi()) << endl;
    out << "  vendor = " << QVariant(device.vendor()) << endl;
    out << "  product = " << QVariant(device.product()) << endl;

    int index = Solid::DeviceInterface::staticMetaObject.indexOfEnumerator("Type");
    QMetaEnum typeEnum = Solid::DeviceInterface::staticMetaObject.enumerator(index);

    for (int i=0; i<typeEnum.keyCount(); i++)
    {
        Solid::DeviceInterface::Type type = (Solid::DeviceInterface::Type)typeEnum.value(i);
        const Solid::DeviceInterface *interface = device.asDeviceInterface(type);

        if (interface)
        {
            const QMetaObject *meta = interface->metaObject();

            for (int i=meta->propertyOffset(); i<meta->propertyCount(); i++)
            {
                QMetaProperty property = meta->property(i);
                out << "  " << QString(meta->className()).mid(7) << "." << property.name()
                    << " = ";

                QVariant value = property.read(interface);

                if (property.isEnumType()) {
                    QMetaEnum metaEnum = property.enumerator();
                    out << "'" << metaEnum.valueToKeys(value.toInt()).constData() << "'"
                        << "  (0x" << QString::number(value.toInt(), 16) << ")  ";
                    if (metaEnum.isFlag()) {
                        out << "(flag)";
                    } else {
                        out << "(enum)";
                    }
                    out << endl;
                } else {
                    out << value << endl;
                }
            }
        }
    }

    return out;
}

std::ostream &operator<<(std::ostream &out, const QMap<QString,QVariant> &properties)
{
    QMap<QString, QVariant>::ConstIterator it = properties.constBegin(), itEnd = properties.constEnd();
    for ( ; it != itEnd; ++it)
    {
        out << "  " << it.key() << " = " << it.value() << endl;
    }

    return out;
}

std::ostream &operator<<(std::ostream &out, const Solid::Control::NetworkInterface &networkdevice)
{
    out << "  UNI =                " << QVariant(networkdevice.uni()) << endl;
    out << "  Type =               " << (networkdevice.type() == Solid::Control::NetworkInterface::Ieee8023 ? "Wired" : "802.11 Wireless") << endl;
    out << "  Active =             " << (networkdevice.isActive() ? "Yes" : "No") << endl;
    out << "  Interface Name =     " << networkdevice.interfaceName() << endl;
    out << "  Driver =             " << networkdevice.driver() << endl;
    //out << "  HW Address =         " << networkdevice.  // TODO add to solid API.
    out << "\n  Capabilities:" << endl;
    out << "    Supported =        " << (networkdevice.capabilities()  & Solid::Control::NetworkInterface::IsManageable ? "Yes" : "No") << endl;
    out << "    Speed =            " << networkdevice.designSpeed() << endl;
#if 0
    if (networkdevice.type() == Solid::Control::NetworkInterface::Ieee8023) {

        out << "    Carrier Detect =   " << (networkdevice.capabilities()  & Solid::Control::NetworkInterface::SupportsCarrierDetect ? "Yes" : "No") << endl;
    }
    else {
        out << "    Wireless Scan =    " << (networkdevice.capabilities()  & Solid::Control::NetworkInterface::SupportsWirelessScan ? "Yes" : "No") << endl;
    }
    out << "    Link Up =          " << (networkdevice.isLinkUp() ? "Yes" : "No") << endl;
#endif

    return out;
}

std::ostream &operator<<(std::ostream &out, const Solid::Control::AccessPoint &ap)
{
    out << "  UNI =                " << QVariant(ap.uni()) << endl;
    out << "  SSID =               " << QVariant(ap.ssid()) << endl;
    out << "  MAC Address =        " << QVariant(ap.hardwareAddress()) << endl;
    out << "  Frequency (MHz) =    " << ap.frequency() << endl;
    out << "  Max BitRate (Kb/s) = " << ap.maxBitRate() << endl;
    out << "  Signal Strength =    " << ap.signalStrength() << endl;
    out << "  Mode =               ";
    switch (ap.mode())
    {
    case Solid::Control::WirelessNetworkInterface::Unassociated:
        cout << "Unassociated" << endl;
        break;
    case Solid::Control::WirelessNetworkInterface::Adhoc:
        cout << "Ad-hoc" << endl;
        break;
    case Solid::Control::WirelessNetworkInterface::Managed:
        cout << "Infrastructure" << endl;
        break;
    case Solid::Control::WirelessNetworkInterface::Master:
        cout << "Master" << endl;
        break;
    case Solid::Control::WirelessNetworkInterface::Repeater:
        cout << "Repeater" << endl;
        break;
    default:
        cout << "Unknown" << endl;
        cerr << "Unknown network operation mode: " << ap.mode() << endl;
        break;
    }
    out << "  Capabilities =       ";
    const Solid::Control::AccessPoint::Capabilities cap = ap.capabilities();
    if (cap)
    {
        if (cap  & Solid::Control::AccessPoint::Privacy)
            out << "Privacy,";
        out << endl;
    }
    else
    {
        out << "(No Capabilities)" << endl;
    }
    out << "  WPA Options =        ";
    const Solid::Control::AccessPoint::WpaFlags wpaFlags = ap.wpaFlags();
    if (wpaFlags)
    {
        if (wpaFlags  & Solid::Control::AccessPoint::PairWep40)
            out << "PairWep40,";
        if (wpaFlags  & Solid::Control::AccessPoint::PairWep104)
            out << "PairWep104,";
        if (wpaFlags  & Solid::Control::AccessPoint::PairTkip)
            out << "PairTkip,";
        if (wpaFlags  & Solid::Control::AccessPoint::PairCcmp)
            out << "PairCcmp,";
        if (wpaFlags  & Solid::Control::AccessPoint::GroupWep40)
            out << "GroupWep40,";
        if (wpaFlags  & Solid::Control::AccessPoint::GroupWep104)
            out << "GroupWep104,";
        if (wpaFlags  & Solid::Control::AccessPoint::GroupTkip)
            out << "GroupTkip,";
        if (wpaFlags  & Solid::Control::AccessPoint::GroupCcmp)
            out << "GroupCcmp,";
        if (wpaFlags  & Solid::Control::AccessPoint::KeyMgmtPsk)
            out << "KeyMgmtPsk,";
        if (wpaFlags  & Solid::Control::AccessPoint::KeyMgmt8021x)
            out << "KeyMgmt8021x,";
        out << endl;
    }
    else
    {
        out << "(No Options)" << endl;
    }
    out << "  RSN Options =        ";
    const Solid::Control::AccessPoint::WpaFlags rsnFlags = ap.rsnFlags();
    if (rsnFlags)
    {
        if (rsnFlags  & Solid::Control::AccessPoint::PairWep40)
            out << "PairWep40,";
        if (rsnFlags  & Solid::Control::AccessPoint::PairWep104)
            out << "PairWep104,";
        if (rsnFlags  & Solid::Control::AccessPoint::PairTkip)
            out << "PairTkip,";
        if (rsnFlags  & Solid::Control::AccessPoint::PairCcmp)
            out << "PairCcmp,";
        if (rsnFlags  & Solid::Control::AccessPoint::GroupWep40)
            out << "GroupWep40,";
        if (rsnFlags  & Solid::Control::AccessPoint::GroupWep104)
            out << "GroupWep104,";
        if (rsnFlags  & Solid::Control::AccessPoint::GroupTkip)
            out << "GroupTkip,";
        if (rsnFlags  & Solid::Control::AccessPoint::GroupCcmp)
            out << "GroupCcmp,";
        if (rsnFlags  & Solid::Control::AccessPoint::KeyMgmtPsk)
            out << "KeyMgmtPsk,";
        if (rsnFlags  & Solid::Control::AccessPoint::KeyMgmt8021x)
            out << "KeyMgmt8021x,";
        out << endl;
    }
    else
    {
        out << "(No Options)" << endl;
    }
    return out;
}

std::ostream &operator<<(std::ostream &out, const Solid::Control::WirelessNetworkInterface &network)
{
    out << static_cast<const Solid::Control::NetworkInterface&>(network);
    out << endl;
    out << "  Mode =               ";
    switch (network.mode())
    {
    case Solid::Control::WirelessNetworkInterface::Unassociated:
        cout << "Unassociated" << endl;
        break;
    case Solid::Control::WirelessNetworkInterface::Adhoc:
        cout << "Ad-hoc" << endl;
        break;
    case Solid::Control::WirelessNetworkInterface::Managed:
        cout << "Infrastructure" << endl;
        break;
    case Solid::Control::WirelessNetworkInterface::Master:
        cout << "Master" << endl;
        break;
    case Solid::Control::WirelessNetworkInterface::Repeater:
        cout << "Repeater" << endl;
        break;
    default:
        cout << "Unknown" << endl;
        cerr << "Unknown network operation mode: " << network.mode() << endl;
        break;
    }
    out << "  Bit Rate =           " << network.bitRate() << endl;
    out << "  Hardware Address =   " << network.hardwareAddress() << endl;
    out << "  Active Access Point= " << qVariantFromValue(network.activeAccessPoint()) << endl;
    out << "  Capabilities =       ";
    const Solid::Control::WirelessNetworkInterface::Capabilities cap = network.wirelessCapabilities();
    if (cap)
    {
        if (cap  & Solid::Control::WirelessNetworkInterface::Wpa)
            out << "WPA,";
        if (cap  & Solid::Control::WirelessNetworkInterface::Wep40)
            out << "WEP40,";
        if (cap  & Solid::Control::WirelessNetworkInterface::Wep104)
            out << "WEP104,";
        if (cap  & Solid::Control::WirelessNetworkInterface::Tkip)
            out << "TKIP,";
        if (cap  & Solid::Control::WirelessNetworkInterface::Ccmp)
            out << "CCMP,";
        if (cap  & Solid::Control::WirelessNetworkInterface::Rsn)
            out << "RSN,";
        out << endl;
    }
    else
    {
        out << "(No Capabilities)" << endl;
    }
    return out;
}

std::ostream &operator<<(std::ostream &out, const Solid::Control::WiredNetworkInterface &network)
{
    out << static_cast<const Solid::Control::NetworkInterface&>(network);
    out << endl;
    out << "  Hardware Address =   " << network.hardwareAddress() << endl;
    out << "  Bit Rate =           " << network.bitRate() << endl;
    out << "  Carrier =            " << qVariantFromValue(network.carrier()) << endl;
    return out;
}


void checkArgumentCount(int min, int max)
{
    int count = KCmdLineArgs::parsedArgs()->count();

    if (count < min)
    {
        cerr << i18n("Syntax Error: Not enough arguments") << endl;
        ::exit(1);
    }

    if ((max > 0) && (count > max))
    {
        cerr << i18n("Syntax Error: Too many arguments") << endl;
        ::exit(1);
    }
}

int main(int argc, char **argv)
{
  KCmdLineArgs::init(argc, argv, appName, 0, ki18n(programName), version, ki18n(description), KCmdLineArgs::CmdLineArgNone);


  KCmdLineOptions options;

  options.add("commands", ki18n("Show available commands by domains"));

  options.add("+command", ki18n("Command (see --commands)"));

  options.add("+[arg(s)]", ki18n("Arguments for command"));

  KCmdLineArgs::addCmdLineOptions(options);

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  KComponentData componentData(appName);

  if (args->isSet("commands"))
  {
      KCmdLineArgs::enable_i18n();

      cout << endl << i18n("Syntax:") << endl << endl;

      cout << "  solid-network listdevices" << endl;
      cout << i18n("             # List the network devices present.\n") << endl;

      cout << "  solid-network listnetworks 'uni'" << endl;
      cout << i18n("             # List the networks known to the device specified by 'uni'.\n") << endl;

      cout << "  solid-network query (status|wireless|wireless-hardware)|(interface 'uni')|(network 'device-uni' 'network-uni')" << endl;
      cout << i18n("             # Query whether networking features are active or not.\n"
                    "             # - If the 'status' option is given, return whether\n"
                    "             # networking is enabled for the system\n"
                    "             # - If the 'wireless' option is given, return whether\n"
                    "             # wireless is enabled for the system\n"
                    "             # - If the 'wireless-hardware' option is given,\n"
                    "             #  return whether the wireless hardware is enabled\n"
                    "             # - If the 'interface' option is given, print the\n"
                    "             # properties of the network interface that 'uni' refers to.\n"
                    "             # - If the 'network' option is given, print the\n"
                    "             # properties of the network on 'device-uni' that 'network-uni' refers to.\n") << endl;

      cout << "  solid-network set wireless (enabled|disabled)" << endl;
      cout << i18n("             # Enable or disable networking on this system.\n") << endl;

      cout << "  solid-network set networking (enabled|disabled)" << endl;
      cout << i18n("             # Enable or disable networking on this system.\n") << endl;

      cout << "  solid-network set network 'device-uni' 'network-uni' [authentication 'key']" << endl;
      cout << i18n("             # Activate the network 'network-uni' on 'device-uni'.\n"
                    "             # Optionally, use WEP128, open-system encryption with hex key 'key'. (Hardcoded)\n"
                    "             # Where 'authentication' is one of:\n"
                    "             # wep hex64|ascii64|hex128|ascii128|passphrase64|passphrase128 'key' [open|shared]\n"
                    "             # wpapsk wpa|wpa2 tkip|ccmp-aes password\n"
                    "             # wpaeap UNIMPLEMENTED IN SOLIDSHELL\n") << endl;

      cout << endl;

      return 0;
  }

  return SolidNetwork::doIt() ? 0 : 1;
}

bool SolidNetwork::doIt()
{
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    checkArgumentCount(1, 0);

    QString command(args->arg(0));

    int fake_argc = 0;
    char **fake_argv = 0;
    SolidNetwork shell(fake_argc, fake_argv);

    if (command == "query")
    {
        checkArgumentCount(2, 4);
        QString what(args->arg(1));
        if (what == "status")
            return shell.netmgrNetworkingEnabled();
        else if (what == "wireless")
            return shell.netmgrWirelessEnabled();
        else if (what == "wireless-hardware")
            return shell.netmgrWirelessHardwareEnabled();
        else if (what == "interface")
        {
            checkArgumentCount(3, 3);
            QString uni(args->arg(2));
            return shell.netmgrQueryNetworkInterface(uni);
        }
        else if (what == "network")
        {
            checkArgumentCount(4, 4);
            QString dev(args->arg(2));
            QString uni(args->arg(3));
            return shell.netmgrQueryNetwork(dev, uni);
        }
        else
            cerr << i18n("Syntax Error: Unknown option '%1'", what) << endl;
    }
    else if (command == "set")
    {
        checkArgumentCount(3, 9);
        QString what(args->arg(1));
        QString how(args->arg(2));
        if (what == "networking")
        {
            bool enabled;
            if (how == "enabled")
            {
                enabled = true;
            }
            else if (how == "disabled")
            {
                enabled = false;
            }
            else
            {
                cerr << i18n("Syntax Error: Unknown option '%1'", how) << endl;
                return false;
            }
            shell.netmgrChangeNetworkingEnabled(enabled);
            return true;
        }
        else if (what == "wireless")
        {
            bool enabled;
            if (how == "enabled")
            {
                enabled = true;
            }
            else if (how == "disabled")
            {
                enabled = false;
            }
            else
            {
                cerr << i18n("Syntax Error: Unknown option '%1'", how) << endl;
                return false;
            }
            shell.netmgrChangeWirelessEnabled(enabled);
            return true;
        }
    /*cout << "  solid-network set network 'device-uni' 'network-uni' [authentication 'key']" << endl; */
        /*wep hex64|ascii64|hex128|ascii128|passphrase 'key' [open|shared] */
        /* wpaeap UNIMPLEMENTED */
        else if (what == "network")
        {
            cerr << i18n("Not implemented");
#if 0 // probably won't be reimplemented since solidshell can't provide a persistent settings service...
            checkArgumentCount(4, 9);
            QString dev(args->arg(2));
            QString uni(args->arg(3));
            Solid::Control::Authentication * auth = 0;
            QMap<QString,QString> secrets;

            if (KCmdLineArgs::parsedArgs()->count() > 4)
            {
                QString hasAuth = args->arg(4);
                if (hasAuth == "authentication")
                {
                    //encrypted network
                    QString authScheme = args->arg(5);
                    if (authScheme == "wep")
                    {
                        Solid::Control::AuthenticationWep *wepAuth = new Solid::Control::AuthenticationWep();
                        QString keyType = args->arg(6);
                        if (keyType == "hex64")
                        {
                            wepAuth->setType(Solid::Control::AuthenticationWep::WepHex);
                            wepAuth->setKeyLength(64);
                        }
                        else if (keyType == "ascii64")
                        {
                            wepAuth->setType(Solid::Control::AuthenticationWep::WepAscii);
                            wepAuth->setKeyLength(64);
                        }
                        else if (keyType == "hex128")
                        {
                            wepAuth->setType(Solid::Control::AuthenticationWep::WepHex);
                            wepAuth->setKeyLength(128);
                        }
                        else if (keyType == "ascii128")
                        {
                            wepAuth->setType(Solid::Control::AuthenticationWep::WepAscii);
                            wepAuth->setKeyLength(128);
                        }
                        else if (keyType == "passphrase64")
                        {
                            wepAuth->setType(Solid::Control::AuthenticationWep::WepPassphrase);
                            wepAuth->setKeyLength(64);
                        }
                        else if (keyType == "passphrase128")
                        {
                            wepAuth->setType(Solid::Control::AuthenticationWep::WepPassphrase);
                            wepAuth->setKeyLength(128);
                        }
                        else
                        {
                            cerr << i18n("Unrecognised WEP type '%1'", keyType) << endl;
                            delete wepAuth;
                            return false;
                        }
    
                        QString key = args->arg(7);
                        secrets.insert("key", key);
                        wepAuth->setSecrets(secrets);
    
                        QString method = args->arg(8);
                        if (method == "open")
                            wepAuth->setMethod(Solid::Control::AuthenticationWep::WepOpenSystem);
                        else if (method == "shared")
                            wepAuth->setMethod(Solid::Control::AuthenticationWep::WepSharedKey);
                        else
                        {
                            cerr << i18n("Unrecognised WEP method '%1'", method) << endl;
                            delete wepAuth;
                            return false;
                        }
                        auth = wepAuth;
                    }
                    else if (authScheme == "wpapsk")
                    {
                        /* wpapsk wpa|wpa2 tkip|ccmp-aes password */
                        Solid::Control::AuthenticationWpaPersonal *wpapAuth = new Solid::Control::AuthenticationWpaPersonal();
                        QString version = args->arg(6);
                        if (version == "wpa")
                            wpapAuth->setVersion(Solid::Control::AuthenticationWpaPersonal::Wpa1);
                        else if (version == "wpa2")
                            wpapAuth->setVersion(Solid::Control::AuthenticationWpaPersonal::Wpa1);
                        else
                        {
                            cerr << i18n("Unrecognised WPA version '%1'", version) << endl;
                            delete wpapAuth;
                            return false;
                        }
                        QString protocol = args->arg(7);
                        if (protocol == "tkip")
                            wpapAuth->setProtocol(Solid::Control::AuthenticationWpaPersonal::WpaTkip);
                        else if (protocol == "ccmp-aes")
                            wpapAuth->setProtocol(Solid::Control::AuthenticationWpaPersonal::WpaCcmpAes);
                        else
                        {
                            cerr << i18n("Unrecognised WPA encryption protocol '%1'", protocol) << endl;
                            delete wpapAuth;
                            return false;
                        }
                        QString key = args->arg(8);
                        secrets.insert("key", key);
                        wpapAuth->setSecrets(secrets);
                        auth = wpapAuth;
                    }
                    else
                    {
                        cerr << i18n("Unimplemented auth scheme '%1'", args->arg(5)) << endl;
                        return false;
                    }
                }
            }
            else
            {
                //unencrypted network
                auth = new Solid::Control::AuthenticationNone;
            }

            return shell.netmgrActivateNetwork(dev, uni, auth);
            delete auth;
#endif
        }
        else
        {
            cerr << i18n("Syntax Error: Unknown object '%1'", what) << endl;
            return false;
        }
    }
    else if (command == "listdevices")
    {
        return shell.netmgrList();
    }
    else if (command == "listnetworks")
    {
        checkArgumentCount(2, 2);
        QString device(args->arg(1));
        return shell.netmgrListNetworks(device);
    }
    else
    {
        cerr << i18n("Syntax Error: Unknown command '%1'" , command) << endl;
    }

    return false;
}

bool SolidNetwork::netmgrNetworkingEnabled()
{
    if (Solid::Control::NetworkManager::isNetworkingEnabled())
        cout << i18n("networking: is enabled")<< endl;
    else
        cout << i18n("networking: is not enabled")<< endl;
    return Solid::Control::NetworkManager::isNetworkingEnabled();
}

bool SolidNetwork::netmgrWirelessEnabled()
{
    if (Solid::Control::NetworkManager::isWirelessEnabled())
        cout << i18n("wireless: is enabled")<< endl;
    else
        cout << i18n("wireless: is not enabled")<< endl;
    return Solid::Control::NetworkManager::isWirelessEnabled();
}

bool SolidNetwork::netmgrWirelessHardwareEnabled()
{
    if (Solid::Control::NetworkManager::isWirelessHardwareEnabled())
        cout << i18n("wireless hardware: is enabled")<< endl;
    else
        cout << i18n("wireless hardware: is not enabled")<< endl;
    return Solid::Control::NetworkManager::isWirelessHardwareEnabled();
}

bool SolidNetwork::netmgrChangeNetworkingEnabled(bool enabled)
{
    Solid::Control::NetworkManager::setNetworkingEnabled(enabled);
    return true;
}

bool SolidNetwork::netmgrChangeWirelessEnabled(bool enabled)
{
    Solid::Control::NetworkManager::setWirelessEnabled(enabled);
    return true;
}

bool SolidNetwork::netmgrList()
{
    const Solid::Control::NetworkInterfaceList all = Solid::Control::NetworkManager::networkInterfaces();

    cerr << "debug: network interface list contains: " << all.count() << " entries" << endl;
    foreach (const Solid::Control::NetworkInterface *device, all)
    {
        cout << "UNI = '" << device->uni() << "'" << endl;
    }
    return true;
}

bool SolidNetwork::netmgrListNetworks(const QString  & deviceUni)
{
    Solid::Control::NetworkInterface * device = Solid::Control::NetworkManager::findNetworkInterface(deviceUni);
    Solid::Control::WirelessNetworkInterface * wifiDev =  qobject_cast<Solid::Control::WirelessNetworkInterface *>(device );
    if (wifiDev) {

        Solid::Control::AccessPointList aps = wifiDev->accessPoints();
        foreach (const QString &apUni, aps)
        {
            cout << "NETWORK UNI = '" << apUni << "'" << endl;
        }

        return true;
    }
    return false;
}

bool SolidNetwork::netmgrQueryNetworkInterface(const QString  & deviceUni)
{
    cerr << "SolidNetwork::netmgrQueryNetworkInterface()" << endl;
    Solid::Control::NetworkInterface * device = Solid::Control::NetworkManager::findNetworkInterface(deviceUni);
    if (!device) {
        cerr << "No such interface: " << deviceUni << endl;
        return false;
    }
    Solid::Control::WirelessNetworkInterface * wifiDev =  qobject_cast<Solid::Control::WirelessNetworkInterface *>(device);
    Solid::Control::WiredNetworkInterface * wiredDev =  qobject_cast<Solid::Control::WiredNetworkInterface *>(device);
    if (wifiDev) {
        cout << *wifiDev << endl;
    } else if (wiredDev) {
        cout << *wiredDev << endl;
    } else {
        cout << *device << endl;
    }
    return true;
}

bool SolidNetwork::netmgrQueryNetwork(const QString  & deviceUni, const QString  & apUni)
{
    cerr << "SolidNetwork::netmgrQueryNetwork()" << endl;
    Solid::Control::NetworkInterface * device = Solid::Control::NetworkManager::findNetworkInterface(deviceUni);
    Solid::Control::WirelessNetworkInterface * wifiDev =  qobject_cast<Solid::Control::WirelessNetworkInterface *>(device );
    if (wifiDev) {
        Solid::Control::AccessPoint * ap = wifiDev->findAccessPoint( apUni );
        if ( ap ) {
            cout << *ap << endl;
            return true;
        }
    }
    return false;
}

#if 0
bool SolidNetwork::netmgrActivateNetwork(const QString  & deviceUni, const QString  & networkUni, Solid::Control::Authentication * auth)
{
    Solid::Control::NetworkInterface * device = Solid::Control::NetworkManager::findNetworkInterface(deviceUni);
    Solid::Control::Network * network = device.findNetwork(networkUni);
    Solid::Control::WirelessNetwork * wlan = 0;
    if (( wlan = qobject_cast<Solid::Control::WirelessNetwork *>(network)))
    {
        wlan->setAuthentication(auth);
        wlan->setActivated(true);
    }
    else
        network->setActivated(true);
    return true;
}
#endif

void SolidNetwork::connectJob(KJob *job)
{
    connect(job, SIGNAL(result(KJob*)),
             this, SLOT(slotResult(KJob*)));
    connect(job, SIGNAL(percent(KJob*,ulong)),
             this, SLOT(slotPercent(KJob*,ulong)));
    connect(job, SIGNAL(infoMessage(KJob*,QString,QString)),
             this, SLOT(slotInfoMessage(KJob*,QString)));
}

void SolidNetwork::slotPercent(KJob */*job */, unsigned long percent)
{
    cout << i18n("Progress: %1%" , percent) << endl;
}

void SolidNetwork::slotInfoMessage(KJob */*job */, const QString &message)
{
    cout << i18n("Info: %1" , message) << endl;
}

void SolidNetwork::slotResult(KJob *job)
{
    m_error = 0;

    if (job->error())
    {
        m_error = job->error();
        m_errorString = job->errorString();
    }

    m_loop.exit();
}

void SolidNetwork::slotStorageResult(Solid::ErrorType error, const QVariant &errorData)
{
    if (error) {
        m_error = 1;
        m_errorString = errorData.toString();
    }
    m_loop.exit();
}

#include "solid-network.moc"
