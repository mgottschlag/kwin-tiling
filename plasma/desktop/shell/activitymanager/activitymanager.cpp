/*
 *   Copyright (C) 2007 by Ivan Cukic <ivan.cukic+kde@gmail.com>
 *   Copyright (C) 2009 by Ana Cecília Martins <anaceciliamb@gmail.com>
 *   Copyright 2010 Chani Armitage <chani@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library/Lesser General Public License
 *   version 2, or (at your option) any later version, as published by the
 *   Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library/Lesser General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "activitymanager.h"

#include <kaction.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kmenu.h>
#include <kpushbutton.h>
#include <kstandardaction.h>
#include <klineedit.h>

#include <plasma/applet.h>
#include <plasma/corona.h>
#include <plasma/containment.h>
#include <plasma/widgets/toolbutton.h>
#include <plasma/widgets/lineedit.h>

#include "activitylist.h"
#include "filterbar.h"


class ActivityManagerPrivate
{

public:
    ActivityManagerPrivate(ActivityManager *w)
        : q(w),
          containment(0),
          iconSize(16) //FIXME bad!
    {
    }

    void init(Plasma::Location location);
    void containmentDestroyed();
    void setLocation(Plasma::Location location);

    Qt::Orientation orientation;
    Plasma::Location location;
    ActivityManager *q;
    Plasma::ToolButton *close;
    Plasma::Containment *containment;

    /**
     * Widget that lists the applets
     */
    ActivityList *activityList;

    /**
     * Widget that contains the search and categories filters
     */
    FilterBar *filteringWidget;
    QGraphicsLinearLayout *filteringLayout;
    QGraphicsLinearLayout *mainLayout;
    int iconSize;
};

void ActivityManagerPrivate::init(Plasma::Location loc)
{
    location = loc;
    //init widgets
    if (loc == Plasma::LeftEdge || loc == Plasma::RightEdge) {
        orientation = Qt::Vertical;
    } else {
        orientation = Qt::Horizontal;
    }

    mainLayout = new QGraphicsLinearLayout(Qt::Vertical);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    filteringLayout = new QGraphicsLinearLayout(Qt::Horizontal);
    filteringWidget = new FilterBar(orientation, q);

    activityList = new ActivityList(loc);
    close = new Plasma::ToolButton;
    close->setIcon(KIcon("dialog-close"));

    //connect
    QObject::connect(filteringWidget, SIGNAL(searchTermChanged(QString)), activityList, SLOT(searchTermChanged(QString)));
    QObject::connect(filteringWidget, SIGNAL(addWidgetsRequested()), q, SIGNAL(addWidgetsRequested()));
    QObject::connect(close, SIGNAL(clicked()), q, SIGNAL(closeClicked()));

    //adding to layout
    if (orientation == Qt::Horizontal) {
        filteringLayout->addItem(filteringWidget);
    } else {
        mainLayout->addItem(filteringWidget);
    }

    mainLayout->addItem(filteringLayout);
    mainLayout->addItem(activityList);
    activityList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mainLayout->setAlignment(activityList, Qt::AlignTop | Qt::AlignHCenter);

    if (orientation == Qt::Horizontal) {
        filteringLayout->addItem(close);
        filteringLayout->setAlignment(close, Qt::AlignVCenter | Qt::AlignHCenter);
    } else {
        mainLayout->setAlignment(filteringWidget, Qt::AlignTop | Qt::AlignHCenter);
        mainLayout->setStretchFactor(activityList, 10);
        mainLayout->addItem(close);
    }

    //filters & models
    //initRunningApplets();

    q->setLayout(mainLayout);
}

void ActivityManagerPrivate::setLocation(Plasma::Location loc)
{
    Qt::Orientation orient;
    if (loc == Plasma::LeftEdge || loc == Plasma::RightEdge) {
        orient = Qt::Vertical;
    } else {
        orient = Qt::Horizontal;
    }

    if (orientation == orient) {
        return;
    }

    location = loc;
//FIXME bet I could make this more efficient
    orientation = orient;
    filteringWidget->setOrientation(orientation);
    activityList->setLocation(containment->location());
    if (orientation == Qt::Horizontal) {
        mainLayout->removeItem(filteringWidget);
        mainLayout->removeItem(close);
        filteringLayout->addItem(filteringWidget);
        filteringLayout->addItem(close);
        filteringLayout->setAlignment(close, Qt::AlignVCenter | Qt::AlignHCenter);
    } else {
        filteringLayout->removeItem(filteringWidget);
        filteringLayout->removeItem(close);
        mainLayout->insertItem(0, filteringWidget);
        mainLayout->addItem(close);
        mainLayout->setAlignment(filteringWidget, Qt::AlignTop | Qt::AlignHCenter);
        mainLayout->setStretchFactor(activityList, 10);
    }
}

void ActivityManagerPrivate::containmentDestroyed()
{
    containment = 0;
}

//ActivityBar

ActivityManager::ActivityManager(Plasma::Location loc, QGraphicsItem *parent)
        :QGraphicsWidget(parent),
        d(new ActivityManagerPrivate(this))
{
    d->init(loc);
}

ActivityManager::ActivityManager(QGraphicsItem *parent)
        :QGraphicsWidget(parent),
        d(new ActivityManagerPrivate(this))
{
    d->init(Plasma::BottomEdge);
}

ActivityManager::~ActivityManager()
{
     delete d;
}

void ActivityManager::setLocation(Plasma::Location loc)
{
    d->setLocation(loc);
    emit(locationChanged(loc));
}

Plasma::Location ActivityManager::location()
{
    return d->location;
}

void ActivityManager::setIconSize(int size)
{
    d->activityList->setIconSize(size);
    adjustSize();
}

int ActivityManager::iconSize() const
{
    return d->activityList->iconSize();
}

void ActivityManager::setContainment(Plasma::Containment *containment)
{
    kDebug() << "Setting containment to" << containment;
    if (d->containment != containment) {
        if (d->containment) {
            d->containment->disconnect(this);
        }

        d->containment = containment;

        if (d->containment) {
            connect(d->containment, SIGNAL(destroyed(QObject*)), this, SLOT(containmentDestroyed()));
        }
    }
}

void ActivityManager::focusInEvent(QFocusEvent* event)
{
    Q_UNUSED(event);
    qDebug() << "ActivityManager::focusInEvent()";
    QTimer::singleShot(300, d->filteringWidget, SLOT(setFocus())); 
}

#include "activitymanager.moc"
