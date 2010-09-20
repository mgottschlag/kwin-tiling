/*
 * Copyright 2010 Marco Martin <mart@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */

#ifndef NOTIFICATIONSEXTENDER_H
#define NOTIFICATIONSEXTENDER_H

#include <Plasma/Extender>

namespace Plasma {
    class Applet;
    class ExtenderItem;
    class TabBar;
}

class NotificationsExtender : public Plasma::Extender
{
    Q_OBJECT
public:
    NotificationsExtender(Plasma::Applet *applet = 0);
    ~NotificationsExtender();

protected:
    virtual void itemAddedEvent(Plasma::ExtenderItem *item, const QPointF &pos = QPointF(-1, -1));
    virtual void itemRemovedEvent(Plasma::ExtenderItem *item);

protected Q_SLOTS:
    void showNotifications();
    void showJobs();

private:
    int itemIndex(Plasma::ExtenderItem *item);

    Plasma::TabBar *m_tabBar;
    QWeakPointer<Plasma::ExtenderItem> m_jobs;
    QWeakPointer<Plasma::ExtenderItem> m_notifications;
};

#endif
