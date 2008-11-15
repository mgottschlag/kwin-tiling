/*
 * Copyright (C) 2008 Dmitry Suzdalev <dimsuz@gmail.com>
 *
 * This program is free software you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/


#ifndef NOTIFICATIONSENGINE_H
#define NOTIFICATIONSENGINE_H

#include <Plasma/DataEngine>

class QSignalMapper;
class QTimer;

/**
 *  Engine which provides data sources for notifications.
 *  Each notification is represented by one source.
 */
class NotificationsEngine : public Plasma::DataEngine
{
    Q_OBJECT

public:
    NotificationsEngine( QObject* parent, const QVariantList& args );
    ~NotificationsEngine();

    virtual void init();

    /**
     *  This function implements part of Notifications DBus interface.
     *  Once called, will add notification source to the engine
     */
    uint Notify(const QString &app_name, uint replaces_id, const QString &event_id, const QString &app_icon, const QString &summary, const QString &body, const QStringList &actions, const QVariantMap &hints, int timeout);

    void CloseNotification( uint id );

    Plasma::Service* serviceForSource(const QString& source);

signals:
    void NotificationClosed( uint id, uint reason );
    void ActionInvoked( uint id, const QString& actionKey );

private slots:
    void onNotificationTimedOut(int id);

private:
    /**
     * Holds the id that will be assigned to the next notification source
     * that will be created
     */
    uint m_nextId;

    /**
     * Used to map timers' signals to notification id's
     */
    QSignalMapper* m_sigMapper;

    /**
     * Timers for timing out notifications
     */
    QHash<uint, QTimer*> m_timers;

    friend class NotificationAction;

};

#endif
