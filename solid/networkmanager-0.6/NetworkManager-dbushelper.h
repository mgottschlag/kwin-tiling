/*  This file is part of the KDE project
    Copyright (C) 2007 Will Stephenson <wstephenson@kde.org>

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

#ifndef SOLID_NETWORKMANAGER_DBUSHELPER_H
#define SOLID_NETWORKMANAGER_DBUSHELPER_H

#include <QList>
#include <QVariant>
#include <solid/control/ifaces/authentication.h>
#include <solid/control/authentication.h>

/**
 * marshaller/unmarshaller for doing dbus operations with Authentication objects
 */
class KDE_EXPORT NMDBusHelper
{
public:
    static QList<QVariant> serialize(Solid::Control::Authentication *, const QString  & essid, QList<QVariant>  & args, bool * error);
private:
    static QList<QVariant> doSerialize(Solid::Control::AuthenticationNone *, const QString  & essid, QList<QVariant>  & args, bool * error);
    static QList<QVariant> doSerialize(Solid::Control::AuthenticationWep *, const QString  & essid, QList<QVariant>  & args, bool * error);
    static QList<QVariant> doSerialize(Solid::Control::AuthenticationWpaPersonal *, const QString  & essid, QList<QVariant>  & args, bool * error);
    static QList<QVariant> doSerialize(Solid::Control::AuthenticationWpaEnterprise *, const QString  & essid, QList<QVariant>  & args, bool * error);
};

#endif // SOLID_NETWORKMANAGER_DBUSHELPER_H
