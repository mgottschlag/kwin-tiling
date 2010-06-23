/***************************************************************************
 *   notificationscroller.h                                                *
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

#ifndef NOTIFICATIONSCROLLER_H
#define NOTIFICATIONSCROLLER_H

#include <QGraphicsWidget>

#include <plasma/plasma.h>

class QGraphicsLinearLayout;

namespace Plasma
{
    class ScrollWidget;
    class TabBar;
}

class NotificationWidget;


class Notification;

class NotificationScroller : public QGraphicsWidget
{
    Q_OBJECT

public:
    NotificationScroller(QGraphicsItem *parent = 0);
    ~NotificationScroller();

    void addNotification(Notification *notification);

    void filterNotificationsByOwner(const QString &owner);

    Plasma::Location location() const;
    void setLocation(const Plasma::Location location);

public Q_SLOTS:
    void removeNotification(Notification *notification);

protected Q_SLOTS:
    void tabSwitched(int index);
    void adjustSize();

Q_SIGNALS:
    void scrollerEmpty();

private:
    QGraphicsWidget *m_mainWidget;
    QGraphicsLinearLayout *m_mainWidgetLayout;
    Plasma::ScrollWidget *m_scroll;
    Plasma::TabBar *m_notificationBar;

    //housekeeping data structures
    QList<Notification *>m_notifications;
    QHash<QString, QSet<Notification *> >m_notificationsForApp;
    QHash<Notification *, NotificationWidget *>m_notificationWidgets;
    QString m_currentFilter;
    Plasma::Location m_location;
    QGraphicsLinearLayout *m_tabsLayout;
    QGraphicsLinearLayout *m_layout;
};


#endif
