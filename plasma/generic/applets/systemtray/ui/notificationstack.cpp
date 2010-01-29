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

#include <Plasma/FrameSvg>
#include <Plasma/Dialog>

namespace SystemTray
{

NotificationStack::NotificationStack(QGraphicsItem *parent)
   : QGraphicsWidget(parent),
     m_size(3),
     m_underMouse(false)
{
    m_background = new Plasma::FrameSvg(this);
    m_background->setImagePath("widgets/extender-background");
    qreal left, top, right, bottom;
    m_background->getMargins(left, top, right, bottom);
    setContentsMargins(left, top, right, bottom);

    m_mainLayout = new QGraphicsLinearLayout(Qt::Vertical, this);
    m_delayedRemoveTimer = new QTimer(this);
    m_delayedRemoveTimer->setSingleShot(true);
    connect(m_delayedRemoveTimer, SIGNAL(timeout()), this, SLOT(popNotification()));
    setAcceptsHoverEvents(true);
}

NotificationStack::~NotificationStack()
{
}

void NotificationStack::addNotification(Notification *notification)
{
    connect(notification, SIGNAL(notificationDestroyed(SystemTray::Notification *)), this, SLOT(removeNotification(SystemTray::Notification *)));
    connect(notification, SIGNAL(expired(SystemTray::Notification *)), this, SLOT(delayedRemoveNotification(SystemTray::Notification *)));

    NotificationWidget *notificationWidget = new NotificationWidget(notification, this);
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
    NotificationWidget *nw = m_notificationWidgets.value(notification);
    if (nw) {
        nw->deleteLater();
    }
    m_mainLayout->removeItem(nw);
    m_notificationWidgets.remove(notification);
    m_notifications.removeAll(notification);

    if (m_notifications.count() > 0) {
        setCurrentNotification(m_notifications.first());
    }

    if (m_notifications.count() == 0) {
        emit stackEmpty();
    }

    resize(sizeHint(Qt::MinimumSize, QSizeF()));
}

void NotificationStack::delayedRemoveNotification(SystemTray::Notification *notification)
{
    m_notificationsToRemove.append(notification);
    if (!m_underMouse) {
        m_delayedRemoveTimer->start(1000);
    }
}

void NotificationStack::setCurrentNotification(SystemTray::Notification *notification)
{
    if (m_notificationWidgets.contains(notification)) {
        if (m_currentNotificationWidget) {
            m_currentNotificationWidget.data()->setCollapsed(true);
        }
        m_currentNotificationWidget = m_notificationWidgets.value(notification);
        m_currentNotificationWidget.data()->setCollapsed(false);
    }
}

void NotificationStack::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event)

    m_underMouse = true;
    m_delayedRemoveTimer->stop();
}

void NotificationStack::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event)

    m_underMouse = false;
    m_delayedRemoveTimer->start(1000);
}

void NotificationStack::paint(QPainter *painter,
                              const QStyleOptionGraphicsItem *option,
                              QWidget *widget)
{
    if (!m_currentNotificationWidget || m_mainLayout->count() < 2) {
        return;
    }

    for (int i = 0; i < m_mainLayout->count(); ++i) {
        //assumption++ all items are NotificationWidget
        NotificationWidget *nw = static_cast<NotificationWidget *>(m_mainLayout->itemAt(i));

        //first element
        if (i == 0 && m_currentNotificationWidget.data() != nw) {
            continue;
        } else if (i == 0) {
            m_background->setEnabledBorders((Plasma::FrameSvg::EnabledBorders)(Plasma::FrameSvg::AllBorders&~Plasma::FrameSvg::TopBorder));
        //last element
        } else if (i == m_mainLayout->count()-1 && m_currentNotificationWidget.data() != nw) {
            continue;
        } else if (i == m_mainLayout->count()-1) {
            m_background->setEnabledBorders((Plasma::FrameSvg::EnabledBorders)Plasma::FrameSvg::AllBorders&~Plasma::FrameSvg::BottomBorder);
        //element under the active one
        } else if (m_currentNotificationWidget.data()->pos().y() < nw->pos().y()) {
            m_background->setEnabledBorders((Plasma::FrameSvg::EnabledBorders)Plasma::FrameSvg::AllBorders^Plasma::FrameSvg::TopBorder);
        //element over the active one
        } else if (m_currentNotificationWidget.data()->pos().y() > nw->pos().y()) {
            m_background->setEnabledBorders((Plasma::FrameSvg::EnabledBorders)Plasma::FrameSvg::AllBorders^Plasma::FrameSvg::BottomBorder);
        //active element
        } else {
            m_background->setEnabledBorders(Plasma::FrameSvg::AllBorders);
        }

        qreal left, top, right, bottom;
        m_background->getMargins(left, top, right, bottom);
        m_background->resizeFrame(QSizeF(size().width(), nw->size().height() + top+bottom));

        m_background->paintFrame(painter, QPointF(0, nw->pos().y() - top));
    }
}

bool NotificationStack::eventFilter(QObject *watched, QEvent *event)
{
    //FIXME: shouldn't be there :)
    Plasma::Dialog *dialog = qobject_cast<Plasma::Dialog *>(watched);
    if (dialog) {
        if (event->type() == QEvent::ContentsRectChange) {
            int left, top, right, bottom;
            dialog->getContentsMargins(&left, &top, &right, &bottom);
            left = 0;
            right = 0;
            dialog->setContentsMargins(left, top, right, bottom);
        }
    }

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
    } else if (event->type() == QEvent::GraphicsSceneMove) {
        update();
    }

    return false;
}

void NotificationStack::popNotification()
{
    if (m_notificationsToRemove.isEmpty()) {
        return;
    }

    Notification *notif = m_notificationsToRemove.first();
    removeNotification(notif);
    m_notificationsToRemove.pop_front();
    m_delayedRemoveTimer->start(1000);
}

}

#include "notificationstack.moc"
