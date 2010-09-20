/*
 * Copyright 2008, 2009 by Rob Scheepmaker <r.scheepmaker@student.utwente.nl>
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

#include "notificationsextender.h"

#include <QGraphicsLinearLayout>

#include <Plasma/Applet>
#include <Plasma/ExtenderGroup>
#include <Plasma/ExtenderItem>
#include <Plasma/TabBar>

NotificationsExtender::NotificationsExtender(Plasma::Applet *applet)
    : Plasma::Extender(applet)
{
    m_tabBar = new Plasma::TabBar(this);
    m_tabBar->setTabBarShown(false);
    QGraphicsLinearLayout *lay = static_cast<QGraphicsLinearLayout *>(layout());
    lay->addItem(m_tabBar);
}

NotificationsExtender::~NotificationsExtender()
{
}

void NotificationsExtender::itemAddedEvent(Plasma::ExtenderItem *item, const QPointF &pos)
{
    const int index = itemIndex(item);
    if (itemIndex(item) > -1) {
        m_tabBar->takeTab(index);
    }
    if (!item->group()) {
        const QString name = item->name();
        if (name == "notifications") {
            const int index = m_tabBar->addTab(QString(), item);
            m_notifications = item;

            if (m_jobs && static_cast<Plasma::ExtenderGroup*>(m_jobs.data())->items().count() == 0) {
                m_tabBar->setCurrentIndex(index);
            }
        } else if (item->isGroup() && name == "jobGroup") {
            const int index = m_tabBar->addTab(QString(), item);
            m_jobs = item;
            m_tabBar->setCurrentIndex(index);
        } else {
            Extender::itemAddedEvent(item, pos);
        }
    }
}

void NotificationsExtender::itemRemovedEvent(Plasma::ExtenderItem *item)
{
    const int index = itemIndex(item);
    if (index > -1) {
        m_tabBar->takeTab(index);
    }
}

void NotificationsExtender::showNotifications()
{
    if (m_notifications) {
        int index = itemIndex(m_notifications.data());

        if (index > -1) {
            m_tabBar->setCurrentIndex(index);
        }
    }
}

void NotificationsExtender::showJobs()
{
    if (m_jobs) {
        int index = itemIndex(m_jobs.data());

        if (index > -1) {
            m_tabBar->setCurrentIndex(index);
        }
    }
}

int NotificationsExtender::itemIndex(Plasma::ExtenderItem *item)
{
    for (int i = 0; i < m_tabBar->count(); ++i) {
        //very dangerous but in this limited context we are sure the tabbar will contain only extenderitems
        Plasma::ExtenderItem *candidate = static_cast<Plasma::ExtenderItem *>(m_tabBar->tabAt(i));

        if (candidate == item) {
            return i;
        }
    }

    return -1;
}

#include "notificationsextender.h"
