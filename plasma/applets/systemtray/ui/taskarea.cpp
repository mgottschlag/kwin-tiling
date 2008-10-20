/***************************************************************************
 *   taskarea.cpp                                                          *
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

#include "taskarea.h"

#include <QtCore/QSet>

#include <plasma/applet.h>

#include "../core/task.h"
#include "compactlayout.h"


namespace SystemTray
{


class TaskArea::Private
{
public:
    Private(Plasma::Applet *h)
        : host(h),
          layout(new CompactLayout())
    {
    }

    QGraphicsWidget* findWidget(Task *task);

    Plasma::Applet *host;
    CompactLayout *layout;
    QSet<QString> hiddenTypes;
};


TaskArea::TaskArea(Plasma::Applet *parent)
    : QGraphicsWidget(parent),
      d(new Private(parent))
{
    setLayout(d->layout);
}


TaskArea::~TaskArea()
{
    delete d;
}


void TaskArea::setHiddenTypes(const QStringList &hiddenTypes)
{
    d->hiddenTypes = QSet<QString>::fromList(hiddenTypes);
}


bool TaskArea::isHiddenType(const QString &typeId) const
{
    return d->hiddenTypes.contains(typeId);
}


void TaskArea::syncTasks(const QList<SystemTray::Task*> &tasks)
{
    foreach (Task *task, tasks) {
        if (isHiddenType(task->typeId())) {
            QGraphicsWidget *widget = d->findWidget(task);
            if (widget) {
                d->layout->removeItem(widget);
                delete widget;
            }
        } else {
            addTask(task);
        }
    }
}


void TaskArea::addTask(Task *task)
{
    if (task->isEmbeddable() && !isHiddenType(task->typeId()) && !d->findWidget(task)) {
        d->layout->addItem(task->widget(d->host));
        emit sizeHintChanged(Qt::PreferredSize);
    }
}


void TaskArea::removeTask(Task *task)
{
    foreach (QGraphicsWidget *widget, task->associatedWidgets()) {
        if (d->layout->containsItem(widget)) {
            d->layout->removeItem(widget);
            emit sizeHintChanged(Qt::PreferredSize);
            break;
        }
    }
}


QGraphicsWidget* TaskArea::Private::findWidget(Task *task)
{
    foreach (QGraphicsWidget *widget, task->associatedWidgets()) {
        if (layout->containsItem(widget)) {
            return widget;
        }
    }

    return 0;
}


}

#include "taskarea.moc"
