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

#ifndef MM04_MODEMGSMUSSDINTERFACE_H
#define MM04_MODEMGSMUSSDINTERFACE_H

#include "modeminterface.h"
#include "solid/control/ifaces/modemgsmussdinterface.h"
#include "dbus/generic-types.h"

class MMModemGsmUssdInterfacePrivate;

class KDE_EXPORT MMModemGsmUssdInterface : public MMModemInterface, virtual public Solid::Control::Ifaces::ModemGsmUssdInterface
{
Q_OBJECT
Q_DECLARE_PRIVATE(MMModemGsmUssdInterface)
Q_INTERFACES(Solid::Control::Ifaces::ModemGsmUssdInterface)

public:
    MMModemGsmUssdInterface(const QString & path, MMModemManager * manager, QObject * parent);
    ~MMModemGsmUssdInterface();

    QString initiate(const QString & command);
    void respond(const QString response);
    void cancel();
    // properties
    QString getState();
    QString getNetworkNotification();
    QString getNetworkRequest();
public Q_SLOTS:
    void propertiesChanged(const QString & interface, const QVariantMap & properties);
Q_SIGNALS:
    // properties
    void stateChanged(const QString & state);
    void networkNotificationChanged(const QString & networkNotification);
    void networkRequestChanged(const QString & networkRequest);
};

#endif

