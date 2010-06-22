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

#include "kactivityconsumer.h"
#include "kactivityconsumer_p.h"
#include "activitymanager_interface.h"
#include "nepomukactivitiesservice_interface.h"

#include <KDebug>

org::kde::ActivityManager * KActivityConsumerPrivate::managerService = 0;

KActivityConsumer::KActivityConsumer(QObject * parent)
    : QObject(parent), d(new KActivityConsumerPrivate())
{
    connect(
        d->manager(), SIGNAL(CurrentActivityChanged(const QString &)),
        this,       SIGNAL(currentActivityChanged(const QString &))
    );
}

KActivityConsumer::~KActivityConsumer()
{
    delete d;
}

// macro defines a shorthand for validating and returning a d-bus result
// @param TYPE type of the result
// @param METHOD invocation of the d-bus method
// @param DEFAULT value to be used if the reply was not valid
#define KACTIVITYCONSUMER_DBUS_RETURN(TYPE, METHOD, DEFAULT)  \
    QDBusReply < TYPE > dbusReply = METHOD;          \
    if (dbusReply.isValid()) {                       \
        return dbusReply.value();                    \
    } else {                                                  \
        kDebug() << "d-bus reply was invalid"                 \
                 << dbusReply.value()                \
                 << dbusReply.error();               \
        return DEFAULT;                                       \
    }

QString KActivityConsumer::currentActivity() const
{
    KACTIVITYCONSUMER_DBUS_RETURN(
        QString, d->manager()->CurrentActivity(), QString() );
}

QStringList KActivityConsumer::availableActivities() const
{
    KACTIVITYCONSUMER_DBUS_RETURN(
        QStringList, d->manager()->AvailableActivities(), QStringList() );
}

QStringList KActivityConsumer::activitiesForResource(const KUrl & uri)
{
    KACTIVITYCONSUMER_DBUS_RETURN(
        QStringList, d->manager()->ActivitiesForResource(uri.url()), QStringList() );
}

#undef KACTIVITYCONSUMER_DBUS_RETURN

void KActivityConsumer::registerResourceWindow(WId wid, const KUrl & uri)
{
    d->manager()->RegisterResourceWindow((uint)wid, uri.url());
}

void KActivityConsumer::unregisterResourceWindow(WId wid, const KUrl & uri)
{
    d->manager()->UnregisterResourceWindow((uint)wid, uri.url());
}

