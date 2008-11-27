/*
 *   Copyright (C) 2007 Robert Knight <robertknight@gmail.com> 
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

// libtaskmanager
#include <taskmanager/taskmanager.h>

using TaskManager::StartupPtr;
using TaskManager::TaskPtr;

/**
 * Tasks Data Engine
 *
 * This engine provides information regarding tasks (windows that are currently open)
 * as well as startup tasks (windows that are about to open).
 * Each task and startup is represented by a unique source. Sources are added and removed
 * as windows are opened and closed. You cannot request a customized source.
 */
class TasksEngine : public Plasma::DataEngine
{

    Q_OBJECT

    public:
        TasksEngine(QObject *parent, const QVariantList &args);

    protected:
        virtual void init();

    private slots:
        void startupChanged();
        void startupAdded(StartupPtr startup);
        void startupRemoved(StartupPtr startup);
        void taskChanged();
        void taskAdded(TaskPtr task);
        void taskRemoved(TaskPtr task);

    private:
        void setDataForStartup(StartupPtr startup);
        void setDataForTask(TaskPtr task);
};

#endif // TASKSENGINE_H
