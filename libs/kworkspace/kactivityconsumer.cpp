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
#include "kactivitymanager_p.h"

#include <KDebug>

KActivityConsumer::KActivityConsumer(QObject * parent)
    : QObject(parent), d(new KActivityConsumerPrivate())
{
    connect(
        KActivityManager::self(), SIGNAL(CurrentActivityChanged(QString)),
        this,       SIGNAL(currentActivityChanged(QString))
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
    QDBusReply < TYPE > dbusReply = METHOD;                   \
    if (dbusReply.isValid()) {                                \
        return dbusReply.value();                             \
    } else {                                                  \
        kDebug() << "d-bus reply was invalid"                 \
                 << dbusReply.value()                         \
                 << dbusReply.error();                        \
        return DEFAULT;                                       \
    }

QString KActivityConsumer::currentActivity() const
{
    KACTIVITYCONSUMER_DBUS_RETURN(
        QString, KActivityManager::self()->CurrentActivity(), QString() );
}

QStringList KActivityConsumer::listActivities(KActivityInfo::State state) const
{
    KACTIVITYCONSUMER_DBUS_RETURN(
        QStringList, KActivityManager::self()->ListActivities(state), QStringList() );
}

QStringList KActivityConsumer::listActivities() const
{
    KACTIVITYCONSUMER_DBUS_RETURN(
        QStringList, KActivityManager::self()->ListActivities(), QStringList() );
}

QStringList KActivityConsumer::activitiesForResource(const KUrl & uri)
{
    KACTIVITYCONSUMER_DBUS_RETURN(
        QStringList, KActivityManager::self()->ActivitiesForResource(uri.url()), QStringList() );
}

#undef KACTIVITYCONSUMER_DBUS_RETURN

void KActivityConsumer::resourceAccessed(const KUrl & uri)
{
    KActivityManager::self()->NotifyResourceAccessed(
        QCoreApplication::instance()->applicationName(),
        uri.url()
    );
}

void KActivityConsumer::resourceAccessed(WId wid, const KUrl & uri, ResourceAction action)
{
    switch (action) {
        case Opened:
            KActivityManager::self()->NotifyResourceOpened(
                    QCoreApplication::instance()->applicationName(),
                    (uint)wid,
                    uri.url()
                );
            break;

        case Modified:
            KActivityManager::self()->NotifyResourceModified(
                    (uint)wid,
                    uri.url()
                );
            break;

        case Closed:
            KActivityManager::self()->NotifyResourceClosed(
                    (uint)wid,
                    uri.url()
                );
            break;
    }
}

KActivityConsumer::ServiceStatus KActivityConsumer::serviceStatus()
{
    if (!KActivityManager::isActivityServiceRunning()) {
        return NotRunning;
    }

    if (!KActivityManager::self()->IsBackstoreAvailable()) {
        return BareFunctionality;
    }

    return FullFunctionality;
}

