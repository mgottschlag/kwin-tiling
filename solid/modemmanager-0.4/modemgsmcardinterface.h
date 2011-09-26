/*
Copyright 2008 Will Stephenson <wstephenson@kde.org>
Copyright 2010 Lamarque Souza <lamarque@gmail.com>

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

#ifndef MM04_MODEMGSMCARDINTERFACE_H
#define MM04_MODEMGSMCARDINTERFACE_H

#include "modeminterface.h"
#include "solid/control/ifaces/modemgsmcardinterface.h"
#include "dbus/generic-types.h"

class MMModemGsmCardInterfacePrivate;

class KDE_EXPORT MMModemGsmCardInterface : public MMModemInterface, virtual public Solid::Control::Ifaces::ModemGsmCardInterface
{
Q_OBJECT
Q_DECLARE_PRIVATE(MMModemGsmCardInterface)
Q_INTERFACES(Solid::Control::Ifaces::ModemGsmCardInterface)

public:
    MMModemGsmCardInterface(const QString & path, MMModemManager * manager, QObject * parent);
    ~MMModemGsmCardInterface();

    QString getImei();
    QString getImsi();
    QDBusPendingReply<> sendPuk(const QString & puk, const QString & pin);
    QDBusPendingReply<> sendPin(const QString & pin);
    QDBusPendingReply<> enablePin(const QString & pin, const bool enabled);
    QDBusPendingReply<> changePin(const QString & oldPin, const QString & newPin);

    // Properties
    Solid::Control::ModemInterface::Band getSupportedBands() const;
    Solid::Control::ModemInterface::Mode getSupportedModes() const;
public Q_SLOTS:
    void propertiesChanged(const QString & interface, const QVariantMap & properties);
Q_SIGNALS:
    // properties
    void supportedBandsChanged(const Solid::Control::ModemInterface::Band band);
    void supportedModesChanged(const Solid::Control::ModemInterface::Mode modes);

};

#endif

