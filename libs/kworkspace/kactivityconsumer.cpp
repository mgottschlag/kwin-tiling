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
#include "activitymanager_interface.h"
#include "nepomukactivitiesservice_interface.h"

class KActivityConsumer::Private {
private:
    static org::kde::ActivityManager * managerService;

public:
    static org::kde::ActivityManager * manager()
    {
        if (!managerService) {
            managerService = new org::kde::ActivityManager(
                "org.kde.ActivityManager",
                "/ActivityManager",
                QDBusConnection::sessionBus()
            );
        }

        return managerService;
    }
};

org::kde::ActivityManager * KActivityConsumer::Private::managerService = 0;

KActivityConsumer::KActivityConsumer(QObject * parent)
    : QObject(parent), d(new Private())
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

QString KActivityConsumer::currentActivity() const
{
    return d->manager()->CurrentActivity();
}

QStringList KActivityConsumer::availableActivities() const
{
    return d->manager()->AvailableActivities();
}

void KActivityConsumer::registerResourceWindow(WId wid, const KUrl & uri)
{
    d->manager()->RegisterResourceWindow((uint)wid, uri.url());
}

void KActivityConsumer::unregisterResourceWindow(WId wid, const KUrl & uri)
{
    d->manager()->UnregisterResourceWindow((uint)wid, uri.url());
}

QStringList KActivityConsumer::activitiesForResource(const KUrl & uri)
{
    return d->manager()->ActivitiesForResource(uri.url());
}

