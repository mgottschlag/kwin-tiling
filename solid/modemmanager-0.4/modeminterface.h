/*
Copyright 2008 Will Stephenson <wstephenson@kde.org>
Copyright 2010 Lamarque Souza <lamarque@gmail.com>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of
the License or (at your option) version 3 or any later version
accepted by the membership of KDE e.V. (or its successor approved
by the membership of KDE e.V.), which shall act as a prox
defined in Section 14 of version 3 of the license.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef MM04_MODEMINTERFACE_H
#define MM04_MODEMINTERFACE_H

#include "solid/control/solid_control_export.h"
#include "solid/control/ifaces/modeminterface.h"

class MMModemInterfacePrivate;
class MMModemManager;

class KDE_EXPORT MMModemInterface : public QObject, virtual public Solid::Control::Ifaces::ModemInterface
{
Q_OBJECT
Q_DECLARE_PRIVATE(MMModemInterface)
Q_INTERFACES(Solid::Control::Ifaces::ModemInterface)
Q_PROPERTY(QString udi READ udi)

public:
    MMModemInterface( const QString & path, MMModemManager * manager, QObject * parent );
    MMModemInterface( MMModemInterfacePrivate &dd, MMModemManager * manager, QObject * parent );
    virtual ~MMModemInterface();

    QString udi() const;

    // From org.freedesktop.ModemManager.Modem
    void enable(const bool enable);
    void connectModem(const QString & number);
    void disconnectModem();
    Solid::Control::ModemInterface::Ip4ConfigType getIp4Config();
    Solid::Control::ModemInterface::InfoType getInfo();

    QString device() const;
    QString masterDevice() const;
    QString driver() const;
    Solid::Control::ModemInterface::Type type() const;
    bool enabled() const;
    QString unlockRequired() const;
    Solid::Control::ModemInterface::Method ipMethod() const;

    // From org.freedesktop.ModemManager.Modem.Simple
    void connectModem(const QVariantMap & properties);
    QVariantMap getStatus();
public Q_SLOTS:
    void propertiesChanged(const QString & interface, const QVariantMap & properties);

Q_SIGNALS:
    void deviceChanged(const QString & device);
    void masterDeviceChanged(const QString & masterDevice);
    void driverChanged(const QString & driver);
    void typeChanged(const Solid::Control::ModemInterface::Type type);
    void enabledChanged(const bool enabled);
    void unlockRequiredChanged(const QString & codeRequired);
    void ipMethodChanged(const Solid::Control::ModemInterface::Method ipMethod);

private:
    void init();

protected:
    MMModemInterfacePrivate * d_ptr;
};

#endif

