/*
 * Copyright (c) 2010 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef ACTIVITY_CONTROLLER_DBUS_P_H
#define ACTIVITY_CONTROLLER_DBUS_P_H

#include <QObject>
#include <QDBusConnection>
#include "kactivitycontroller.h"
#include "activitymanager_interface.h"

class KActivityControllerDbus: public QObject {
    Q_OBJECT

public:
    explicit KActivityControllerDbus(org::kde::ActivityManager * manager, QObject *parent = 0);

    ~KActivityControllerDbus();

public Q_SLOTS:
    void ActivityAdded(const QString & id);
    void ActivityRemoved(const QString & id);

    void ResourceWindowRegistered(uint wid, const QString & uri);
    void ResourceWindowUnregistered(uint wid, const QString & uri);

Q_SIGNALS:
    void activityAdded(const QString & id);
    void activityRemoved(const QString & id);
    void resourceWindowRegistered(uint wid, const QString & uri);
    void resourceWindowUnregistered(uint wid, const QString & uri);

private:
    QString m_service;
    QDBusConnection m_dbus;
};

#endif // ACTIVITY_CONTROLLER_DBUS_P_H
