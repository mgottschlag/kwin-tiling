/***************************************************************************
 *   manager.cpp                                                           *
 *                                                                         *
 *   Copyright (C) 2008 Jason Stubbs <jasonbstubbs@gmail.com>              *
 *   Copyright (C) 2010 Marco Martin <notmart@gmail.com>                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "notificationsmanager.h"

#include <QTimer>

#include <KGlobal>

#include <plasma/applet.h>

#include "job.h"
#include "notification.h"
#include "protocol.h"

#include "../protocols/notifications/dbusnotificationprotocol.h"
#include "../protocols/jobs/dbusjobprotocol.h"

namespace SystemTray
{

static const int idleCheckInterval = 60 * 1000;

class Manager::Private
{
public:
    Private(Manager *manager)
        : q(manager),
          jobTotals(new Job(manager)),
          jobProtocol(0),
          notificationProtocol(0)
    {
    }

    void setupProtocol(Protocol *protocol);
    void checkIdle();

    Manager *q;
    QList<Task *> tasks;
    QList<Notification*> notifications;
    QList<Job *> jobs;
    Job *jobTotals;
    Protocol *jobProtocol;
    Protocol *notificationProtocol;
    QTimer *idleTimer;
};


Manager::Manager()
    : d(new Private(this))
{
    d->idleTimer = new QTimer(this);
    d->idleTimer->setSingleShot(false);
    connect(d->idleTimer, SIGNAL(timeout()), this, SLOT(checkIdle()));
}

Manager::~Manager()
{
    delete d;
}

void Manager::registerNotificationProtocol()
{
    if (!d->notificationProtocol) {
        d->notificationProtocol = new DBusNotificationProtocol(this);
        d->setupProtocol(d->notificationProtocol);
    }
}

void Manager::unregisterNotificationProtocol()
{
    if (d->notificationProtocol) {
        delete d->notificationProtocol;
        d->notificationProtocol = 0;
    }
}

void Manager::addNotification(Notification* notification)
{
    connect(notification, SIGNAL(notificationDestroyed(SystemTray::Notification*)),
            this, SLOT(removeNotification(SystemTray::Notification*)));
    connect(notification, SIGNAL(changed(SystemTray::Notification*)),
            this, SIGNAL(notificationChanged(SystemTray::Notification*)));
    connect(notification, SIGNAL(expired(SystemTray::Notification*)),
            this, SIGNAL(notificationExpired(SystemTray::Notification*)));

    d->notifications.append(notification);

    if (!d->idleTimer->isActive()) {
        d->idleTimer->start(idleCheckInterval);
    }
    connect(this, SIGNAL(idleTerminated()), notification, SLOT(startDeletionCountdown()));

    emit notificationAdded(notification);
}

void Manager::removeNotification(Notification *notification)
{
    d->notifications.removeAll(notification);
    disconnect(notification, 0, this, 0);
    disconnect(this, 0, notification, 0);

    if (d->notifications.isEmpty()) {
        d->idleTimer->stop();
    }

    emit notificationRemoved(notification);
}

QList<Notification*> Manager::notifications() const
{
    return d->notifications;
}

void Manager::clearNotifications()
{
    qDeleteAll(d->notifications);
    d->notifications.clear();
}

void Manager::registerJobProtocol()
{
    if (!d->jobProtocol) {
        d->jobProtocol = new DBusJobProtocol(this);
        d->setupProtocol(d->jobProtocol);
    }
}

void Manager::unregisterJobProtocol()
{
    if (d->jobProtocol) {
        delete d->jobProtocol;
        d->jobProtocol = 0;
    }
}

void Manager::addJob(Job *job)
{
    connect(job, SIGNAL(destroyed(SystemTray::Job*)), this, SLOT(removeJob(SystemTray::Job*)));
    connect(job, SIGNAL(changed(SystemTray::Job*)), this, SIGNAL(jobChanged(SystemTray::Job*)));
    connect(job, SIGNAL(stateChanged(SystemTray::Job*)), this, SIGNAL(jobStateChanged(SystemTray::Job*)));
    connect(job, SIGNAL(changed(SystemTray::Job*)), this, SLOT(updateTotals()));
    connect(job, SIGNAL(destroyed(SystemTray::Job*)), this, SLOT(updateTotals()));

    d->jobs.append(job);
    emit jobAdded(job);
}

void Manager::removeJob(Job *job)
{
    d->jobs.removeAll(job);
    disconnect(job, 0, this, 0);
    emit jobRemoved(job);
}

void Manager::updateTotals()
{
    uint totalPercent = 0;
    ulong totalEta = 0;
    foreach (Job *job, d->jobs) {
        totalPercent += job->percentage();
        if (job->eta() > totalEta) {
            totalEta = job->eta();
        }
    }

    if (d->jobs.count() > 0) {
        d->jobTotals->setPercentage(totalPercent / d->jobs.count());
        d->jobTotals->setMessage(i18np("1 running job (%2 remaining)", "%1 running jobs (%2 remaining)",
                                 d->jobs.count(),
                                 KGlobal::locale()->prettyFormatDuration(totalEta)));
    } else {
        d->jobTotals->setPercentage(0);
        d->jobTotals->setMessage(i18n("no running jobs"));
    }
    //TODO: set a sensible icon
}

Job *Manager::jobTotals() const
{
    return d->jobTotals;
}

QList<Job*> Manager::jobs() const
{
    return d->jobs;
}

void Manager::checkIdle()
{
    int totalIdle;
#ifdef HAVE_LIBXSS      // Idle detection.
    XScreenSaverInfo*  _mit_info;
    _mit_info = XScreenSaverAllocInfo();
    XScreenSaverQueryInfo( QX11Info::display(), QX11Info::appRootWindow(), _mit_info );
    totalIdle =  _mit_info->idle;
    XFree( _mit_info );
#else
    totalIdle = 0;
#endif // HAVE_LIBXSS

    if (totalIdle < idleCheckInterval) {
        d->idleTimer->stop();
        emit idleTerminated();
    }
}

void Manager::Private::setupProtocol(Protocol *protocol)
{
    connect(protocol, SIGNAL(jobCreated(SystemTray::Job*)), q, SLOT(addJob(SystemTray::Job*)));
    connect(protocol, SIGNAL(notificationCreated(SystemTray::Notification*)),
            q, SLOT(addNotification(SystemTray::Notification*)));
    protocol->init();
}

}


#include "notificationsmanager.moc"
