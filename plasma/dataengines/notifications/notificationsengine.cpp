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
#include "notificationsadaptor.h"

#include <Plasma/DataContainer>
#include <Plasma/Service>

#include <QImage>

NotificationsEngine::NotificationsEngine( QObject* parent, const QVariantList& args )
    : Plasma::DataEngine( parent, args ), m_nextId( 1 )
{
    NotificationsAdaptor* adaptor = new NotificationsAdaptor(this);
    connect(this, SIGNAL(NotificationClosed(uint, uint)),
            adaptor, SIGNAL(NotificationClosed(uint,uint)));
    connect(this, SIGNAL(ActionInvoked(uint, const QString&)),
            adaptor, SIGNAL(ActionInvoked(uint, const QString&)));

    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerService( "org.freedesktop.Notifications" );
    dbus.registerObject( "/org/freedesktop/Notifications", this );
}

NotificationsEngine::~NotificationsEngine()
{
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.unregisterService( "org.freedesktop.Notifications" );
}

void NotificationsEngine::init()
{
}

inline void copyLineRGB32(int* dst, const char* src, int width)
{
    const char* end = src + width * 3;
    for (; src != end; ++dst, src+=3) {
        *dst = qRgb(src[0], src[1], src[2]);
    }
}

inline void copyLineARGB32(int* dst, const char* src, int width)
{
    const char* end = src + width * 4;
    for (; src != end; ++dst, src+=4) {
        *dst = qRgba(src[0], src[1], src[2], src[3]);
    }
}

static QImage decodeNotificationSpecImageHint(const QDBusArgument& arg)
{
    int width, height, rowStride, hasAlpha, bitsPerSample, channels;
    QByteArray pixels;
    char* ptr;
    char* end;

    arg.beginStructure();
    arg >> width >> height >> rowStride >> hasAlpha >> bitsPerSample >> channels >> pixels;
    arg.endStructure();
    //kDebug() << width << height << rowStride << hasAlpha << bitsPerSample << channels;

    QImage::Format format = QImage::Format_Invalid;
    void (*fcn)(int*, const char*, int) = 0;
    if (bitsPerSample == 8) {
        if (channels == 4) {
            format = QImage::Format_ARGB32;
            fcn = copyLineARGB32;
        } else if (channels == 3) {
            format = QImage::Format_RGB32;
            fcn = copyLineRGB32;
        }
    }
    if (format == QImage::Format_Invalid) {
        kWarning() << "Unsupported image format (hasAlpha:" << hasAlpha << "bitsPerSample:" << bitsPerSample << "channels:" << channels << ")";
        return QImage();
    }

    QImage image(width, height, format);
    ptr = pixels.data();
    end = ptr + pixels.length();
    for (int y=0; y<height && ptr < end; ++y, ptr += rowStride) {
        fcn((int*)image.scanLine(y), ptr, width);
    }

    return image;
}

uint NotificationsEngine::Notify(const QString &app_name, uint replaces_id,
                                 const QString &app_icon, const QString &summary, const QString &body,
                                 const QStringList &actions, const QVariantMap &hints, int timeout)
{
    uint id = 0;
    id = replaces_id ? replaces_id : m_nextId++;

    QString appname_str = app_name;
    if (appname_str.isEmpty()) {
        appname_str = i18n("Unknown Application");
    }

    if (timeout == -1) {
        const int AVERAGE_WORD_LENGTH = 6;
        const int WORD_PER_MINUTE = 250;
        int count = summary.length() + body.length();
        timeout = 60000 * count / AVERAGE_WORD_LENGTH / WORD_PER_MINUTE;

        // Add two seconds for the user to notice the notification, and ensure
        // it last at least five seconds, otherwise all the user see is a
        // flash
        timeout = 2000 + qMin(timeout, 3000);
    }

    const QString source = QString("notification %1").arg(id);
    if (replaces_id) {
        Plasma::DataContainer *container = containerForSource(source);
        if (container && container->data()["expireTimeout"].toInt() != timeout) {
            int timerId = m_sourceTimers.value(source);
            killTimer(timerId);
            m_sourceTimers.remove(source);
            m_timeouts.remove(timerId);
        }
    }

    Plasma::DataEngine::Data notificationData;
    notificationData.insert("id", QString::number(id));
    notificationData.insert("appName", appname_str);
    notificationData.insert("appIcon", app_icon);
    notificationData.insert("summary", summary);
    notificationData.insert("body", body);
    notificationData.insert("actions", actions);
    notificationData.insert("expireTimeout", timeout);

    QImage image;
    if (hints.contains("image_data")) {
        QDBusArgument arg = hints["image_data"].value<QDBusArgument>();
        image = decodeNotificationSpecImageHint(arg);
    }
    notificationData.insert("image", image);

    setData(source, notificationData );

    if (timeout) {
        int timerId = startTimer(timeout);
        m_sourceTimers.insert(source, timerId);
        m_timeouts.insert(timerId, source);
    }

    return id;
}

void NotificationsEngine::timerEvent(QTimerEvent *event)
{
    const QString source = m_timeouts.value(event->timerId());
    if (!source.isEmpty()) {
        m_sourceTimers.remove(source);
        m_timeouts.remove(event->timerId());
        removeSource(source);
        return;
    }

    Plasma::DataEngine::timerEvent(event);
}

void NotificationsEngine::CloseNotification(uint id)
{
    removeSource(QString("notification %1").arg(id));
    emit NotificationClosed(id, 0);
}

Plasma::Service* NotificationsEngine::serviceForSource(const QString& source)
{
    return new NotificationService(this, source);
}

QStringList NotificationsEngine::GetCapabilities()
{
    return QStringList()
        << "body"
        << "body-hyperlinks"
        << "body-markup"
        << "icon-static"
        << "actions"
        ;
}

// FIXME: Signature is ugly
QString NotificationsEngine::GetServerInformation(QString& vendor, QString& version, QString& specVersion)
{
    vendor = "KDE";
    version = "1.0"; // FIXME
    specVersion = "0.10";
    return "Plasma";
}

K_EXPORT_PLASMA_DATAENGINE(notifications, NotificationsEngine)

#include "notificationsengine.moc"
