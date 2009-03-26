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
#include <QtGui/QApplication>
#include <QtGui/QGraphicsLinearLayout>
#include <QtGui/QWidget> // QWIDGETSIZE_MAX

#include <KIcon>
#include <KIconLoader>

#include <Plasma/IconWidget>

#include "../core/manager.h"
#include "../core/task.h"

#include "applet.h"
#include "compactlayout.h"


namespace SystemTray
{


class TaskArea::Private
{
public:
    Private(SystemTray::Applet *h)
        : host(h),
          unhider(0),
          topLayout(new QGraphicsLinearLayout(Qt::Horizontal)),
          firstTasksLayout(new CompactLayout()),
          normalTasksLayout(new CompactLayout()),
          lastTasksLayout(new CompactLayout()),
          showingHidden(false),
          hasHiddenTasks(false),
          hasTasksThatCanHide(false)
    {
    }

    SystemTray::Applet *host;
    Plasma::IconWidget *unhider;
    QGraphicsLinearLayout *topLayout;
    CompactLayout *firstTasksLayout;
    CompactLayout *normalTasksLayout;
    CompactLayout *lastTasksLayout;

    QSet<QString> hiddenTypes;
    bool showingHidden : 1;
    bool hasHiddenTasks : 1;
    bool hasTasksThatCanHide : 1;
};


TaskArea::TaskArea(SystemTray::Applet *parent)
    : QGraphicsWidget(parent),
      d(new Private(parent))
{
    setLayout(d->topLayout);
    d->topLayout->addItem(d->firstTasksLayout);
    d->topLayout->addItem(d->normalTasksLayout);
    d->topLayout->addItem(d->lastTasksLayout);
    d->topLayout->setContentsMargins(0, 0, 0, 0);
}


TaskArea::~TaskArea()
{
    delete d;
}


void TaskArea::setHiddenTypes(const QStringList &hiddenTypes)
{
    d->hiddenTypes = QSet<QString>::fromList(hiddenTypes);
}



void TaskArea::syncTasks(const QList<SystemTray::Task*> &tasks)
{
    d->hasTasksThatCanHide = false;
    d->hasHiddenTasks = false;
    foreach (Task *task, tasks) {
        kDebug() << "checking" << task->name() << d->showingHidden;
        if (d->hiddenTypes.contains(task->typeId())) {
            task->setHidden(task->hidden()|Task::UserHidden);
        } else if (task->hidden()&Task::UserHidden) {
            task->setHidden(task->hidden()^Task::UserHidden);
        }
        addWidgetForTask(task);
    }

    checkUnhideTool();
    d->topLayout->invalidate();
    emit sizeHintChanged(Qt::PreferredSize);
}

void TaskArea::addTask(Task *task)
{
    if (d->hiddenTypes.contains(task->typeId())) {
        task->setHidden(task->hidden()|Task::UserHidden);
    }

    addWidgetForTask(task);
    connect (task, SIGNAL(changed(Task*)), this, SLOT(addWidgetForTask(SystemTray::Task *)));

    checkUnhideTool();
    emit sizeHintChanged(Qt::PreferredSize);
}

void TaskArea::addWidgetForTask(SystemTray::Task *task)
{
    QGraphicsWidget *widget = findWidget(task);
    if (!task->isEmbeddable() && !widget) {
        kDebug() << "task is not embeddable, so FAIL" << task->name();
        return;
    }

    d->hasTasksThatCanHide = d->hasTasksThatCanHide || (task->hidden() != Task::NotHidden);


    if (widget) {
        kDebug() << "widget already exists, trying to reposition it";
        d->firstTasksLayout->removeItem(widget);
        d->normalTasksLayout->removeItem(widget);
        d->lastTasksLayout->removeItem(widget);
    }

    if (!d->showingHidden && task->hidden() != Task::NotHidden) {
        kDebug() << "is a hidden type";
        d->hasHiddenTasks = true;
        if (widget) {
            kDebug() << "just hiding the widget";
            widget->hide();
        }
    } else {
        if (!widget) {
            widget = task->widget(d->host);
            widget->setParent(this);
        }

        if (widget) {
            switch (task->order()) {
            case SystemTray::Task::First:
                d->firstTasksLayout->addItem(widget);
                break;
            case SystemTray::Task::Normal:
                d->normalTasksLayout->addItem(widget);
                break;
            case SystemTray::Task::Last:
                //not really pretty, but for consistency attempts to put the extender expander always in the last position
                if (task->typeId() == "toggle_extender") {
                    d->lastTasksLayout->addItem(widget);
                } else {
                    d->lastTasksLayout->insertItem(0, widget);
                }
                break;
            }
            widget->show();
        }
    }

    //the applet could have to be repainted due to easement change
    QGraphicsWidget *applet = dynamic_cast<QGraphicsWidget *>(parentItem());

    if (applet) {
        applet->update();
    }
}

//TODO: check if is still necessary with 4.5
void TaskArea::checkSizes()
{
    d->topLayout->updateGeometry();

    QSizeF s = d->topLayout->effectiveSizeHint(Qt::PreferredSize);

    setPreferredSize(s);
}

void TaskArea::removeTask(Task *task)
{
    foreach (QGraphicsWidget *widget, task->associatedWidgets()) {
        if (d->normalTasksLayout->containsItem(widget) ||
            d->lastTasksLayout->containsItem(widget) ||
            d->firstTasksLayout->containsItem(widget)) {

            //try to remove from all three layouts, one will succeed
            d->firstTasksLayout->removeItem(widget);
            d->normalTasksLayout->removeItem(widget);
            d->lastTasksLayout->removeItem(widget);
            d->topLayout->invalidate();
            emit sizeHintChanged(Qt::PreferredSize);
            break;
        }
    }
}

int TaskArea::leftEasement() const
{
    if (d->unhider) {
        const int cheat = 6;

        if (d->topLayout->orientation() == Qt::Horizontal) {
            return d->unhider->size().width() / 2 + cheat;
        } else {
            return d->unhider->size().height() / 2 + cheat;
        }
    }

    return 0;
}


int TaskArea::rightEasement() const
{
    if (d->lastTasksLayout->count() > 0) {
        QGraphicsLayoutItem *item = d->lastTasksLayout->itemAt(0);

        if (d->topLayout->orientation() == Qt::Vertical) {
            return size().height() - item->geometry().top() + d->topLayout->spacing();
        } else if (QApplication::layoutDirection() == Qt::RightToLeft) {
            return item->geometry().right() + d->topLayout->spacing();
        } else {
            return size().width() - item->geometry().left() + d->topLayout->spacing();
        }
    } else {
        return 0;
    }
}

bool TaskArea::hasHiddenTasks() const
{
    return d->hasHiddenTasks;
}

void TaskArea::setOrientation(Qt::Orientation o)
{
    d->topLayout->setOrientation(o);

    if (d->unhider) {
        d->unhider->setOrientation(o);
        if (d->topLayout->orientation() == Qt::Horizontal) {
            d->unhider->setMaximumSize(KIconLoader::SizeSmallMedium, QWIDGETSIZE_MAX);
            d->unhider->setMinimumSize(KIconLoader::SizeSmallMedium, KIconLoader::SizeSmall);
            d->unhider->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
        } else {
            d->unhider->setMaximumSize(QWIDGETSIZE_MAX, KIconLoader::SizeSmallMedium);
            d->unhider->setMinimumSize(KIconLoader::SizeSmallMedium, KIconLoader::SizeSmall);
            d->unhider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
        }
    }
    updateUnhideToolIcon();

    /*on the first added "last" task add also a little separator: the size depends from the applet margins,
    in order to make the background of the last items look "balanced"*/
    QGraphicsWidget *applet = dynamic_cast<QGraphicsWidget *>(parentItem());

    if (applet) {
      qreal left, top, right, bottom;
      applet->getContentsMargins(&left, &top, &right, &bottom);

      if (o == Qt::Horizontal) {
          d->topLayout->setSpacing(right);
      } else {
          d->topLayout->setSpacing(bottom);
      }
    }
}

void TaskArea::initUnhideTool()
{
    if (d->unhider) {
        return;
    }

    d->unhider = new Plasma::IconWidget(this);
    updateUnhideToolIcon();

    if (d->topLayout->orientation() == Qt::Horizontal) {
        d->unhider->setMaximumSize(KIconLoader::SizeSmallMedium, QWIDGETSIZE_MAX);
        d->unhider->setMinimumSize(KIconLoader::SizeSmallMedium, KIconLoader::SizeSmall);
        d->unhider->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    } else {
        d->unhider->setMaximumSize(QWIDGETSIZE_MAX, KIconLoader::SizeSmallMedium);
        d->unhider->setMinimumSize(KIconLoader::SizeSmallMedium, KIconLoader::SizeSmall);
        d->unhider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    }

    //FIXME
    //d->topLayout->removeItem(d->taskLayout);
    d->topLayout->insertItem(0, d->unhider);
    //d->topLayout->addItem(d->unhider);
    //d->topLayout->addItem(d->taskLayout);
    connect(d->unhider, SIGNAL(clicked()), this, SLOT(toggleHiddenItems()));

    emit sizeHintChanged(Qt::PreferredSize);
}

void TaskArea::updateUnhideToolIcon()
{
    if (!d->unhider) {
        return;
    }

    if (!d->showingHidden && d->topLayout->orientation() == Qt::Vertical) {
        d->unhider->setSvg("widgets/systemtray", "expander-up");
    } else if(d->showingHidden && d->topLayout->orientation() == Qt::Vertical){
        d->unhider->setSvg("widgets/systemtray", "expander-down");
    }else if (d->showingHidden || QApplication::layoutDirection() == Qt::RightToLeft) {
        d->unhider->setSvg("widgets/systemtray", "expander-right");
    } else {
        d->unhider->setSvg("widgets/systemtray", "expander-left");
    }
}

void TaskArea::toggleHiddenItems()
{
    d->showingHidden = !d->showingHidden;
    updateUnhideToolIcon();
    syncTasks(d->host->manager()->tasks());
    emit sizeHintChanged(Qt::PreferredSize);
}

void TaskArea::checkUnhideTool()
{
    if (d->hasTasksThatCanHide) {
        initUnhideTool();
    } else {
        // hide the show tool
        d->topLayout->removeItem(d->unhider);
        if (d->unhider) {
            d->unhider->deleteLater();
            d->unhider = 0;
        }
    }
}

QGraphicsWidget* TaskArea::findWidget(Task *task)
{
    foreach (QGraphicsWidget *widget, task->associatedWidgets()) {

        if (widget->parent() == this) {
            return widget;
        }
    }

    return 0;
}


}

#include "taskarea.moc"
