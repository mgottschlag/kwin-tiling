/***************************************************************************
 *   manager.h                                                             *
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

#ifndef SYSTEMTRAYMANAGER_H
#define SYSTEMTRAYMANAGER_H

#include <QtCore/QObject>

namespace SystemTray
{
    class Notification;
    class Task;

/**
 * @short Creator and amalgamator of the supported system tray specifications
 **/
class Manager : public QObject
{
    Q_OBJECT

public:
    class Singleton;

    /**
     * @return the global Manager instance
     **/
    static Manager* self();

    /**
     * @return a list of all known Task instances
     **/
    QList<Task*> tasks() const;

    /**
     * @return a list of all known Task instances
     **/
    QList<Notification*> notifications() const;

signals:
    /**
     * Emitted when a new task has been added
     **/
    void taskAdded(SystemTray::Task *task);

    /**
     * Emitted when something about a task changes (such as it changing from
     * non-embeddable to embeddable)
     **/
    void taskChanged(SystemTray::Task *task);

    /**
     * Emitted when a task has been removed
     **/
    void taskRemoved(SystemTray::Task *task);

    /**
     * Emitted when a new notification has been added
     **/
    void notificationAdded(SystemTray::Notification *notification);

    /**
     * Emitted when something about a notification changes
     **/
    void notificationChanged(SystemTray::Notification *notification);

    /**
     * Emitted when a notification has been removed
     **/
    void notificationRemoved(SystemTray::Notification *notification);

private slots:
    void addTask(SystemTray::Task *task);
    void removeTask(SystemTray::Task *task);
    void addNotification(SystemTray::Notification *notification);
    void removeNotification(SystemTray::Notification *notification);

private:
    Manager();
    ~Manager();

    class Private;
    Private* const d;
};

}


#endif
