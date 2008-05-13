/*
Copyright 2008 Will Stephenson <wstephenson@kde.org>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of
the License or (at your option) version 3 or any later version
accepted by the membership of KDE e.V. (or its successor approved
by the membership of KDE e.V.), which shall act as a proxy 
defined in Section 14 of version 3 of the license.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef NM07_NETWORKINTERFACE_H
#define NM07_NETWORKINTERFACE_H

#include <solid/control/solid_control_export.h>
#include <solid/control/networkinterface.h>
#include <solid/control/ifaces/networkinterface.h>

class NMNetworkInterfacePrivate;
class NMNetworkManager;

class KDE_EXPORT NMNetworkInterface : public QObject, virtual public Solid::Control::Ifaces::NetworkInterface
{
Q_OBJECT
Q_DECLARE_PRIVATE(NMNetworkInterface)
Q_INTERFACES(Solid::Control::Ifaces::NetworkInterface)
Q_PROPERTY(QString uni READ uni WRITE setUni)
Q_PROPERTY(QString interfaceName READ interfaceName WRITE setInterfaceName)
Q_PROPERTY(QString driver READ driver WRITE setDriver)
Q_PROPERTY(QVariant genericCapabilities READ capabilitiesV WRITE setCapabilitiesV)
Q_PROPERTY(bool managed READ managed WRITE setManaged)
//Q_PROPERTY(Solid::Control::IPv4Config ipV4Config READ ipV4Config WRITE setIpV4Config)
Q_PROPERTY(Solid::Control::NetworkInterface::ConnectionState connectionState READ connectionState WRITE setConnectionState)
Q_FLAGS(Solid::Control::NetworkInterface::Capabilities)

public:
    NMNetworkInterface( const QString & path, NMNetworkManager * manager, QObject * parent );
    NMNetworkInterface( NMNetworkInterfacePrivate &dd, NMNetworkManager * manager, QObject * parent );
    ~NMNetworkInterface();
    QString uni() const;
    void setUni(const QVariant&);
    QString interfaceName() const;
    void setInterfaceName(const QVariant&);
    QString driver() const;
    void setDriver(const QVariant&);
    Solid::Control::IPv4Config ipV4Config() const;
    void setIpV4Config(const QVariant&);
    bool isActive() const;
    Solid::Control::NetworkInterface::ConnectionState connectionState() const;
    void setConnectionState(const QVariant&);
    int designSpeed() const;
    bool isLinkUp() const;
    Solid::Control::NetworkInterface::Capabilities capabilities() const;
    void setCapabilities(const QVariant&);
    QVariant capabilitiesV() const;
    void setCapabilitiesV(const QVariant&);
    bool managed() const;
    void setManaged(const QVariant&);

    QString activeConnection() const;

Q_SIGNALS:
    void ipDetailsChanged();
    //void linkUpChanged(bool linkActivated);
    void connectionStateChanged(int state);
protected Q_SLOTS:
    void stateChanged(uint);
private:
    Solid::Control::NetworkInterface::Capabilities convertCapabilities(uint theirCaps);
    Solid::Control::NetworkInterface::ConnectionState convertState(uint theirState);
protected:
    NMNetworkInterfacePrivate * d_ptr;
};

#endif

