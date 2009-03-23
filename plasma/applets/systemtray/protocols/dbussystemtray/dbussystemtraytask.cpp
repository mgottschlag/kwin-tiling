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
#include "systemtray_interface.h"
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
          movieTimer(0)
    {
    }

    ~DBusSystemTrayTaskPrivate()
    {
        delete iconWidget;
    }

    void syncIcon();
    void syncTooltip();
    void syncStatus(int status);
    QPixmap iconDataToPixmap(const Icon &icon) const;
    void syncMovie();
    void updateMovieFrame();


    DBusSystemTrayTask *q;
    QString name;
    QString title;
    DBusSystemTrayTask::Status status;
    DBusSystemTrayTask::Category category;
    QIcon icon;
    QVector<QPixmap> movie;
    int currentFrame;
    QTimer *movieTimer;
    Plasma::IconWidget *iconWidget;
    Plasma::ToolTipContent tooltipData;
    org::kde::SystemTray *systemTrayIcon;
};


DBusSystemTrayTask::DBusSystemTrayTask(const QString &service)
    : Task(),
      d(new DBusSystemTrayTaskPrivate(this))
{
    setObjectName("DBusSystemTrayTask");
    qDBusRegisterMetaType<Icon>();
    qDBusRegisterMetaType<IconVector>();

    d->name = service;
    d->iconWidget = new Plasma::IconWidget();

    d->iconWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    d->iconWidget->setMinimumSize(KIconLoader::SizeSmallMedium, KIconLoader::SizeSmallMedium);
    d->iconWidget->setPreferredSize(KIconLoader::SizeSmallMedium, KIconLoader::SizeSmallMedium);

    d->systemTrayIcon = new org::kde::SystemTray(service, "/SystemTray",
                                                 QDBusConnection::sessionBus());
    d->title = d->systemTrayIcon->title().value();
    d->category = (Category)d->systemTrayIcon->category().value();
    d->syncIcon();
    d->syncTooltip();
    d->syncStatus(d->systemTrayIcon->status().value());

    connect(d->systemTrayIcon, SIGNAL(newIcon()), this, SLOT(syncIcon()));
    connect(d->systemTrayIcon, SIGNAL(newTooltip()), this, SLOT(syncTooltip()));
    connect(d->systemTrayIcon, SIGNAL(newStatus(int)), this, SLOT(syncStatus(int)));

    connect(d->iconWidget, SIGNAL(clicked()), d->systemTrayIcon, SLOT(activate()));
    d->iconWidget->installEventFilter(this);
}


DBusSystemTrayTask::~DBusSystemTrayTask()
{
    delete d;
}


QGraphicsWidget* DBusSystemTrayTask::createWidget(Plasma::Applet *host)
{
    Q_UNUSED(host)
    return static_cast<QGraphicsWidget*>(d->iconWidget);
}

bool DBusSystemTrayTask::isEmbeddable() const
{
    return true;
}

bool DBusSystemTrayTask::isValid() const
{
    return !d->name.isEmpty();
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
    if (watched == d->iconWidget && event->type() == QEvent::GraphicsSceneContextMenu) {
        QGraphicsSceneMouseEvent *me = static_cast<QGraphicsSceneMouseEvent *>(event);
        d->systemTrayIcon->contextMenu(me->screenPos().x(), me->screenPos().y());
        return true;
    }
    return false;
}



//DBusSystemTrayTaskPrivate

QPixmap DBusSystemTrayTaskPrivate::iconDataToPixmap(const Icon &icon) const
{
    QImage iconImage(QSize(icon.width, icon.height), QImage::Format_ARGB32);
    iconImage.loadFromData(icon.data);
    return QPixmap::fromImage(iconImage);
}

void DBusSystemTrayTaskPrivate::syncMovie()
{
    QDBusReply<IconVector> movieReply = systemTrayIcon->attentionMovie();

    QVector<Icon> movieData = movieReply.value();
    movie = QVector<QPixmap>(movieData.size());

    if (!movieData.isEmpty()) {
        for (int i=0; i<movieData.size(); ++i) {
            movie[i] = iconDataToPixmap(movieData[i]);
        }
    }
}

void DBusSystemTrayTaskPrivate::updateMovieFrame()
{
    iconWidget->setIcon(movie[currentFrame]);
    currentFrame = (currentFrame + 1) % movie.size();
}

void DBusSystemTrayTaskPrivate::syncIcon()
{
    if (systemTrayIcon->icon().value().length() > 0) {
        icon = KIcon(systemTrayIcon->icon());
    } else {
        QDBusReply<Icon> iconReply = systemTrayIcon->image();
        Icon iconStruct = iconReply.value();
        icon = iconDataToPixmap(iconStruct);
    }

    if (status != DBusSystemTrayTask::NeedsAttention) {
        iconWidget->setIcon(icon);
    }
}

void DBusSystemTrayTaskPrivate::syncTooltip()
{
    if (systemTrayIcon->tooltipTitle().value().isEmpty()) {
        Plasma::ToolTipManager::self()->clearContent(iconWidget);
        return;
    }

    QIcon tooltipIcon;
    if (systemTrayIcon->tooltipIcon().value().length() > 0) {
        tooltipIcon = KIcon(systemTrayIcon->tooltipIcon());
    } else {
        QDBusReply<Icon> iconReply = systemTrayIcon->tooltipImage();
        Icon iconStruct = iconReply.value();
        tooltipIcon = iconDataToPixmap(iconStruct);
    }

    tooltipData.setMainText(systemTrayIcon->tooltipTitle().value());
    tooltipData.setSubText(systemTrayIcon->tooltipSubTitle().value());
    tooltipData.setImage(tooltipIcon);
    Plasma::ToolTipManager::self()->setContent(iconWidget, tooltipData);
}

void DBusSystemTrayTaskPrivate::syncStatus(int newStatus)
{
    status = (DBusSystemTrayTask::Status)newStatus;
    if (status == DBusSystemTrayTask::NeedsAttention) {
        q->setOrder(Task::Last);
        syncMovie();
        if (movie.size() != 0) {
            if (!movieTimer) {
                movieTimer = new QTimer(q);
                q->connect(movieTimer, SIGNAL(timeout()), q, SLOT(updateMovieFrame()));
                movieTimer->start(250);
            }
        }
    } else {
        q->setOrder(Task::Normal);
        if (movieTimer) {
            movieTimer->stop();
            movieTimer->deleteLater();
            movieTimer = 0;
        }
        syncIcon();
    }
    emit q->changed(q);
}

}

#include "dbussystemtraytask.moc"
