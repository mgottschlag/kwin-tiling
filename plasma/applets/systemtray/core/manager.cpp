/***************************************************************************
 *   manager.cpp                                                           *
 *                                                                         *
 *   Copyright (C) 2008 Jason Stubbs <jasonbstubbs@gmail.com>              *
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

#include "manager.h"

#include <KGlobal>

#include <plasma/applet.h>

#include "extendertask.h"
#include "job.h"
#include "notification.h"
#include "protocol.h"
#include "task.h"

#include "../protocols/notifications/dbusnotificationprotocol.h"
#include "../protocols/fdo/fdoprotocol.h"
#include "../protocols/plasmoid/plasmoidtaskprotocol.h"
#include "../protocols/jobs/dbusjobprotocol.h"
#include "../protocols/dbussystemtray/dbussystemtrayprotocol.h"

namespace SystemTray
{


class Manager::Private
{
public:
    Private(Manager *manager)
        : q(manager),
          extenderTask(0),
          jobTotals(new Job(manager)),
          jobProtocol(0),
          notificationProtocol(0),
          plasmoidProtocol(0)
    {
    }

    void setupProtocol(Protocol *protocol);

    Manager *q;
    Task *extenderTask;
    QList<Task *> tasks;
    QList<Notification*> notifications;
    QList<Job *> jobs;
    Job *jobTotals;
    Protocol *jobProtocol;
    Protocol *notificationProtocol;
    PlasmoidProtocol *plasmoidProtocol;
};


Manager::Manager()
    : d(new Private(this))
{
    d->plasmoidProtocol = new PlasmoidProtocol(this);
    d->setupProtocol(d->plasmoidProtocol);
    d->setupProtocol(new SystemTray::FdoProtocol(this));
    d->setupProtocol(new SystemTray::DBusSystemTrayProtocol(this));
}

Manager::~Manager()
{
    delete d;
}


Task* Manager::extenderTask(bool createIfNecessary) const
{
    if (!d->extenderTask && createIfNecessary) {
        d->extenderTask = new ExtenderTask(this);
        connect(d->extenderTask, SIGNAL(destroyed(SystemTray::Task*)), this, SLOT(removeTask(SystemTray::Task*)));
        connect(d->extenderTask, SIGNAL(changed(SystemTray::Task*)), this, SIGNAL(taskChanged(SystemTray::Task*)));
    }

    return d->extenderTask;
}

QList<Task*> Manager::tasks() const
{
    return d->tasks;
}

void Manager::addTask(Task *task)
{
    connect(task, SIGNAL(destroyed(SystemTray::Task*)), this, SLOT(removeTask(SystemTray::Task*)));
    connect(task, SIGNAL(changed(SystemTray::Task*)), this, SIGNAL(taskChanged(SystemTray::Task*)));

    kDebug() << task->name() << "(" << task->typeId() << ")";

    d->tasks.append(task);
    emit taskAdded(task);
}


void Manager::removeTask(Task *task)
{
    if (task == d->extenderTask) {
        d->extenderTask = 0;
    }

    d->tasks.removeAll(task);
    emit taskRemoved(task);
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
    connect(notification, SIGNAL(destroyed(SystemTray::Notification*)),
            this, SLOT(removeNotification(SystemTray::Notification*)));
    connect(notification, SIGNAL(changed(SystemTray::Notification*)),
            this, SIGNAL(notificationChanged(SystemTray::Notification*)));

    d->notifications.append(notification);
    emit notificationAdded(notification);
}

void Manager::removeNotification(Notification *notification)
{
    d->notifications.removeAll(notification);
    emit notificationRemoved(notification);
}

QList<Notification*> Manager::notifications() const
{
    return d->notifications;
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

void Manager::Private::setupProtocol(Protocol *protocol)
{
    connect(protocol, SIGNAL(jobCreated(SystemTray::Job*)), q, SLOT(addJob(SystemTray::Job*)));
    connect(protocol, SIGNAL(taskCreated(SystemTray::Task*)), q, SLOT(addTask(SystemTray::Task*)));
    connect(protocol, SIGNAL(notificationCreated(SystemTray::Notification*)),
            q, SLOT(addNotification(SystemTray::Notification*)));
    protocol->init();
}

void Manager::addJob(Job *job)
{
    connect(job, SIGNAL(destroyed(SystemTray::Job*)), this, SLOT(removeJob(SystemTray::Job*)));
    connect(job, SIGNAL(changed(SystemTray::Job*)), this, SIGNAL(jobChanged(SystemTray::Job*)));
    connect(job, SIGNAL(changed(SystemTray::Job*)), this, SLOT(updateTotals()));
    connect(job, SIGNAL(destroyed(SystemTray::Job*)), this, SLOT(updateTotals()));

    d->jobs.append(job);
    emit jobAdded(job);
}

void Manager::removeJob(Job *job)
{
    d->jobs.removeAll(job);
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

void Manager::forwardConstraintsEvent(Plasma::Constraints constraints)
{
    d->plasmoidProtocol->forwardConstraintsEvent(constraints);
}

}


#include "manager.moc"
