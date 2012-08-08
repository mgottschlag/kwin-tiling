/***************************************************************************
 *   notificationgroup.cpp                                                *
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

#include "notificationgroup.h"
#include "../core/notification.h"
#include "notificationwidget.h"

#include <QGraphicsLinearLayout>
#include <QWidget>

#include <KDebug>
#include <KIcon>
#include <KLocale>

#include <Plasma/TabBar>
#include <Plasma/ExtenderItem>


NotificationGroup::NotificationGroup(Plasma::Extender *parent, uint groupId)
   : Plasma::ExtenderGroup(parent, groupId)
{
    setTransient(true);
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
    QGraphicsLinearLayout *tabsLayout = new QGraphicsLinearLayout(Qt::Horizontal, widget);
    widget->setContentsMargins(0, 4, 0, 0);
    tabsLayout->setContentsMargins(0, 0, 0, 0);
    tabsLayout->addStretch();
    tabsLayout->addItem(m_notificationBar);
    tabsLayout->addStretch();

    setWidget(widget);
    setCollapsed(true);
    setAutoCollapse(false);
}

NotificationGroup::~NotificationGroup()
{
    m_extenderItemsForNotification.clear();
    m_notificationForExtenderItems.clear();
    qDeleteAll(m_notifications);
}


void NotificationGroup::addNotification(Notification *notification)
{
    connect(notification, SIGNAL(notificationDestroyed(Notification*)), this, SLOT(removeNotification(Notification*)));

    NotificationWidget *notificationWidget = new NotificationWidget(notification, this);
    notificationWidget->setTitleBarVisible(false);
    Plasma::ExtenderItem *extenderItem = new ExtenderItem(extender());
    extenderItem->setGroup(this, QPointF(0,0));
    extenderItem->setTransient(true);
    extenderItem->config().writeEntry("type", "notification");
    extenderItem->setWidget(notificationWidget);
    extenderItem->setIcon(QIcon());
    extenderItem->showCloseButton();
    connect(extenderItem, SIGNAL(destroyed(Plasma::ExtenderItem*)), this, SLOT(extenderItemDestroyed(Plasma::ExtenderItem*)));
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
    m_appForNotification[notification] = notification->applicationName();

    if (!m_currentFilter.isNull() && m_currentFilter != notification->applicationName()) {
        extenderItem->setMaximumHeight(0);
        extenderItem->setVisible(false);
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
        if (m_notificationBar->count() > 2) {
            setCollapsed(false);
            setAutoCollapse(true);
        }
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

void NotificationGroup::extenderItemDestroyed(Plasma::ExtenderItem *object)
{
    if (m_extenderItemsForNotification.isEmpty()) {
        // either we aren't tracking this notification or else we're being deleted
        return;
    }

    Notification *n = m_notificationForExtenderItems.value(object);

    if (n) {
        m_notificationForExtenderItems.remove(object);
        removeNotification(n);
        n->deleteLater();
    }
}

void NotificationGroup::removeNotification(Notification *notification)
{
    if (m_extenderItemsForNotification.isEmpty()) {
        // either we aren't tracking this notification or else we're being deleted
        return;
    }

    Plasma::ExtenderItem *item = m_extenderItemsForNotification.value(notification);
    if (item) {
        m_notificationForExtenderItems.remove(item);
    }

    m_extenderItemsForNotification.remove(notification);
    m_notifications.removeAll(notification);
    QString applicationName = m_appForNotification.value(notification);

    if (applicationName.isEmpty()) {
        return;
    }

    m_appForNotification.remove(notification);

    if (m_notificationsForApp.contains(applicationName)) {
        m_notificationsForApp[applicationName].remove(notification);
        if (m_notificationsForApp[applicationName].isEmpty()) {
            m_notificationsForApp.remove(applicationName);
        }
    }

    //clear tabbar
    for (int i = 1; i < m_notificationBar->count(); ++i) {
        if (!m_notificationsForApp.contains(m_notificationBar->tabText(i))) {
            if (i == m_notificationBar->currentIndex()) {
                m_notificationBar->setCurrentIndex(0);
            }
            m_notificationBar->removeTab(i);
            //2 tabs means just "all" and a single application, no need to display it
            if (m_notificationBar->count() <= 2) {
                setCollapsed(true);
                setAutoCollapse(false);
            }
        }
    }

    if (m_notifications.count() == 0) {
        emit scrollerEmpty();
        return;
    }
}


void NotificationGroup::filterNotificationsByOwner(const QString &owner)
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

void NotificationGroup::tabSwitched(int index)
{
    if (index > 0) {
        filterNotificationsByOwner(m_notificationBar->tabText(index));
    } else {
        filterNotificationsByOwner(QString());
    }
}


#include "notificationgroup.moc"

