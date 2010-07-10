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
#include <Plasma/Extender>
#include <Plasma/ScrollWidget>


NotificationScroller::NotificationScroller(QGraphicsItem *parent)
   : QGraphicsWidget(parent),
     m_location(Plasma::BottomEdge)
{
    m_layout = new QGraphicsLinearLayout(Qt::Vertical, this);
    m_tabsLayout = new QGraphicsLinearLayout(Qt::Horizontal);

    m_notificationBar = new Plasma::TabBar(this);
    m_notificationBar->nativeWidget()->setMaximumWidth(400);
    m_notificationBar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_notificationBar->addTab(KIcon("dialog-information"), i18nc("Show all  notifications", "All"));
    connect(m_notificationBar, SIGNAL(currentChanged(int)), this, SLOT(tabSwitched(int)));

    m_scroll = new Plasma::ScrollWidget(this);
    m_scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_tabsLayout->addStretch();
    m_tabsLayout->addItem(m_notificationBar);
    m_tabsLayout->addStretch();

    m_layout->addItem(m_scroll);
    m_layout->addItem(m_tabsLayout);

    m_mainWidget = new QGraphicsWidget(m_scroll);
    m_mainWidgetLayout = new QGraphicsLinearLayout(Qt::Vertical, m_mainWidget);
    m_scroll->setWidget(m_mainWidget);
}

NotificationScroller::~NotificationScroller()
{
    qDeleteAll(m_notifications);
}


void NotificationScroller::addNotification(Notification *notification)
{
    connect(notification, SIGNAL(notificationDestroyed(Notification *)), this, SLOT(removeNotification(Notification *)));

    NotificationWidget *notificationWidget = new NotificationWidget(notification, m_mainWidget);
    connect(notificationWidget, SIGNAL(destroyed()), this, SLOT(adjustSize()));

    m_notificationWidgets[notification] = notificationWidget;
    m_notifications.append(notification);
    m_notificationsForApp[notification->applicationName()].insert(notification);

    if (!m_currentFilter.isNull() && m_currentFilter != notification->applicationName()) {
        notificationWidget->setMaximumHeight(0);
        notificationWidget->setVisible(false);
    }

    m_mainWidgetLayout->insertItem(0, notificationWidget);


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

    notificationWidget->layout()->activate();
    adjustSize();
}

void NotificationScroller::removeNotification(Notification *notification)
{
    m_mainWidgetLayout->removeItem(m_notificationWidgets.value(notification));
    m_notificationWidgets.remove(notification);
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
        if (owner.isNull() || notification->applicationName() == owner) {
            if (m_notificationWidgets.contains(notification)) {
                m_notificationWidgets.value(notification)->setMaximumHeight(QWIDGETSIZE_MAX);
                m_notificationWidgets.value(notification)->setVisible(true);
            }
        } else {
            if (m_notificationWidgets.contains(notification)) {
                m_notificationWidgets.value(notification)->setMaximumHeight(0);
                m_notificationWidgets.value(notification)->setVisible(false);
            }
        }
    }

    m_currentFilter = owner;

    adjustSize();
}

void NotificationScroller::tabSwitched(int index)
{
    if (index > 0) {
        filterNotificationsByOwner(m_notificationBar->tabText(index));
    } else {
        filterNotificationsByOwner(QString());
    }
}

Plasma::Location NotificationScroller::location() const
{
    return m_location;
}

void NotificationScroller::setLocation(const Plasma::Location location)
{
    if (!m_layout || (location == m_location)) {
        return;
    }

    m_location = location;

    if (m_location == Plasma::TopEdge) {
        m_layout->removeItem(m_tabsLayout);
        m_layout->insertItem(0, m_tabsLayout);
    } else {
        m_layout->removeItem(m_tabsLayout);
        m_layout->addItem(m_tabsLayout);
    }

}

void NotificationScroller::adjustSize()
{
    m_mainWidgetLayout->invalidate();
    static_cast<QGraphicsLayoutItem *>(m_mainWidget)->updateGeometry();
    m_mainWidget->resize(m_mainWidget->effectiveSizeHint(Qt::PreferredSize));
    static_cast<QGraphicsLayoutItem *>(m_scroll)->updateGeometry();
    updateGeometry();

    Plasma::ExtenderItem *ei = qobject_cast<Plasma::ExtenderItem *>(parentWidget());
    if (ei && ei->extender()) {
        static_cast<QGraphicsLayoutItem *>(ei)->updateGeometry();
        static_cast<QGraphicsLayoutItem *>(ei->extender())->updateGeometry();

        QSizeF hint = ei->extender()->effectiveSizeHint(Qt::PreferredSize);
        ei->extender()->resize(hint);
    }
}

