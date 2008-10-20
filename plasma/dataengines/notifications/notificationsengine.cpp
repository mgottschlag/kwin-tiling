/*
 *   Copyright (C) 2008 Dmitry Suzdalev <dimsuz@gmail.com>
 *
 * This program is free software you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "notificationsengine.h"
#include "notificationservice.h"
#include "visualnotificationsadaptor.h"

#include <plasma/service.h>

NotificationsEngine::NotificationsEngine( QObject* parent, const QVariantList& args )
    : Plasma::DataEngine( parent, args ), m_nextId( 1 ), m_sigMapper(0)
{
    VisualNotificationsAdaptor* adaptor = new VisualNotificationsAdaptor(this);
    connect(this, SIGNAL(NotificationClosed(uint, uint)),
            adaptor, SIGNAL(NotificationClosed(uint,uint)));
    connect(this, SIGNAL(ActionInvoked(uint, const QString&)),
            adaptor, SIGNAL(ActionInvoked(uint, const QString&)));

    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerService( "org.kde.VisualNotifications" );
    dbus.registerObject( "/VisualNotifications", this );
}

NotificationsEngine::~NotificationsEngine()
{
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.unregisterService( "org.kde.VisualNotifications" );
}

void NotificationsEngine::init()
{
}

uint NotificationsEngine::Notify(const QString &app_name, uint replaces_id, const QString &app_icon,
                                 const QString &summary, const QString &body, const QStringList &actions,
                                 const QVariantMap &hints, int timeout)
{
    uint id = 0;
    if (replaces_id == 0) {
        // new notification

        id = m_nextId++;

        // TODO implement hints support
        Q_UNUSED(hints)

        QString appname_str = app_name;
        if (appname_str.isEmpty()) {
            appname_str = i18n("unknown app");
        }

        Plasma::DataEngine::Data notificationData;
        notificationData.insert("id", QString::number(id ));
        notificationData.insert("appName", appname_str );
        notificationData.insert("appIcon", app_icon );
        notificationData.insert("summary", summary );
        notificationData.insert("body", body );
        notificationData.insert("actions", actions );
        notificationData.insert("expireTimeout", timeout );

        setData(QString("notification %1").arg(id), notificationData );

        // setup timer if needed
        if (timeout != 0){
            // lazy create mapper
            if (!m_sigMapper){
                m_sigMapper = new QSignalMapper(this);
                connect(m_sigMapper, SIGNAL(mapped(int)), SLOT(onNotificationTimedOut(int)));
            }

            // we have a bunch of per-notification timers and map each of them
            // to a notification id.
            QTimer* timer = new QTimer(this);
            timer->setSingleShot(true);
            timer->setInterval(timeout);
            connect(timer, SIGNAL(timeout()), m_sigMapper, SLOT(map()));

            m_sigMapper->setMapping(timer, static_cast<int>(id));

            if (m_timers.contains(id)) {
                kDebug() << "aleady have a timer for this notifiation? that's odd!";
                QTimer *timer = m_timers.value(id);
                delete timer;
            }

            m_timers[id] = timer;
            timer->start();
        }
    } else {
        id = replaces_id;
        // TODO: update existing source
        kDebug() << "notice: updating notifications isn't implemented yet";
    }

    return id;
}

void NotificationsEngine::CloseNotification(uint id)
{
    removeSource(QString("notification %1").arg(id));
    emit NotificationClosed(id,0);
}

Plasma::Service* NotificationsEngine::serviceForSource(const QString& source)
{
    return new NotificationService(this, source);
}

void NotificationsEngine::onNotificationTimedOut(int timedoutId)
{
    uint id = static_cast<uint>(timedoutId);
    // don't forget to destroy timer
    if (m_timers.contains(id)) {
        QTimer *timer = m_timers.value(id);
        m_timers.remove(id);
        timer->deleteLater();
    }

    CloseNotification(id);
}

K_EXPORT_PLASMA_DATAENGINE(notifications, NotificationsEngine)

#include "notificationsengine.moc"
