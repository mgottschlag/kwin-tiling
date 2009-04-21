/***************************************************************************
 *   task.cpp                                                              *
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

#include "task.h"

#include <QtGui/QGraphicsWidget>


namespace SystemTray
{


class Task::Private
{
public:
    Private()
        : hiddenState(Task::NotHidden),
          order(Task::Normal),
          status(Task::UnknownStatus),
          category(Task::UnknownCategory)
    {
    }

    QList<QGraphicsWidget*> associatedWidgets;
    Task::HideStates hiddenState;
    Task::Order order;
    Task::Status status;
    Task::Category category;
};


Task::Task()
    : d(new Private)
{
}

Task::~Task()
{
    emit destroyed(this);
    delete d;
}


QGraphicsWidget* Task::widget(Plasma::Applet *host)
{
    Q_ASSERT(host);

    QGraphicsWidget *widget;
    widget = createWidget(host);
    d->associatedWidgets.append(widget);
    connect(widget, SIGNAL(destroyed()), this, SLOT(widgetDeleted()));
    connect(this, SIGNAL(destroyed()), widget, SLOT(deleteLater()));

    return widget;
}


void Task::widgetDeleted()
{
    bool wasEmbeddable = isEmbeddable();
    d->associatedWidgets.removeAll(static_cast<QGraphicsWidget*>(sender()));
    if (!wasEmbeddable && isEmbeddable()) {
        emit changed(this);
    }
}


QList<QGraphicsWidget*> Task::associatedWidgets() const
{
    return d->associatedWidgets;
}

bool Task::isHideable() const
{
    return true;
}

void Task::setHidden(HideStates state)
{
    d->hiddenState = state;
}

Task::HideStates Task::hidden() const
{
    return d->hiddenState;
}

Task::Order Task::order() const
{
    return d->order;
}

void Task::setOrder(Order order)
{
    d->order = order;
}

void Task::setCategory(Category category)
{
    if (d->category == category) {
        return;
    }

    d->category = category;
    emit changed(this);
}

Task::Category Task::category() const
{
    return d->category;
}

void Task::setStatus(Status status)
{
    if (d->status == status) {
        return;
    }

    if (status == NeedsAttention) {
        setOrder(Last);
        if (hidden() & AutoHidden) {
            setHidden(hidden() ^ AutoHidden);
        }
    } else {
        if (status == Active && (hidden() & AutoHidden)) {
            setHidden(hidden() ^ AutoHidden);
        } else if (status == Passive) {
            setHidden(hidden() | AutoHidden);
        }

        setOrder(Task::Normal);
    }

    emit changed(this);
}

Task::Status Task::status() const
{
    return d->status;
}

}


#include "task.moc"
