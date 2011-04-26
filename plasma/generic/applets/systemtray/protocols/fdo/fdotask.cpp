/***************************************************************************
 *   fdotask.cpp                                                           *
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

#include "fdographicswidget.h"
#include "fdotask.h"

#include <KWindowSystem>
#include <Plasma/Applet>

namespace SystemTray
{

class FdoTask::Private
{
public:
    Private(WId winId)
        : winId(winId),
          widget(0)
    {
        KWindowInfo info = KWindowSystem::windowInfo(winId, NET::WMName, NET::WM2WindowClass);

        // FIXME: This isn't unique
        typeId = info.windowClassName();

        name = info.name();
        if (name.isEmpty()) {
            name = typeId;
        }

        icon = KWindowSystem::icon(winId);
    }

    WId winId;
    QString name;
    QString typeId;
    QIcon icon;
    FdoGraphicsWidget *widget;
};


FdoTask::FdoTask(WId winId, QObject *parent)
    : Task(parent),
      d(new Private(winId))
{
    setCategory(ApplicationStatus);
}

FdoTask::~FdoTask()
{
    emit taskDeleted(d->winId);
    delete d;
}

bool FdoTask::isEmbeddable() const
{
    return !isUsed();
}

QString FdoTask::name() const
{
    return d->name;
}

QString FdoTask::typeId() const
{
    return d->typeId;
}

QIcon FdoTask::icon() const
{
    return d->icon;
}

void FdoTask::abandon(Plasma::Applet *host)
{
    forget(host);
    if (d->widget) {
        d->widget->hide();
    }
}

QGraphicsWidget* FdoTask::createWidget(Plasma::Applet *applet)
{
    if (!d->widget) {
        d->widget = new FdoGraphicsWidget(d->winId, applet);
        connect(d->widget, SIGNAL(clientClosed()), this, SLOT(deleteLater()));
    }

    return d->widget;
}

}

#include "fdotask.moc"
