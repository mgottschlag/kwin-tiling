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

#ifndef TASKSENGINE_H
#define TASKSENGINE_H

// plasma
#include <Plasma/DataEngine>
#include <Plasma/Service>

// libtaskmanager
#include <taskmanager/taskmanager.h>

namespace TaskManager
{
    class Startup;
    class Task;
} // namespace TaskManager

/**
 * Tasks Data Engine
 *
 * This engine provides information regarding tasks (windows that are currently open)
 * as well as startup tasks (windows that are about to open).
 * Each task and startup is represented by a unique source. Sources are added and removed
 * as windows are opened and closed. You cannot request a customized source.
 *
 * A service is also provided for each task. It exposes some operations that can be
 * performed on the windows (ex: maximize, minimize, activate).
 *
 * The data and operations are provided and handled by the taskmanager library.
 * It should be noted that only a subset of data and operations are exposed.
 */
class TasksEngine : public Plasma::DataEngine
{

    Q_OBJECT

    public:
        TasksEngine(QObject *parent, const QVariantList &args);
        ~TasksEngine();
        Plasma::Service *serviceForSource(const QString &name);

    protected:
        static const QString getStartupName(::TaskManager::Startup *startup);
        static const QString getTaskName(::TaskManager::Task *task);
        virtual void init();
        bool sourceRequestEvent(const QString &source);

    private slots:
        void startupAdded(::TaskManager::Startup *startup);
        void startupRemoved(::TaskManager::Startup *startup);
        void taskAdded(::TaskManager::Task *task);
        void taskRemoved(::TaskManager::Task *task);

    private:
        friend class TaskSource;
};

#endif // TASKSENGINE_H
