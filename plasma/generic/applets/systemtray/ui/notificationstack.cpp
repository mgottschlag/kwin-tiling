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

#include <KDebug>

#include <QGraphicsLinearLayout>

namespace SystemTray
{

NotificationStack::NotificationStack(QGraphicsItem *parent)
   : QGraphicsWidget(parent),
     m_size(3)
{
    m_mainLayout = new QGraphicsLinearLayout(Qt::Vertical, this);
}

NotificationStack::~NotificationStack()
{
}

void NotificationStack::addNotification(Notification *notification)
{
    connect(notification, SIGNAL(notificationDestroyed(SystemTray::Notification *)), this, SLOT(removeNotification(SystemTray::Notification *)));
    connect(notification, SIGNAL(expired(SystemTray::Notification *)), this, SLOT(removeNotification(SystemTray::Notification *)));

    NotificationWidget *notificationWidget = new NotificationWidget(notification, this);
    notificationWidget->setAutoDelete(true);

    m_notificationWidgets[notification] = notificationWidget;
    m_notifications.append(notification);

    if (m_notifications.size() > m_size) {
        Notification *notif = m_notifications.first();
        m_notificationWidgets[notif]->deleteLater();
        m_notificationWidgets.remove(notif);
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

}

#include "notificationstack.moc"
