/***************************************************************************
 *   notificationgroup.h                                                *
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

#ifndef NOTIFICATIONGROUP_H
#define NOTIFICATIONGROUP_H


#include <Plasma/Extender>
#include <Plasma/ExtenderGroup>


#include <Plasma/Plasma>

class QGraphicsLinearLayout;

namespace Plasma
{
    class ExtenderItem;
    class TabBar;
    class Extender;
}

class NotificationWidget;
class Notification;

class NotificationGroup : public Plasma::ExtenderGroup
{
    Q_OBJECT

public:
    NotificationGroup(Plasma::Extender *parent, uint groupId = 0);
    ~NotificationGroup();

    void addNotification(Notification *notification);

    void filterNotificationsByOwner(const QString &owner);


public Q_SLOTS:
    void removeNotification(Notification *notification);

protected Q_SLOTS:
    void tabSwitched(int index);
    void extenderItemDestroyed(Plasma::ExtenderItem *object);

Q_SIGNALS:
    void scrollerEmpty();

private:
    Plasma::TabBar *m_notificationBar;

    //housekeeping data structures
    QList<Notification *>m_notifications;

    //Those two are kept on both ways since we are not sure the thing
    //contained in the hash is still valid so we couldn't obtain the
    //info to remove the proper key, unless both ways are stored
    QHash<QString, QSet<Notification *> > m_notificationsForApp;
    QHash<Notification *, QString> m_appForNotification;

    QHash<Notification *, Plasma::ExtenderItem *>m_extenderItemsForNotification;
    QHash<Plasma::ExtenderItem *, Notification *>m_notificationForExtenderItems;

    QString m_currentFilter;
};


#endif
