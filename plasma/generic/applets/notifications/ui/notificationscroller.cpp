/***************************************************************************
 *   notificationscroller.cpp                                                *
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

#include "notificationscroller.h"
#include "../core/notification.h"
#include "notificationwidget.h"

#include <QGraphicsLinearLayout>
#include <QWidget>

#include <KDebug>
#include <KIcon>
#include <KLocale>

#include <Plasma/TabBar>
#include <Plasma/ExtenderItem>


NotificationScroller::NotificationScroller(Extender *parent, uint groupId)
   : Plasma::ExtenderGroup(parent, groupId)
{
    setTransient(true);
    setAutoCollapse(true);
    config().writeEntry("type", "notification");
    setName("notifications");
    setTitle(i18n("Notifications"));
    setIcon("dialog-information");
    showCloseButton();


    m_notificationBar = new Plasma::TabBar(this);
    m_notificationBar->nativeWidget()->setMaximumWidth(400);
    m_notificationBar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_notificationBar->addTab(KIcon("dialog-information"), i18nc("Show all  notifications", "All"));
    connect(m_notificationBar, SIGNAL(currentChanged(int)), this, SLOT(tabSwitched(int)));


    QGraphicsWidget *widget = new QGraphicsWidget(this);
    m_tabsLayout = new QGraphicsLinearLayout(Qt::Horizontal, widget);
    widget->setContentsMargins(0, 4, 0, 0);
    m_tabsLayout->setContentsMargins(0, 0, 0, 0);
    m_tabsLayout->addStretch();
    m_tabsLayout->addItem(m_notificationBar);
    m_tabsLayout->addStretch();

    setWidget(widget);
}

NotificationScroller::~NotificationScroller()
{
    qDeleteAll(m_notifications);
}


void NotificationScroller::addNotification(Notification *notification)
{
    connect(notification, SIGNAL(notificationDestroyed(Notification *)), this, SLOT(removeNotification(Notification *)));

    NotificationWidget *notificationWidget = new NotificationWidget(notification, this);
    notificationWidget->setTitleBarVisible(false);
    Plasma::ExtenderItem *extenderItem = new ExtenderItem(extender());
    extenderItem->setGroup(this);
    extenderItem->setTransient(true);
    extenderItem->config().writeEntry("type", "notification");
    extenderItem->setWidget(notificationWidget);
    extenderItem->setIcon(QIcon());
    extenderItem->showCloseButton();
    connect(extenderItem, SIGNAL(destroyed(QObject *)), this, SLOT(extenderItemDestroyed(QObject *)));
    connect(notificationWidget, SIGNAL(destroyed()), extenderItem, SLOT(deleteLater()));

    if (!notification->summary().isEmpty()) {
        extenderItem->setTitle(notification->summary());
    } else {
        extenderItem->setTitle(i18n("Notification from %1", notification->applicationName()));
    }

    m_extenderItemsForNotification[notification] = extenderItem;
    m_notificationForExtenderItems[extenderItem] = notification;
    m_notifications.append(notification);
    m_notificationsForApp[notification->applicationName()].insert(notification);

    if (!m_currentFilter.isNull() && m_currentFilter != notification->applicationName()) {
        notificationWidget->setMaximumHeight(0);
        notificationWidget->setVisible(false);
    }

    //adjust tabbar
    bool found = false;
    for (int i = 0; i < m_notificationBar->count(); ++i) {
        if (m_notificationBar->tabText(i) == notification->applicationName()) {
            found = true;
            break;
        }
    }

    if (!found) {
        m_notificationBar->addTab(notification->applicationIcon(), notification->applicationName());
    }

    if (items().count() == 1) {
        //ensure the notifications group is the last item
        Plasma::ExtenderGroup *jobGroup = extender()->group("jobGroup");
        if (jobGroup && jobGroup->isVisible()) {
            if (extender()->appearance() == Plasma::Extender::TopDownStacked) {
                setExtender(extender(), QPointF(0,0));
            } else {
                setExtender(extender(), jobGroup->geometry().bottomLeft());
            }
        }
    }

    notificationWidget->layout()->activate();
}

void NotificationScroller::extenderItemDestroyed(QObject *object)
{
    Notification *n = m_notificationForExtenderItems.value(static_cast<Plasma::ExtenderItem *>(object));

    if (n) {
        removeNotification(n);
        n->deleteLater();
        m_notificationForExtenderItems.remove(static_cast<Plasma::ExtenderItem *>(object));
    }
}

void NotificationScroller::removeNotification(Notification *notification)
{
    m_extenderItemsForNotification.remove(notification);
    m_notifications.removeAll(notification);
    if (m_notificationsForApp.contains(notification->applicationName())) {
        m_notificationsForApp[notification->applicationName()].remove(notification);
        if (m_notificationsForApp[notification->applicationName()].isEmpty()) {
            m_notificationsForApp.remove(notification->applicationName());
        }
    }

    if (m_notifications.count() == 0) {
        emit scrollerEmpty();
        return;
    }

    //clear tabbar
    for (int i = 1; i < m_notificationBar->count(); ++i) {
        if (!m_notificationsForApp.contains(m_notificationBar->tabText(i))) {
            if (i == m_notificationBar->currentIndex()) {
                m_notificationBar->setCurrentIndex(0);
            }
            m_notificationBar->removeTab(i);
        }
    }
}


void NotificationScroller::filterNotificationsByOwner(const QString &owner)
{
    foreach (Notification *notification, m_notifications) {
        Plasma::ExtenderItem *item = m_extenderItemsForNotification.value(notification);

        if (!item || item->group() != this) {
            continue;
        }

        if (owner.isNull() || notification->applicationName() == owner) {
            item->setMaximumHeight(QWIDGETSIZE_MAX);
            item->setVisible(true);
        } else {
            item->setMaximumHeight(0);
            item->setVisible(false);
        }
    }

    m_currentFilter = owner;
}

void NotificationScroller::tabSwitched(int index)
{
    if (index > 0) {
        filterNotificationsByOwner(m_notificationBar->tabText(index));
    } else {
        filterNotificationsByOwner(QString());
    }
}


#include "notificationscroller.moc"

