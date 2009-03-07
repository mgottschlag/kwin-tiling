/***************************************************************************
 *   DBusSystemTrayTask.cpp                                                  *
 *                                                                         *
 *   Copyright (C) 2008 Jason Stubbs <jasonbstubbs@gmail.com>              *
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
        : q(q)
    {
    }

    ~DBusSystemTrayTaskPrivate()
    {
        delete iconWidget;
    }

    void askContextMenu();
    void syncIcon();
    void syncTooltip();
    void syncStatus(int status);


    DBusSystemTrayTask *q;
    QString name;
    QString title;
    DBusSystemTrayTask::Status status;
    DBusSystemTrayTask::Category category;
    QIcon icon;
    Plasma::IconWidget *iconWidget;
    Plasma::ToolTipContent tooltipData;
    org::kde::SystemTray *systemTrayIcon;
};


DBusSystemTrayTask::DBusSystemTrayTask(const QString &service)
    : Task(),
      d(new DBusSystemTrayTaskPrivate(this))
{
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
        d->askContextMenu();
        return true;
    }
    return false;
}

//DBusSystemTrayTaskPrivate

void DBusSystemTrayTaskPrivate::askContextMenu()
{
    QPoint popupPos(0,0);

    QGraphicsView *view = Plasma::viewFor(iconWidget);
    if (view) {
        popupPos = view->mapToGlobal(view->mapFromScene(iconWidget->scenePos()));
    }

    systemTrayIcon->contextMenu(popupPos.x(), popupPos.y());
}

void DBusSystemTrayTaskPrivate::syncIcon()
{
    if (!systemTrayIcon->iconName().value().isNull()) {
        icon = KIcon(systemTrayIcon->iconName());
    } else {
        QImage iconImage; iconImage.loadFromData(systemTrayIcon->iconPixmap().value());
        icon = QPixmap::fromImage(iconImage);
    }
    iconWidget->setIcon(icon);
}

void DBusSystemTrayTaskPrivate::syncTooltip()
{
    if (systemTrayIcon->tooltipTitle().value().isEmpty()) {
        Plasma::ToolTipManager::self()->clearContent(iconWidget);
        return;
    }

    QIcon tooltipIcon;
    if (!systemTrayIcon->iconName().value().isNull()) {
        tooltipIcon = KIcon(systemTrayIcon->iconName());
    } else {
        QImage iconImage; iconImage.loadFromData(systemTrayIcon->iconPixmap().value());
        tooltipIcon = QPixmap::fromImage(iconImage);
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
    } else {
        q->setOrder(Task::Normal);
    }
}

}
