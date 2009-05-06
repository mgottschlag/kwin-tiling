/***************************************************************************
 *                                                                         *
 *   Copyright (C) 2009 Marco Martin <notmart@gmail.com>                   *
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

#include "dbussystemtraytask.h"

#include <QGraphicsWidget>
#include <QGraphicsSceneContextMenuEvent>
#include <QIcon>
#include <QDBusReply>

#include <KIcon>
#include <KIconLoader>

#include <plasma/plasma.h>
#include <Plasma/Corona>
#include <Plasma/View>
#include <Plasma/IconWidget>
#include <Plasma/ToolTipContent>
#include <Plasma/ToolTipManager>
#include <Plasma/Applet>

#include "dbussystemtraywidget.h"
#include "notificationitem_interface.h"
#include "systemtraytypes.h"

namespace SystemTray
{

class DBusSystemTrayTaskPrivate
{
public:
    DBusSystemTrayTaskPrivate(DBusSystemTrayTask *q)
        : q(q),
          currentFrame(0),
          movieTimer(0),
          blinkTimer(0),
          blink(false)
    {
    }

    ~DBusSystemTrayTaskPrivate()
    {
        delete movieTimer;
        delete blinkTimer;
    }

    QPixmap ExperimentalKDbusImageStructToPixmap(const ExperimentalKDbusImageStruct &image) const;
    QIcon imageVectorToPixmap(const ExperimentalKDbusImageVector &vector) const;
    void overlayIcon(QIcon *icon, QIcon *overlay);

    void iconDestroyed(QObject *obj);
    void refresh();

    void blinkAttention();
    void updateMovieFrame();

    void syncToolTip();
    void syncStatus(QString status);

    //callbacks
    void syncToolTip(const ExperimentalKDbusToolTipStruct &);
    void syncMovie(const ExperimentalKDbusImageVector &);
    void refreshCallback(QDBusPendingCallWatcher *call);


    DBusSystemTrayTask *q;
    QString id;
    QString name;
    QString title;
    QIcon icon;
    QIcon attentionIcon;
    QVector<QPixmap> movie;
    int currentFrame;
    QTimer *movieTimer;
    QTimer *blinkTimer;
    bool blink;
    QHash<Plasma::Applet *, Plasma::IconWidget *>iconWidgets;
    Plasma::ToolTipContent toolTipData;
    org::kde::NotificationItem *notificationItemInterface;
};


DBusSystemTrayTask::DBusSystemTrayTask(const QString &service, QObject *parent)
    : Task(parent),
      d(new DBusSystemTrayTaskPrivate(this))
{
    setObjectName("DBusSystemTrayTask");
    qDBusRegisterMetaType<ExperimentalKDbusImageStruct>();
    qDBusRegisterMetaType<ExperimentalKDbusImageVector>();
    qDBusRegisterMetaType<ExperimentalKDbusToolTipStruct>();

    d->id = service;
    d->name = service;

    d->notificationItemInterface = new org::kde::NotificationItem(service, "/NotificationItem",
                                                                          QDBusConnection::sessionBus(), this);

    d->refresh();

    connect(d->notificationItemInterface, SIGNAL(NewIcon()), this, SLOT(refresh()));
    connect(d->notificationItemInterface, SIGNAL(NewAttentionIcon()), this, SLOT(refresh()));
    connect(d->notificationItemInterface, SIGNAL(NewToolTip()), this, SLOT(refresh()));
    connect(d->notificationItemInterface, SIGNAL(NewStatus(QString)), this, SLOT(syncStatus(QString)));
}


DBusSystemTrayTask::~DBusSystemTrayTask()
{
    delete d;
}


QGraphicsWidget* DBusSystemTrayTask::createWidget(Plasma::Applet *host)
{
    if (d->iconWidgets.contains(host)) {
        return d->iconWidgets[host];
    }

    DBusSystemTrayWidget *iconWidget = new DBusSystemTrayWidget(host, d->notificationItemInterface);
    iconWidget->show();

    iconWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    iconWidget->setMinimumSize(KIconLoader::SizeSmallMedium, KIconLoader::SizeSmallMedium);
    iconWidget->setPreferredSize(KIconLoader::SizeSmallMedium, KIconLoader::SizeSmallMedium);

    connect(iconWidget, SIGNAL(destroyed(QObject *)), this, SLOT(iconDestroyed(QObject *)));
    d->iconWidgets[host] = iconWidget;

    //Delay because syncStatus needs that createWidget is done
    QTimer::singleShot(0, this, SLOT(refresh()));
    return iconWidget;
}

bool DBusSystemTrayTask::isEmbeddable() const
{
    return true;
}

bool DBusSystemTrayTask::isValid() const
{
    return !d->id.isEmpty();
}

QString DBusSystemTrayTask::name() const
{
    return d->name;
}


QString DBusSystemTrayTask::typeId() const
{
    return d->id;
}


QIcon DBusSystemTrayTask::icon() const
{
    return d->icon;
}


//DBusSystemTrayTaskPrivate

void DBusSystemTrayTaskPrivate::iconDestroyed(QObject *obj)
{
    Plasma::IconWidget *iw = static_cast<Plasma::IconWidget *>(obj);

    QHash<Plasma::Applet *, Plasma::IconWidget*>::const_iterator i = iconWidgets.constBegin();
    while (i != iconWidgets.constEnd()) {
        if (i.value() == iw) {
            iconWidgets.remove(i.key());
            return;
        }
        ++i;
    }
}

void DBusSystemTrayTaskPrivate::refresh()
{
    QDBusMessage message = QDBusMessage::createMethodCall(notificationItemInterface->service(),
    notificationItemInterface->path(), "org.freedesktop.DBus.Properties", "GetAll");

    message << notificationItemInterface->interface();
    QDBusPendingCall call = notificationItemInterface->connection().asyncCall(message);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, q);
    q->connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher *)), q, SLOT(refreshCallback(QDBusPendingCallWatcher *)));
}


void DBusSystemTrayTaskPrivate::refreshCallback(QDBusPendingCallWatcher *call)
{
    QDBusPendingReply<QVariantMap> reply = *call;
    QVariantMap properties = reply.argumentAt<0>();
    if (!reply.isError()) {
        QString cat = properties["Category"].toString();
        if (!cat.isEmpty()) {
            int index = q->metaObject()->indexOfEnumerator("Category");
            int key = q->metaObject()->enumerator(index).keyToValue(cat.toLatin1());

            if (key != -1) {
                q->setCategory((Task::Category)key);
            }
        }

        syncStatus(properties["Status"].toString());

        QString title = properties["Title"].toString();
        if (!title.isEmpty()) {
            name = title;
        }

        QIcon overlay;

        //Icon
        {
            ExperimentalKDbusImageVector image;

            properties["OverlayImage"].value<QDBusArgument>()>>image;
            if (image.size() == 0) {
                overlay = KIcon(properties["OverlayIcon"].toString());
            } else {
                overlay = imageVectorToPixmap(image);
            }

            properties["Image"].value<QDBusArgument>()>>image;
            if (image.size() == 0) {
                icon = KIcon(properties["Icon"].toString());
            } else {
                icon = imageVectorToPixmap(image);
            }
            overlayIcon(&icon, &overlay);
        }

        if (q->status() != Task::NeedsAttention) {
            foreach (Plasma::IconWidget *iconWidget, iconWidgets) {
                iconWidget->setIcon(icon);
            }
        }

        //Attention icon
        {
            ExperimentalKDbusImageVector image;
            properties["AttentionImage"].value<QDBusArgument>()>>image;
            if (image.size() == 0) {
                attentionIcon = KIcon(properties["AttentionIcon"].toString());
            } else {
                attentionIcon = imageVectorToPixmap(image);
            }
            overlayIcon(&attentionIcon, &overlay);
        }

        ExperimentalKDbusImageVector movie;
        properties["AttentionMovie"].value<QDBusArgument>()>>movie;
        syncMovie(movie);

        ExperimentalKDbusToolTipStruct toolTip;
        properties["ToolTip"].value<QDBusArgument>()>>toolTip;
        syncToolTip(toolTip);
    }
    delete call;
}

QPixmap DBusSystemTrayTaskPrivate::ExperimentalKDbusImageStructToPixmap(const ExperimentalKDbusImageStruct &icon) const
{
    QImage iconImage( icon.width, icon.height, QImage::Format_ARGB32 );
    memcpy(iconImage.bits(), (uchar*)icon.data.data(), iconImage.numBytes());

    return QPixmap::fromImage(iconImage);
}

QIcon DBusSystemTrayTaskPrivate::imageVectorToPixmap(const ExperimentalKDbusImageVector &vector) const
{
    QIcon icon;

    for (int i = 0; i<vector.size(); ++i) {
        icon.addPixmap(ExperimentalKDbusImageStructToPixmap(vector[i]));
    }

    return icon;
}

void DBusSystemTrayTaskPrivate::overlayIcon(QIcon *icon, QIcon *overlay)
{
    QPixmap iconPixmap = icon->pixmap(KIconLoader::SizeSmall, KIconLoader::SizeSmall);

    QPainter p(&iconPixmap);

    const int size = KIconLoader::SizeSmall/2;
    p.drawPixmap(QRect(size, size, size, size), overlay->pixmap(size, size), QRect(0,0,size,size));
    p.end();

    //if an icon exactly that size wasn't found don't add it to the vector
    iconPixmap = icon->pixmap(KIconLoader::SizeSmallMedium, KIconLoader::SizeSmallMedium);
    if (iconPixmap.width() == KIconLoader::SizeSmallMedium) {
        const int size = KIconLoader::SizeSmall;
        QPainter p(&iconPixmap);
        p.drawPixmap(QRect(iconPixmap.width()-size, iconPixmap.height()-size, size, size), overlay->pixmap(size, size), QRect(0,0,size,size));
        p.end();
    }

    iconPixmap = icon->pixmap(KIconLoader::SizeMedium, KIconLoader::SizeMedium);
    if (iconPixmap.width() == KIconLoader::SizeMedium) {
        const int size = KIconLoader::SizeSmall;
        QPainter p(&iconPixmap);
        p.drawPixmap(QRect(iconPixmap.width()-size, iconPixmap.height()-size, size, size), overlay->pixmap(size, size), QRect(0,0,size,size));
        p.end();
    }

    iconPixmap = icon->pixmap(KIconLoader::SizeLarge, KIconLoader::SizeLarge);
    if (iconPixmap.width() == KIconLoader::SizeLarge) {
        const int size = KIconLoader::SizeSmallMedium;
        QPainter p(&iconPixmap);
        p.drawPixmap(QRect(iconPixmap.width()-size, iconPixmap.height()-size, size, size), overlay->pixmap(size, size), QRect(0,0,size,size));
        p.end();
    }

    //hopefully huge and enormous not necessary right now, since it's quite costly
}

void DBusSystemTrayTaskPrivate::blinkAttention()
{
    if (blink) {
        foreach (Plasma::IconWidget *iconWidget, iconWidgets) {
            iconWidget->setIcon(attentionIcon);
        }
    } else {
        foreach (Plasma::IconWidget *iconWidget, iconWidgets) {
            iconWidget->setIcon(icon);
        }
    }
    blink = !blink;
}

void DBusSystemTrayTaskPrivate::syncMovie(const ExperimentalKDbusImageVector &movieData)
{
    movie = QVector<QPixmap>(movieData.size());

    if (!movieData.isEmpty()) {
        for (int i=0; i<movieData.size(); ++i) {
            movie[i] = ExperimentalKDbusImageStructToPixmap(movieData[i]);
        }
    }
}



void DBusSystemTrayTaskPrivate::updateMovieFrame()
{
    foreach (Plasma::IconWidget *iconWidget, iconWidgets) {
        iconWidget->setIcon(movie[currentFrame]);
    }
    currentFrame = (currentFrame + 1) % movie.size();
}


//toolTip

void DBusSystemTrayTaskPrivate::syncToolTip(const ExperimentalKDbusToolTipStruct &tipStruct)
{
    if (tipStruct.title.isEmpty()) {
        foreach (Plasma::IconWidget *iconWidget, iconWidgets) {
            Plasma::ToolTipManager::self()->clearContent(iconWidget);
        }
        return;
    }

    QIcon toolTipIcon;
    if (tipStruct.image.size() == 0) {
        toolTipIcon = KIcon(tipStruct.icon);
    } else {
        toolTipIcon = imageVectorToPixmap(tipStruct.image);
    }

    toolTipData.setMainText(tipStruct.title);
    toolTipData.setSubText(tipStruct.subTitle);
    toolTipData.setImage(toolTipIcon);
    foreach (Plasma::IconWidget *iconWidget, iconWidgets) {
        Plasma::ToolTipManager::self()->setContent(iconWidget, toolTipData);
    }
}


//Status

void DBusSystemTrayTaskPrivate::syncStatus(QString newStatus)
{
    Task::Status status = (Task::Status)q->metaObject()->enumerator(q->metaObject()->indexOfEnumerator("Status")).keyToValue(newStatus.toLatin1());

    if (status == Task::NeedsAttention) {
        if (movie.size() != 0) {
            if (!movieTimer) {
                movieTimer = new QTimer(q);
                q->connect(movieTimer, SIGNAL(timeout()), q, SLOT(updateMovieFrame()));
                movieTimer->start(100);
            }
        } else if (!attentionIcon.isNull()) {
            if (!blinkTimer) {
                blinkTimer = new QTimer(q);
                q->connect(blinkTimer, SIGNAL(timeout()), q, SLOT(blinkAttention()));
                blinkTimer->start(500);
            }
        }
    } else {
        if (movieTimer) {
            movieTimer->stop();
            movieTimer->deleteLater();
            movieTimer = 0;
        }

        if (blinkTimer) {
            blinkTimer->stop();
            blinkTimer->deleteLater();
            blinkTimer = 0;
        }

        foreach (Plasma::IconWidget *iconWidget, iconWidgets) {
            iconWidget->setIcon(icon);
        }
    }

    q->setStatus(status);
}

}

#include "dbussystemtraytask.moc"
