/***************************************************************************
 *   manager.h                                                             *
 *                                                                         *
 *   Copyright (C) 2008 Jason Stubbs <jasonbstubbs@gmail.com>              *
 *   Copyright (C) 2010 Marco Martin <notmart@gmail.com>                   *
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

#ifndef NOTIFICATIONSMANAGER_H
#define NOTIFICATIONSMANAGER_H

#include <QtCore/QObject>

#include <KConfigGroup>

#include <plasma/plasma.h>
#include "../ui/notifications.h"


namespace Plasma
{
class Applet;
}

class Notifications;
class Notification;
class Task;
class Job;

/**
 * w
 * @short Creator and amalgamator of the supported system tray specifications
 **/
class Manager : public QObject
{
    Q_OBJECT

public:
    Manager(Notifications *parentApplet);
    ~Manager();

    /**
     * @return a list of all known Notification instances
     **/
    QList<Notification*> notifications() const;

    /**
     * clear all notifications
     */
    void clearNotifications();

    /**
     * @return a list of all known Job instances
     **/
    QList<Job*> jobs() const;

    /**
     * @return a Job instance that can be used to monitor total progress
     **/
    Job *jobTotals() const;

    /**
     * Integrates the Job progress info into the applet's notification system
     **/
    void registerJobProtocol();

    /**
     * Iintegrates the notifications into the applet's notification system
     **/
    void registerNotificationProtocol();

      /**
     * Removes the Job progress info from the applet's notification system
     **/
    void unregisterJobProtocol();

    /**
     * Removes the notifications from the applet's notification system
     **/
    void unregisterNotificationProtocol();

    Notifications *applet() const;

signals:
    /**
     * Emitted when a new notification has been added
     **/
    void notificationAdded(Notification *notification);

    /**
     * Emitted when something about a notification changes
     **/
    void notificationChanged(Notification *notification);

    /**
     * The notification is expired and wants to hide itself
     */
    void notificationExpired(Notification *notification);

    /**
     * Emitted when a notification has been removed
     **/
    void notificationRemoved(Notification *notification);

    /**
     * Emitted when a new job has been added
     **/
    void jobAdded(Job *job);

    /**
     * Emitted when the state of a job changes
     **/
    void jobStateChanged(Job *job);

    /**
     * Emitted when something about a job changes
     **/
    void jobChanged(Job *job);

    /**
     * Emitted when a job has been removed
     **/
    void jobRemoved(Job *job);

    /**
     * the pc is out of idle and is starting being used
     */
    void idleTerminated();

private slots:
    void addNotification(Notification *notification);
    void removeNotification(Notification *notification);
    void addJob(Job *job);
    void removeJob(Job *job);
    void updateTotals();
    void checkIdle();

private:
    class Private;
    Private* const d;

    friend class Notifications;
};



#endif
