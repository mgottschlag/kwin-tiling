/*
 * Copyright 2008 Alain Boyer <alainboyer@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License version 2 as
 * published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "tasksource.h"

// own
#include "tasksengine.h"
#include "taskservice.h"

TaskSource::TaskSource(StartupPtr startup, QObject *parent) :
    Plasma::DataContainer(parent),
    m_startup(startup),
    m_task(),
    m_isTask(false)
{
    setObjectName(TasksEngine::getStartupName(m_startup));
    setData("startup", true);
    setData("task", false);
    updateStartup(TaskManager::TaskUnchanged);
}

TaskSource::TaskSource(TaskPtr task, QObject *parent) :
    Plasma::DataContainer(parent),
    m_startup(),
    m_task(task),
    m_isTask(true)
{
    setObjectName(TasksEngine::getTaskName(m_task));
    setData("startup", false);
    setData("task", true);
    updateTask(TaskManager::EverythingChanged);
}

TaskSource::~TaskSource()
{
}

Plasma::Service *TaskSource::createService()
{
    return new TaskService(this);
}

TaskPtr TaskSource::getTask()
{
    return m_task;
}

bool TaskSource::isTask()
{
    return m_isTask;
}

void TaskSource::updateStartup(::TaskManager::TaskChanges startupChanges)
{
    switch (startupChanges) {
        case TaskManager::TaskUnchanged:
            setData("text", m_startup->text());
            setData("bin", m_startup->bin());
            setData("icon", m_startup->icon());
    }
    checkForUpdate();
}

void TaskSource::updateTask(::TaskManager::TaskChanges taskChanges)
{
    // only a subset of task information is exported
    switch (taskChanges) {
        case TaskManager::EverythingChanged:
            setData("name", m_task->name());
            setData("visibleName", m_task->visibleName());
            setData("visibleNameWithState", m_task->visibleNameWithState());
            setData("maximized", m_task->isMaximized());
            setData("minimized", m_task->isMinimized());
            setData("shaded", m_task->isShaded());
            setData("fullScreen", m_task->isFullScreen());
            setData("alwaysOnTop", m_task->isAlwaysOnTop());
            setData("keptBelowOthers", m_task->isKeptBelowOthers());
            setData("active", m_task->isActive());
            setData("onTop", m_task->isOnTop());
            setData("onCurrentDesktop", m_task->isOnCurrentDesktop());
            setData("onAllDesktops", m_task->isOnAllDesktops());
            setData("desktop", m_task->desktop());
            break;
        case TaskManager::NameChanged:
            setData("name", m_task->name());
            setData("visibleName", m_task->visibleName());
            setData("visibleNameWithState", m_task->visibleNameWithState());
            break;
        case TaskManager::StateChanged:
            setData("maximized", m_task->isMaximized());
            setData("minimized", m_task->isMinimized());
            setData("shaded", m_task->isShaded());
            setData("fullScreen", m_task->isFullScreen());
            setData("alwaysOnTop", m_task->isAlwaysOnTop());
            setData("keptBelowOthers", m_task->isKeptBelowOthers());
            setData("active", m_task->isActive());
            setData("onTop", m_task->isOnTop());
            break;
        case TaskManager::DesktopChanged:
            setData("onCurrentDesktop", m_task->isOnCurrentDesktop());
            setData("onAllDesktops", m_task->isOnAllDesktops());
            setData("desktop", m_task->desktop());
            break;
        default:
            break;
    }
    checkForUpdate();
}

#include "tasksource.moc"
