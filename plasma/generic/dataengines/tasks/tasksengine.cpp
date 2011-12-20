/*
 * Copyright 2007 Robert Knight <robertknight@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "tasksengine.h"
#include "virtualdesktopssource.h"

// own
#include "tasksource.h"

TasksEngine::TasksEngine(QObject *parent, const QVariantList &args) :
    Plasma::DataEngine(parent, args)
{
    Q_UNUSED(args);
}

TasksEngine::~TasksEngine()
{
}

Plasma::Service *TasksEngine::serviceForSource(const QString &name)
{
    TaskSource *source = dynamic_cast<TaskSource*>(containerForSource(name));
    // if source does not exist or it represents a startup task, return a null service
    if (!source || !source->task()) {
        return Plasma::DataEngine::serviceForSource(name);
    }

    // if source represent a proper task, return task service
    Plasma::Service *service = source->createService();
    service->setParent(this);
    return service;
}

const QString TasksEngine::getStartupName(::TaskManager::Startup *startup)
{
    return startup->id().id();
}

const QString TasksEngine::getTaskName(::TaskManager::Task *task)
{
    return QString::number(task->window());
}

void TasksEngine::init()
{
    foreach (TaskManager::Task *task, TaskManager::TaskManager::self()->tasks()) {
        Q_ASSERT(task);
        taskAdded(task);
    }

    TaskManager::TaskManager *manager = TaskManager::TaskManager::self();
    connect(manager, SIGNAL(startupAdded(::TaskManager::Startup*)), this, SLOT(startupAdded(::TaskManager::Startup*)));
    connect(manager, SIGNAL(startupRemoved(::TaskManager::Startup*)), this, SLOT(startupRemoved(::TaskManager::Startup*)));
    connect(manager, SIGNAL(taskAdded(::TaskManager::Task*)), this, SLOT(taskAdded(::TaskManager::Task*)));
    connect(manager, SIGNAL(taskRemoved(::TaskManager::Task*)), this, SLOT(taskRemoved(::TaskManager::Task*)));
}

void TasksEngine::startupRemoved(::TaskManager::Startup *startup)
{
    Q_ASSERT(startup);
    // there is an event loop ref counting bug in Qt that prevents deleteLater() from working
    // properly, so we need to remove the source our selves with a single shot
    //removeSource(getStartupName(startup));
    if (Plasma::DataContainer *container = containerForSource(getStartupName(startup))) {
        QTimer::singleShot(0, container, SLOT(deleteLater()));
    }
}

void TasksEngine::taskRemoved(::TaskManager::Task *task)
{
    Q_ASSERT(task);
    // there is an event loop ref counting bug in Qt that prevents deleteLater() from working
    // properly, so we need to remove the source our selves with a single shot
    //removeSource(getTaskName(task));
    if (Plasma::DataContainer *container = containerForSource(getTaskName(task))) {
        QTimer::singleShot(0, container, SLOT(deleteLater()));
    }
}

void TasksEngine::startupAdded(::TaskManager::Startup *startup)
{
    Q_ASSERT(startup);
    if (!containerForSource(getStartupName(startup))) {
        TaskSource *taskSource = new TaskSource(startup, this);
        connect(startup, SIGNAL(changed(::TaskManager::TaskChanges)), taskSource, SLOT(updateStartup(::TaskManager::TaskChanges)));
        addSource(taskSource);
    }
}

void TasksEngine::taskAdded(::TaskManager::Task *task)
{
    Q_ASSERT(task);
    if (!containerForSource(getTaskName(task))) {
        TaskSource *taskSource = new TaskSource(task, this);
        connect(task, SIGNAL(changed(::TaskManager::TaskChanges)), taskSource, SLOT(updateTask(::TaskManager::TaskChanges)));
        connect(TaskManager::TaskManager::self(), SIGNAL(desktopChanged(int)), taskSource, SLOT(updateDesktop()));
        connect(TaskManager::TaskManager::self(), SIGNAL(activityChanged(QString)), taskSource, SLOT(updateActivity()));
        addSource(taskSource);
    }
}

bool TasksEngine::sourceRequestEvent(const QString &source)
{
    if (source == "virtualDesktops") {
        addSource(new VirtualDesktopsSource);
        return true;
    }

    return false;
}

K_EXPORT_PLASMA_DATAENGINE(tasks, TasksEngine)

#include "tasksengine.moc"
