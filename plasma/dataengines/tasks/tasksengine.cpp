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
    // if source does not exist, return null service
    if (!source) {
        return Plasma::DataEngine::serviceForSource(name);
    }
    // if source represents a startup task, return null service
    if (!source->isTask()) {
        return Plasma::DataEngine::serviceForSource(name);
    }
    // if source represent a proper task, return task service
    Plasma::Service *service = source->createService();
    service->setParent(this);
    return service;
}

const QString TasksEngine::getStartupName(StartupPtr startup)
{
    return startup->id().id();
}

const QString TasksEngine::getTaskName(TaskPtr task)
{
    return QString::number(task->window());
}

void TasksEngine::init()
{
    foreach (const TaskPtr &task, TaskManager::TaskManager::self()->tasks()) {
        Q_ASSERT(task);
        addTask(task);
    }
    connect(TaskManager::TaskManager::self(), SIGNAL(startupAdded(StartupPtr)), this, SLOT(startupAdded(StartupPtr)));
    connect(TaskManager::TaskManager::self(), SIGNAL(startupRemoved(StartupPtr)), this, SLOT(startupRemoved(StartupPtr)));
    connect(TaskManager::TaskManager::self(), SIGNAL(taskAdded(TaskPtr)), this, SLOT(taskAdded(TaskPtr)));
    connect(TaskManager::TaskManager::self(), SIGNAL(taskRemoved(TaskPtr)), this, SLOT(taskRemoved(TaskPtr)));
}

void TasksEngine::startupAdded(StartupPtr startup)
{
    Q_ASSERT(startup);
    addStartup(startup);
}

void TasksEngine::startupRemoved(StartupPtr startup)
{
    Q_ASSERT(startup);
    removeSource(getStartupName(startup));
}

void TasksEngine::taskAdded(TaskPtr task)
{
    Q_ASSERT(task);
    addTask(task);
}

void TasksEngine::taskRemoved(TaskPtr task)
{
    Q_ASSERT(task);
    removeSource(getTaskName(task));
}

void TasksEngine::addStartup(StartupPtr startup)
{
    TaskSource *taskSource = new TaskSource(startup, this);
    connect(startup.constData(), SIGNAL(changed(::TaskManager::TaskChanges)), taskSource, SLOT(updateStartup(::TaskManager::TaskChanges)));
    addSource(taskSource);
}

void TasksEngine::addTask(TaskPtr task)
{
    TaskSource *taskSource = new TaskSource(task, this);
    connect(task.constData(), SIGNAL(changed(::TaskManager::TaskChanges)), taskSource, SLOT(updateTask(::TaskManager::TaskChanges)));
    connect(TaskManager::TaskManager::self(), SIGNAL(desktopChanged(int)), taskSource, SLOT(updateDesktop(int)));
    addSource(taskSource);
}

K_EXPORT_PLASMA_DATAENGINE(tasks, TasksEngine)

#include "tasksengine.moc"
