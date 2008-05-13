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

// Qt
#include <QObject>
#include <QString>
#include <QList>

// Plasma
#include <plasma/dataengine.h>
#include <taskmanager/taskmanager.h>

using TaskManager::StartupPtr;
using TaskManager::TaskPtr;

/**
 * This class evaluates the basic expressions given in the interface.
 */
class TasksEngine : public Plasma::DataEngine
{
    Q_OBJECT

    public:
		TasksEngine( QObject* parent, const QVariantList& args);

    protected:
        virtual void init();

    private slots:
        void taskChanged();
        void taskAdded(TaskPtr task);
        void taskRemoved(TaskPtr task);
        void startupChanged();
        void startupAdded(StartupPtr task);
        void startupRemoved(StartupPtr);

    private:
        void connectTask(TaskPtr task);
        void setDataForTask(TaskPtr task);
        void connectStartup(StartupPtr task);
        void setDataForStartup(StartupPtr task);
};

K_EXPORT_PLASMA_DATAENGINE(tasks, TasksEngine)

#endif
