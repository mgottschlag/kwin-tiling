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

#ifndef TASKSOURCE_H
#define TASKSOURCE_H

// plasma
#include <Plasma/DataContainer>

// libtaskmanager
#include <taskmanager/taskmanager.h>

/**
 * Task Source
 *
 * This custom DataContainer represents a task or startup task as a unique source.
 * It holds a shared pointer to the task or startup task and is responsible for setting
 * and updating the data exposed by the source.
 */
class TaskSource : public Plasma::DataContainer
{

    Q_OBJECT

    public:
        TaskSource(::TaskManager::Startup *startup, QObject *parent);
        TaskSource(::TaskManager::Task *task, QObject *parent);
        ~TaskSource();

        Plasma::Service *createService();
        ::TaskManager::Task *task();

    private slots:
        void updateStartup(::TaskManager::TaskChanges startupChanges);
        void updateTask(::TaskManager::TaskChanges taskChanges);
        void updateDesktop();
        void updateActivity();

    private:
        QWeakPointer< ::TaskManager::Startup > m_startup;
        QWeakPointer< ::TaskManager::Task > m_task;
};

#endif // TASKSOURCE_H
