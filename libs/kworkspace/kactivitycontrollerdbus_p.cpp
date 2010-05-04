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

#include "kactivitycontrollerdbus_p.h"
#include "activitycontrolleradaptor.h"

#include <QCoreApplication>

KActivityControllerDbus::KActivityControllerDbus(KActivityController * parent,
        org::kde::ActivityManager * manager)
    : QObject(parent), q(parent),
    m_service(QString("org.kde.ActivityController-%1")
                      .arg(QCoreApplication::applicationPid())),
    m_dbus(QDBusConnection::connectToBus(QDBusConnection::SessionBus, m_service))
{
    new ActivityControllerAdaptor(this);
    m_dbus.registerService(m_service);
    m_dbus.registerObject("/ActivityController", this);

    manager->RegisterActivityController(m_service);
}

KActivityControllerDbus::~KActivityControllerDbus()
{
    m_dbus.unregisterService(m_service);
}


void KActivityControllerDbus::ActivityAdded(const QString & id)
{
    emit q->activityAdded(id);
}

void KActivityControllerDbus::ActivityRemoved(const QString & id)
{
    emit q->activityRemoved(id);
}

void KActivityControllerDbus::ResourceWindowRegistered(uint wid, const QString & uri)
{
    emit q->resourceWindowRegistered(wid, uri);
}

void KActivityControllerDbus::ResourceWindowUnregistered(uint wid, const QString & uri)
{
    emit q->resourceWindowUnregistered(wid, uri);
}

