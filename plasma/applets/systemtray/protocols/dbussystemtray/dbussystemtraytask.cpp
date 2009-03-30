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
#include "notificationarea_interface.h"
#include "systemtraytypes.h"

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

    QPixmap iconDataToPixmap(const Icon &icon) const;

    void iconDestroyed(QObject *obj);
    void refresh();
    void syncIcon();

    void syncAttentionIcon();
    void blinkAttention();
    void syncMovie();
    void updateMovieFrame();

    void syncTooltip();
    void syncStatus(QString status);



    DBusSystemTrayTask *q;
    QString name;
    QString title;
    DBusSystemTrayTask::Status status;
    DBusSystemTrayTask::Category category;
    QIcon icon;
    QIcon attentionIcon;
    QVector<QPixmap> movie;
    int currentFrame;
    QTimer *movieTimer;
    QTimer *blinkTimer;
    bool blink;
    QList<Plasma::IconWidget *>iconWidgets;
    Plasma::ToolTipContent tooltipData;
    org::kde::NotificationAreaItem *notificationAreaItemInterface;
};


DBusSystemTrayTask::DBusSystemTrayTask(const QString &service)
    : Task(),
      d(new DBusSystemTrayTaskPrivate(this))
{
    setObjectName("DBusSystemTrayTask");
    qDBusRegisterMetaType<Icon>();
    qDBusRegisterMetaType<IconVector>();

    d->name = service;

    d->notificationAreaItemInterface = new org::kde::NotificationAreaItem(service, "/NotificationAreaItem",
                                                 QDBusConnection::sessionBus());
    d->notificationAreaItemInterface->title();

    d->category = (Category)metaObject()->enumerator(metaObject()->indexOfEnumerator("Category")).keyToValue(d->notificationAreaItemInterface->category().toLatin1());


    connect(d->notificationAreaItemInterface, SIGNAL(newIcon()), this, SLOT(syncIcon()));
    connect(d->notificationAreaItemInterface, SIGNAL(newAttentionIcon()), this, SLOT(syncAttentionIcon()));
    connect(d->notificationAreaItemInterface, SIGNAL(newTooltip()), this, SLOT(syncTooltip()));
    connect(d->notificationAreaItemInterface, SIGNAL(newStatus(QString)), this, SLOT(syncStatus(QString)));
}


DBusSystemTrayTask::~DBusSystemTrayTask()
{
    delete d;
}


QGraphicsWidget* DBusSystemTrayTask::createWidget(Plasma::Applet *host)
{
    kDebug()<<"creating a new icon for the applet"<<(QObject*)host;

    Plasma::IconWidget *iconWidget = new Plasma::IconWidget(host);

    iconWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    iconWidget->setMinimumSize(KIconLoader::SizeSmallMedium, KIconLoader::SizeSmallMedium);
    iconWidget->setPreferredSize(KIconLoader::SizeSmallMedium, KIconLoader::SizeSmallMedium);

    connect(iconWidget, SIGNAL(clicked()), d->notificationAreaItemInterface, SLOT(activate()));
    iconWidget->installEventFilter(this);

    connect(iconWidget, SIGNAL(destroyed(QObject *)), this, SLOT(iconDestroyed(QObject *)));
    d->iconWidgets.append(iconWidget);

    //Delay because syncStatus needs that createWidget is done
    QTimer::singleShot(0, this, SLOT(refresh()));

    return static_cast<QGraphicsWidget*>(iconWidget);
}

bool DBusSystemTrayTask::isEmbeddable() const
{
    return true;
}

bool DBusSystemTrayTask::isValid() const
{
    return !d->name.isEmpty();
}

DBusSystemTrayTask::Category DBusSystemTrayTask::category() const
{
    return d->category;
}

QString DBusSystemTrayTask::name() const
{
    return d->name;
}


QString DBusSystemTrayTask::typeId() const
{
    return d->name;
}


QIcon DBusSystemTrayTask::icon() const
{
    return d->icon;
}


bool DBusSystemTrayTask::eventFilter(QObject *watched, QEvent *event)
{
    Plasma::IconWidget *iw = qobject_cast<Plasma::IconWidget *>(watched);
    if (d->iconWidgets.contains(iw) && event->type() == QEvent::GraphicsSceneContextMenu) {
        QGraphicsSceneMouseEvent *me = static_cast<QGraphicsSceneMouseEvent *>(event);
        d->notificationAreaItemInterface->contextMenu(me->screenPos().x(), me->screenPos().y());
        return true;
    }
    return false;
}



