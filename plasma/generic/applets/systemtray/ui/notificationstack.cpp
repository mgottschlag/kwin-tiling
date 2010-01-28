/***************************************************************************
 *   notificationstack.cpp                                                *
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

#include "notificationstack.h"
#include "../core/notification.h"
#include "notificationwidget.h"

#include <QGraphicsLinearLayout>
#include <QTimer>

#include <KDebug>


namespace SystemTray
{

NotificationStack::NotificationStack(QGraphicsItem *parent)
   : QGraphicsWidget(parent),
     m_size(3)
{
    m_mainLayout = new QGraphicsLinearLayout(Qt::Vertical, this);
    m_separatorTimer = new QTimer(this);
    m_separatorTimer->setSingleShot(true);
}

NotificationStack::~NotificationStack()
{
}

void NotificationStack::addNotification(Notification *notification)
{
    //artificially enlarge the  timeout of notifications to be able to see at least a second of each one
    if (m_separatorTimer->isActive()) {
        notification->setTimeout(notification->timeout() + 1000);
    }
    m_separatorTimer->start(1000);

    connect(notification, SIGNAL(notificationDestroyed(SystemTray::Notification *)), this, SLOT(removeNotification(SystemTray::Notification *)));
    connect(notification, SIGNAL(expired(SystemTray::Notification *)), this, SLOT(removeNotification(SystemTray::Notification *)));

    NotificationWidget *notificationWidget = new NotificationWidget(notification, this);
    notificationWidget->setAutoDelete(true);
    notificationWidget->installEventFilter(this);
    notificationWidget->setAcceptsHoverEvents(this);

    m_notificationWidgets[notification] = notificationWidget;
    m_notifications.append(notification);

    if (m_notifications.size() > 1) {
        notificationWidget->setCollapsed(true);
    } else {
        m_currentNotificationWidget = notificationWidget;
    }

    if (m_notifications.size() > m_size) {
        bool found = false;

        //try to kill the oldest notification of the same app
        foreach (Notification *notif, m_notifications) {
            if (notif->applicationName() == notification->applicationName()) {
                m_notificationWidgets[notif]->deleteLater();
                m_notificationWidgets.remove(notif);
                m_notifications.removeAll(notif);
                found = true;
                break;
            }
        }
        //or kill the oldest one
        if (!found) {
            Notification *notif = m_notifications.first();
            m_notificationWidgets[notif]->deleteLater();
            m_notificationWidgets.remove(notif);
            m_notifications.pop_front();
        }
    }

    m_mainLayout->insertItem(0, notificationWidget);
    m_mainLayout->activate();
    updateGeometry();
    resize(effectiveSizeHint(Qt::PreferredSize));
}

void NotificationStack::removeNotification(SystemTray::Notification *notification)
{
    m_mainLayout->removeItem(m_notificationWidgets.value(notification));
    m_notificationWidgets.remove(notification);
    m_notifications.removeAll(notification);

    if (m_notifications.count() == 0) {
        emit stackEmpty();
    }

    resize(sizeHint(Qt::MinimumSize, QSizeF()));
}

bool NotificationStack::eventFilter(QObject *watched, QEvent *event)
{
    NotificationWidget *nw = qobject_cast<NotificationWidget *>(watched);

    if (!nw) {
        return false;
    }

    if (event->type() == QEvent::GraphicsSceneHoverEnter) {
        if (m_currentNotificationWidget) {
            m_currentNotificationWidget.data()->setCollapsed(true);
        }
        nw->setCollapsed(false);
        m_currentNotificationWidget = nw;
    }

    return false;
}

}

#include "notificationstack.moc"
