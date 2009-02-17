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

#include <KIcon>
#include <KIconLoader>

#include <Plasma/IconWidget>

namespace SystemTray
{

class DBusSystemTrayTaskPrivate
{
public:
    DBusSystemTrayTaskPrivate(QString service, DBusSystemTrayTask *q)
        : q(q),
          service(service)
    {
    }

    ~DBusSystemTrayTaskPrivate()
    {
        delete iconWidget;
    }


    DBusSystemTrayTask *q;
    QString service;
    QString name;
    QIcon icon;
    Plasma::IconWidget *iconWidget;
};


DBusSystemTrayTask::DBusSystemTrayTask(const QString &service)
    : Task(),
      d(new DBusSystemTrayTaskPrivate(service, this))
{
    d->name = service;
    d->iconWidget = new Plasma::IconWidget();
    d->icon = KIcon("applications-engineering");
    d->iconWidget->setIcon(d->icon);
    d->iconWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    d->iconWidget->setMinimumSize(KIconLoader::SizeSmallMedium, KIconLoader::SizeSmallMedium);
    d->iconWidget->setPreferredSize(KIconLoader::SizeSmallMedium, KIconLoader::SizeSmallMedium);
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

}
