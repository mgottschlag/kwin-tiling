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

TaskSource::TaskSource(::TaskManager::Startup *startup, QObject *parent)
    : Plasma::DataContainer(parent),
      m_startup(startup)
{
    setObjectName(TasksEngine::getStartupName(startup));
    setData("startup", true);
    setData("task", false);
    updateStartup(TaskManager::TaskUnchanged);
}

TaskSource::TaskSource(::TaskManager::Task *task, QObject *parent)
    : Plasma::DataContainer(parent),
      m_task(task)
{
    setObjectName(TasksEngine::getTaskName(task));
    setData("startup", false);
    setData("task", true);
    setData("className", task->className());
    setData("classClass", task->classClass());
    updateTask(TaskManager::EverythingChanged);
}

TaskSource::~TaskSource()
{
}

Plasma::Service *TaskSource::createService()
{
    return new TaskService(this);
}

::TaskManager::Task *TaskSource::task()
{
    return m_task.data();
}

void TaskSource::updateStartup(::TaskManager::TaskChanges startupChanges)
{
    ::TaskManager::Startup *startup = m_startup.data();
    if (!startup) {
        return;
    }

    switch (startupChanges) {
        case TaskManager::TaskUnchanged:
            setData("text", startup->text());
            setData("bin", startup->bin());
            setData("icon", startup->icon());
    }
    checkForUpdate();
}

void TaskSource::updateTask(::TaskManager::TaskChanges taskChanges)
{
    ::TaskManager::Task *taskPtr = m_task.data();
    if (!taskPtr) {
        return;
    }

    // only a subset of task information is exported
    switch (taskChanges) {
        case TaskManager::EverythingChanged:
            setData("name", taskPtr->name());
            setData("visibleName", taskPtr->visibleName());
            setData("visibleNameWithState", taskPtr->visibleNameWithState());
            setData("maximized", taskPtr->isMaximized());
            setData("minimized", taskPtr->isMinimized());
            setData("shaded", taskPtr->isShaded());
            setData("fullScreen", taskPtr->isFullScreen());
            setData("alwaysOnTop", taskPtr->isAlwaysOnTop());
            setData("keptBelowOthers", taskPtr->isKeptBelowOthers());
            setData("active", taskPtr->isActive());
            setData("onTop", taskPtr->isOnTop());
            setData("onCurrentDesktop", taskPtr->isOnCurrentDesktop());
            setData("onAllDesktops", taskPtr->isOnAllDesktops());
            setData("desktop", taskPtr->desktop());
            setData("onCurrentActivity", taskPtr->isOnCurrentActivity());
            setData("onAllActivities", taskPtr->isOnAllActivities());
            setData("activities", taskPtr->activities());
            setData("icon", taskPtr->icon());
            setData("actionMinimize", taskPtr->info().actionSupported(NET::ActionMinimize));
            setData("actionMaximize", taskPtr->info().actionSupported(NET::ActionMax));
            setData("actionShade", taskPtr->info().actionSupported(NET::ActionShade));
            setData("actionResize", taskPtr->info().actionSupported(NET::ActionResize));
            setData("actionMove", taskPtr->info().actionSupported(NET::ActionMove));
            setData("actionClose", taskPtr->info().actionSupported(NET::ActionClose));
            setData("actionChangeDesktop", taskPtr->info().actionSupported(NET::ActionChangeDesktop));
            setData("actionFullScreen", taskPtr->info().actionSupported(NET::ActionFullScreen));
            break;
        case TaskManager::IconChanged:
            setData("icon", taskPtr->icon());
            break;
        case TaskManager::NameChanged:
            setData("name", taskPtr->name());
            setData("visibleName", taskPtr->visibleName());
            setData("visibleNameWithState", taskPtr->visibleNameWithState());
            break;
        case TaskManager::StateChanged:
            setData("maximized", taskPtr->isMaximized());
            setData("minimized", taskPtr->isMinimized());
            setData("shaded", taskPtr->isShaded());
            setData("fullScreen", taskPtr->isFullScreen());
            setData("alwaysOnTop", taskPtr->isAlwaysOnTop());
            setData("keptBelowOthers", taskPtr->isKeptBelowOthers());
            setData("active", taskPtr->isActive());
            setData("onTop", taskPtr->isOnTop());
            setData("visibleNameWithState", taskPtr->visibleNameWithState());
            break;
        case TaskManager::DesktopChanged:
            setData("onCurrentDesktop", taskPtr->isOnCurrentDesktop());
            setData("onAllDesktops", taskPtr->isOnAllDesktops());
            setData("desktop", taskPtr->desktop());
            break;
        case TaskManager::ActivitiesChanged:
            setData("onCurrentActivity", taskPtr->isOnCurrentActivity());
            setData("onAllActivities", taskPtr->isOnAllActivities());
            setData("activities", taskPtr->activities());
        case TaskManager::ActionsChanged:
            setData("actionMinimize", taskPtr->info().actionSupported(NET::ActionMinimize));
            setData("actionMaximize", taskPtr->info().actionSupported(NET::ActionMax));
            setData("actionShade", taskPtr->info().actionSupported(NET::ActionShade));
            setData("actionResize", taskPtr->info().actionSupported(NET::ActionResize));
            setData("actionMove", taskPtr->info().actionSupported(NET::ActionMove));
            setData("actionClose", taskPtr->info().actionSupported(NET::ActionClose));
            setData("actionChangeDesktop", taskPtr->info().actionSupported(NET::ActionChangeDesktop));
            setData("actionFullScreen", taskPtr->info().actionSupported(NET::ActionFullScreen));
            break;
        default:
            break;
    }
    checkForUpdate();
}

void TaskSource::updateDesktop()
{
    if (!m_task) {
        return;
    }

    const bool onCurrentDesktop = m_task.data()->isOnCurrentDesktop();
    if (data()["onCurrentDesktop"].toBool() != onCurrentDesktop) {
        setData("onCurrentDesktop", onCurrentDesktop);
        checkForUpdate();
    }
}

void TaskSource::updateActivity()
{
    if (!m_task) {
        return;
    }

    const bool onCurrentActivity = m_task.data()->isOnCurrentActivity();
    if (data()["onCurrentActivity"].toBool() != onCurrentActivity) {
        setData("onCurrentActivity", onCurrentActivity);
        checkForUpdate();
    }
}

#include "tasksource.moc"
