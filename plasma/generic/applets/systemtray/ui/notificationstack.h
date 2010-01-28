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

#ifndef NOTIFICATIONSTACK_H
#define NOTIFICATIONSTACK_H

#include <QGraphicsWidget>

class QGraphicsLinearLayout;
class QTimer;

class NotificationWidget;

namespace SystemTray
{
class Notification;

class NotificationStack : public QGraphicsWidget
{
    Q_OBJECT

public:
    NotificationStack(QGraphicsItem *parent = 0);
    ~NotificationStack();

    void addNotification(SystemTray::Notification *notification);

    //TODO:accessor
    void setCurrentNotification(SystemTray::Notification *notification);

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    bool eventFilter(QObject *watched, QEvent *event);

public Q_SLOTS:
    void removeNotification(SystemTray::Notification *notification);
    void delayedRemoveNotification(SystemTray::Notification *notification);

private Q_SLOTS:
    void popNotification();

Q_SIGNALS:
    void stackEmpty();

private:
    QList<Notification *> m_notifications;
    QList<Notification *> m_notificationsToRemove;
    QHash<Notification *, NotificationWidget *> m_notificationWidgets;
    QGraphicsLinearLayout *m_mainLayout;
    int m_size;
    bool m_underMouse;
    QWeakPointer<NotificationWidget> m_currentNotificationWidget;
    QTimer *m_delayedRemoveTimer;
};

}

#endif
