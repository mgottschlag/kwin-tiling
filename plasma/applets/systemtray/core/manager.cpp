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

#include "notification.h"
#include "notificationprotocol.h"
#include "task.h"
#include "taskprotocol.h"

#include "../protocols/dbus/dbusnotificationprotocol.h"
#include "../protocols/fdo/fdonotificationprotocol.h"
#include "../protocols/fdo/fdotaskprotocol.h"
#include "../protocols/plasmoid/plasmoidtaskprotocol.h"

namespace SystemTray
{


class Manager::Singleton
{
public:
    Manager instance;
};

K_GLOBAL_STATIC(Manager::Singleton, singleton)


class Manager::Private
{
public:
    Private(Manager *manager)
        : q(manager)
    {
        registerTaskProtocol(new Plasmoid::TaskProtocol(q));
        registerTaskProtocol(new FDO::TaskProtocol(q));
        registerNotificationProtocol(new FDO::NotificationProtocol(q));
        registerNotificationProtocol(new DBus::NotificationProtocol(q));
    }

    void registerTaskProtocol(TaskProtocol *protocol);
    void registerNotificationProtocol(NotificationProtocol *protocol);

    Manager *q;
    QList<Task*> tasks;
    QList<Notification*> notifications;
};


Manager* Manager::self()
{
    return &singleton->instance;
}

Manager::Manager()
    : d(new Private(this))
{
}

Manager::~Manager()
{
    delete d;
}


QList<Task*> Manager::tasks() const
{
    return d->tasks;
}


void Manager::Private::registerTaskProtocol(TaskProtocol *protocol)
{
    connect(protocol, SIGNAL(taskCreated(SystemTray::Task*)),
            q, SLOT(addTask(SystemTray::Task*)));
    protocol->init();
}


void Manager::addTask(Task *task)
{
    connect(task, SIGNAL(destroyed(SystemTray::Task*)),
            this, SLOT(removeTask(SystemTray::Task*)));
    connect(task, SIGNAL(changed(SystemTray::Task*)),
            this, SIGNAL(taskChanged(SystemTray::Task*)));

    kDebug() << task->name() << "(" << task->typeId() << ")";

    d->tasks.append(task);
    emit taskAdded(task);
}


void Manager::removeTask(Task *task)
{
    d->tasks.removeAll(task);
    emit taskRemoved(task);
}


void Manager::Private::registerNotificationProtocol(NotificationProtocol *protocol)
{
    connect(protocol, SIGNAL(notificationCreated(SystemTray::Notification*)),
            q, SLOT(addNotification(SystemTray::Notification*)));
    protocol->init();
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

}


#include "manager.moc"
