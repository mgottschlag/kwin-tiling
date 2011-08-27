/*
Copyright 2008 Will Stephenson <wstephenson@kde.org>
Copyright 2010 Lamarque Souza <lamarque@gmail.com>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of
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

#include "manager.h"
#include "modemgsmsmsinterface.h"
#include "modemgsmsmsinterface_p.h"

#include <KDebug>

MMModemGsmSmsInterfacePrivate::MMModemGsmSmsInterfacePrivate(const QString &path, QObject *owner)
    : MMModemInterfacePrivate(path, owner), modemGsmSmsIface(MMModemManager::DBUS_SERVICE, path, QDBusConnection::systemBus())
{
}

MMModemGsmSmsInterface::MMModemGsmSmsInterface(const QString & path, MMModemManager * manager, QObject * parent)
    : MMModemInterface(*new MMModemGsmSmsInterfacePrivate(path, this), manager, parent)
{
    Q_D(const MMModemGsmSmsInterface);
    connect( &d->modemGsmSmsIface, SIGNAL(smsReceived(uint,bool)),
                this, SIGNAL(smsReceived(int,bool)));
    connect( &d->modemGsmSmsIface, SIGNAL(completed(uint,bool)),
                this, SIGNAL(completed(int,bool)));
}

MMModemGsmSmsInterface::~MMModemGsmSmsInterface()
{
}

void MMModemGsmSmsInterface::deleteSms(const int index)
{
    Q_D(MMModemGsmSmsInterface);
    d->modemGsmSmsIface.Delete(index);
}

QVariantMap MMModemGsmSmsInterface::get(const int index)
{
    Q_D(MMModemGsmSmsInterface);
    QDBusReply<QVariantMap> sms = d->modemGsmSmsIface.Get(index);

    if (sms.isValid()) {
        return sms.value();
    }

    return QVariantMap();
}

int MMModemGsmSmsInterface::getFormat()
{
    Q_D(MMModemGsmSmsInterface);
    QDBusReply<uint> format = d->modemGsmSmsIface.GetFormat();

    if (format.isValid()) {
        return format.value();
    }

    return 0;
}

void MMModemGsmSmsInterface::setFormat(const int format)
{
    Q_D(MMModemGsmSmsInterface);
    d->modemGsmSmsIface.SetFormat(format);
}

QString MMModemGsmSmsInterface::getSmsc()
{
    Q_D(MMModemGsmSmsInterface);
    QDBusReply<QString> smsc = d->modemGsmSmsIface.GetSmsc();

    if (smsc.isValid()) {
        return smsc.value();
    }

    return QString();
}

QList<QVariantMap> MMModemGsmSmsInterface::list()
{
    Q_D(MMModemGsmSmsInterface);
    QDBusReply<QList<QVariantMap> > sms = d->modemGsmSmsIface.List();

    if (sms.isValid()) {
        return sms.value();
    }

    return QList<QVariantMap>();
}

void MMModemGsmSmsInterface::save(const QVariantMap & properties)
{
    Q_D(MMModemGsmSmsInterface);
    d->modemGsmSmsIface.Save(properties);
}

void MMModemGsmSmsInterface::send(const QVariantMap & properties)
{
    Q_D(MMModemGsmSmsInterface);
    d->modemGsmSmsIface.Save(properties);
}

void MMModemGsmSmsInterface::sendFromStorage(const int index)
{
    Q_D(MMModemGsmSmsInterface);
    d->modemGsmSmsIface.SendFromStorage(index);
}

void MMModemGsmSmsInterface::setIndication(const int mode, const int mt, const int bm, const int ds, const int brf)
{
    Q_D(MMModemGsmSmsInterface);
    d->modemGsmSmsIface.SetIndication(mode, mt, bm, ds, brf);
}

#include "modemgsmsmsinterface.moc"

