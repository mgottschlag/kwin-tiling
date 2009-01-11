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

#include <Plasma/Service>

NotificationsEngine::NotificationsEngine( QObject* parent, const QVariantList& args )
    : Plasma::DataEngine( parent, args ), m_nextId( 1 )
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

uint NotificationsEngine::Notify(const QString &app_name, uint replaces_id, const QString &event_id,
                                 const QString &app_icon, const QString &summary, const QString &body,
                                 const QStringList &actions, const QVariantMap &hints, int timeout)
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
        notificationData.insert("eventId", event_id );
        notificationData.insert("summary", summary );
        notificationData.insert("body", body );
        notificationData.insert("actions", actions );
        notificationData.insert("expireTimeout", timeout );

        setData(QString("notification %1").arg(id), notificationData );
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

K_EXPORT_PLASMA_DATAENGINE(notifications, NotificationsEngine)

#include "notificationsengine.moc"