//DBusSystemTrayTaskPrivate

void DBusSystemTrayTaskPrivate::iconDestroyed(QObject *obj)
{
    Plasma::IconWidget *iw = static_cast<Plasma::IconWidget *>(obj);
    iconWidgets.removeAll(iw);
}

void DBusSystemTrayTaskPrivate::refresh()
{
    syncIcon();
    syncAttentionIcon();
    syncMovie();
    syncTooltip();
    syncStatus(notificationAreaItemInterface->status());
}

QPixmap DBusSystemTrayTaskPrivate::iconDataToPixmap(const Icon &icon) const
{
    QImage iconImage(QSize(icon.width, icon.height), QImage::Format_ARGB32);
    iconImage.loadFromData(icon.data);
    return QPixmap::fromImage(iconImage);
}

//normal icon
void DBusSystemTrayTaskPrivate::syncIcon()
{
    if (notificationAreaItemInterface->icon().length() > 0) {
        icon = KIcon(notificationAreaItemInterface->icon());
    } else {
        icon = iconDataToPixmap(notificationAreaItemInterface->image());
    }

    if (status != DBusSystemTrayTask::NeedsAttention) {
        foreach (Plasma::IconWidget *iconWidget, iconWidgets) {
            iconWidget->setIcon(icon);
        }
    }
}


//Attention icon and movie

void DBusSystemTrayTaskPrivate::syncAttentionIcon()
{
    if (notificationAreaItemInterface->attentionIcon().length() > 0) {
        attentionIcon = KIcon(notificationAreaItemInterface->attentionIcon());
    } else {
        attentionIcon = iconDataToPixmap(notificationAreaItemInterface->attentionImage());
    }
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

void DBusSystemTrayTaskPrivate::syncMovie()
{
    IconVector movieData = notificationAreaItemInterface->attentionMovie();
    movie = QVector<QPixmap>(movieData.size());

    if (!movieData.isEmpty()) {
        for (int i=0; i<movieData.size(); ++i) {
            movie[i] = iconDataToPixmap(movieData[i]);
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


//Tooltip

void DBusSystemTrayTaskPrivate::syncTooltip()
{
    if (notificationAreaItemInterface->tooltipTitle().isEmpty()) {
        foreach (Plasma::IconWidget *iconWidget, iconWidgets) {
            Plasma::ToolTipManager::self()->clearContent(iconWidget);
        }
        return;
    }

    QIcon tooltipIcon;
    if (notificationAreaItemInterface->tooltipIcon().length() > 0) {
        tooltipIcon = KIcon(notificationAreaItemInterface->tooltipIcon());
    } else {
        tooltipIcon = iconDataToPixmap(notificationAreaItemInterface->tooltipImage());
    }

    tooltipData.setMainText(notificationAreaItemInterface->tooltipTitle());
    tooltipData.setSubText(notificationAreaItemInterface->tooltipSubTitle());
    tooltipData.setImage(tooltipIcon);
    foreach (Plasma::IconWidget *iconWidget, iconWidgets) {
        Plasma::ToolTipManager::self()->setContent(iconWidget, tooltipData);
    }
}


//Status

void DBusSystemTrayTaskPrivate::syncStatus(QString newStatus)
{
    status = (DBusSystemTrayTask::Status)q->metaObject()->enumerator(q->metaObject()->indexOfEnumerator("Status")).keyToValue(newStatus.toLatin1());

    if (status == DBusSystemTrayTask::NeedsAttention) {
        q->setOrder(Task::Last);
        if (q->hidden() & Task::AutoHidden) {
            q->setHidden(q->hidden()^Task::AutoHidden);
        }

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
        if (status == DBusSystemTrayTask::Active &&
            (q->hidden() & Task::AutoHidden)) {
            q->setHidden(q->hidden()^Task::AutoHidden);
        } else if (status == DBusSystemTrayTask::Passive) {
            q->setHidden(q->hidden()|Task::AutoHidden);
        }
        q->setOrder(Task::Normal);
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

    emit q->changed(q);
}

}

#include "dbussystemtraytask.moc"
