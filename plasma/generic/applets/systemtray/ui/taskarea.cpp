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
#include <QtCore/QTimer>
#include <QtGui/QApplication>
#include <QtGui/QGraphicsGridLayout>
#include <QtGui/QGraphicsLinearLayout>
#include <QtGui/QWidget> // QWIDGETSIZE_MAX
#include <QtGui/QGraphicsScene>
#include <QGraphicsSceneMouseEvent>

#include <KIcon>
#include <KIconLoader>

#include <Plasma/Containment>
#include <Plasma/IconWidget>
#include <Plasma/ItemBackground>
#include <Plasma/Label>
#include <Plasma/ToolTipManager>

#include "../core/manager.h"
#include "../core/task.h"

#include "applet.h"
#include "compactlayout.h"
#include "taskarea_p.h"

namespace SystemTray
{

class TaskArea::Private
{
public:
    Private(SystemTray::Applet *h)
        : host(h),
          unhider(0),
          hiddenRelayoutTimer(new QTimer(h)),
          delayedUpdateTimer(new QTimer(h)),
          topLayout(new QGraphicsLinearLayout(Qt::Horizontal)),
          firstTasksLayout(new CompactLayout()),
          normalTasksLayout(new CompactLayout()),
          lastTasksLayout(new CompactLayout()),
          location(Plasma::BottomEdge),
          sizeHintChanged(false)
    {
    }

    bool isTaskProperlyPlaced(Task *task, QGraphicsWidget *widget)
    {
        //kDebug() << "========" << task->name() << "==========";
        //kDebug() << "      " << task->hidden() << Task::NotHidden << task->category() << host->shownCategories().contains(task->category());
        if (task->hidden() == Task::NotHidden && host->shownCategories().contains(task->category())) {
            //kDebug() << "    " << task->order() << firstTasksLayout->containsItem(widget) << lastTasksLayout->containsItem(widget) << normalTasksLayout->containsItem(widget);
            if (task->order() == SystemTray::Task::First && firstTasksLayout->containsItem(widget)) {
                return true;
            } else if (task->order() == SystemTray::Task::Last && lastTasksLayout->containsItem(widget)) {
                return true;
            } else if (task->order() == SystemTray::Task::Normal && normalTasksLayout->containsItem(widget) ) {
                if (taskCategories.contains(task) && taskCategories.value(task) == task->category()) {
                    return true;
                } else {
                    taskCategories[task] = task->category();
                    return false;
                }
            }
        }

        return false;
    }

    SystemTray::Applet *host;
    Plasma::IconWidget *unhider;
    QTimer *hiddenRelayoutTimer;
    QTimer *delayedUpdateTimer;
    QGraphicsLinearLayout *topLayout;
    CompactLayout *firstTasksLayout;
    CompactLayout *normalTasksLayout;
    QHash<Task *, Task::Category> taskCategories;
    QHash<QGraphicsWidget *, SystemTray::Task*> taskForWidget;
    CompactLayout *lastTasksLayout;
    QGraphicsWidget *hiddenTasksWidget;
    QGraphicsGridLayout *hiddenTasksLayout;
    Plasma::Location location;
    Plasma::ItemBackground *itemBackground;
    bool sizeHintChanged;

    QSet<QString> hiddenTypes;
    QSet<QString> alwaysShownTypes;
    QHash<SystemTray::Task*, HiddenTaskLabel *> hiddenTasks;
};


TaskArea::TaskArea(SystemTray::Applet *parent)
    : QGraphicsWidget(parent),
      d(new Private(parent))
{
    d->itemBackground = new Plasma::ItemBackground;
    setLayout(d->topLayout);
    d->topLayout->addItem(d->firstTasksLayout);
    d->topLayout->addItem(d->normalTasksLayout);
    d->topLayout->addItem(d->lastTasksLayout);
    d->topLayout->setContentsMargins(0, 0, 0, 0);
    d->topLayout->setSpacing(5);

    d->hiddenTasksWidget = new QGraphicsWidget(this);
    d->hiddenTasksLayout = new QGraphicsGridLayout(d->hiddenTasksWidget);
    d->hiddenTasksLayout->setHorizontalSpacing(0);

    d->hiddenRelayoutTimer->setSingleShot(true);
    d->hiddenRelayoutTimer->setInterval(50);
    connect(d->hiddenRelayoutTimer, SIGNAL(timeout()), this, SLOT(relayoutHiddenTasks()));

    d->delayedUpdateTimer->setSingleShot(true);
    d->delayedUpdateTimer->setInterval(0);
    connect(d->delayedUpdateTimer, SIGNAL(timeout()), this, SLOT(delayedAppletUpdate()));
}


TaskArea::~TaskArea()
{
    delete d->firstTasksLayout;
    delete d->normalTasksLayout;
    delete d->lastTasksLayout;
    delete d->itemBackground;
    delete d;
}

QGraphicsWidget *TaskArea::hiddenTasksWidget() const
{
    return d->hiddenTasksWidget;
}

void TaskArea::setHiddenTypes(const QStringList &hiddenTypes)
{
    d->hiddenTypes = QSet<QString>::fromList(hiddenTypes);
}

QStringList TaskArea::hiddenTypes() const
{
    return d->hiddenTypes.toList();
}

void TaskArea::setAlwaysShownTypes(const QStringList &alwaysShownTypes)
{
    d->alwaysShownTypes.clear();

    foreach (const QString &type, alwaysShownTypes) {
        if (!d->hiddenTypes.contains(type)) {
            d->alwaysShownTypes.insert(type);
        }
    }
}

QStringList TaskArea::alwaysShownTypes() const
{
    return d->alwaysShownTypes.toList();
}

void TaskArea::syncTasks(const QList<SystemTray::Task*> &tasks)
{
    bool changedPositioning = false;
    foreach (Task *task, tasks) {
        //kDebug() << "checking" << task->name() << task->typeId() << d->alwaysShownTypes;
        changedPositioning = addWidgetForTask(task) || changedPositioning;
    }

    if (checkUnhideTool() || changedPositioning) {
        d->topLayout->invalidate();
        emit sizeHintChanged(Qt::PreferredSize);
    }
}

void TaskArea::addTask(Task *task)
{
    bool changedPositioning = addWidgetForTask(task);
    if (checkUnhideTool() || changedPositioning) {
        d->topLayout->invalidate();
        emit sizeHintChanged(Qt::PreferredSize);
    }
}

void TaskArea::checkVisibility(Task *task)
{
    if (d->hiddenTypes.contains(task->typeId())) {
        task->setHidden(task->hidden() | Task::UserHidden);
    } else if (d->alwaysShownTypes.contains(task->typeId())) {
        task->setHidden(task->hidden() & ~Task::UserHidden);
        task->setHidden(task->hidden() & ~Task::AutoHidden);
    } else if (task->hidden() & Task::UserHidden) {
        task->setHidden(task->hidden() & ~Task::UserHidden);
    } else {
        task->resetHiddenStatus();
    }
}

bool TaskArea::removeFromHiddenArea(SystemTray::Task *task)
{
    if (!d->hiddenTasks.contains(task)) {
        return false;
    }

    QGraphicsWidget *widget = task->widget(d->host, false);
    QGraphicsWidget *taskLabel = d->hiddenTasks.value(task);

    if (widget) {
        for (int i = 0; i < d->hiddenTasksLayout->count(); ++i) {
            if (d->hiddenTasksLayout->itemAt(i) == widget) {
                d->hiddenTasksLayout->removeAt(i);
                break;
            }
        }
    }

    if (taskLabel) {
        disconnect(task, 0, taskLabel, 0);
        for (int i = 0; i < d->hiddenTasksLayout->count(); ++i) {
            if (d->hiddenTasksLayout->itemAt(i) == taskLabel) {
                d->hiddenTasksLayout->removeAt(i);
                break;
            }
        }
        taskLabel->deleteLater();
    }

    checkUnhideTool();
    d->hiddenTasks.remove(task);
    d->hiddenRelayoutTimer->start();
    return true;
}

bool TaskArea::addWidgetForTask(SystemTray::Task *task)
{
    //kDebug() << "adding task" << task->name();
    if (!task->isEmbeddable(d->host)) {
        //kDebug() << "task is not embeddable, so FAIL" << task->name();
        return false;
    }


    checkVisibility(task);
    QGraphicsWidget *widget = task->widget(d->host, false);
    const bool newWidget = !widget;
    if (!widget) {
        widget = task->widget(d->host);
    }

    if (!widget) {
        //kDebug() << "embeddable, but we received no widget?!";
        return false;
    }

    //check if it's not necessary to move the icon
    if (d->isTaskProperlyPlaced(task, widget)) {
        //kDebug() << "widget is properly placed";
        return false;
    }

    //kDebug() << "widget already exists, trying to reposition it";
    d->firstTasksLayout->removeItem(widget);
    d->normalTasksLayout->removeItem(widget);
    d->lastTasksLayout->removeItem(widget);
    if (d->firstTasksLayout->count() == 0) {
        d->topLayout->removeItem(d->firstTasksLayout);
    }

    //If the applet doesn't want to show FDO tasks, remove (not just hide) any of them
    //if the dbus icon has a category that the applet doesn't want to show remove it
    if (!d->host->shownCategories().contains(task->category()) && !qobject_cast<Plasma::Applet *>(widget)) {
        removeFromHiddenArea(task);
        task->abandon(d->host);
        return true;
    }

    d->taskForWidget.insert(widget, task);

    // keep track of the hidden tasks
    // needs to be done because if a task is added multiple times (like when coming out of sleep)
    // it may be autohidden for a while until the final one which will not be hidden
    // therefore we need a way to track the hidden tasks
    // if the task appears in the hidden list, then we know there are hidden tasks
    if (task->hidden() != Task::NotHidden) {
        // hiddent task, so make sure it's handled
        if (!d->hiddenTasks.contains(task)) {
            HiddenTaskLabel *hiddenLabel = new HiddenTaskLabel(widget, task->name(), d->itemBackground, d->host, d->hiddenTasksWidget);
            connect(task, SIGNAL(changed(SystemTray::Task*)), hiddenLabel, SLOT(taskChanged(SystemTray::Task*)));
            d->hiddenTasks.insert(task, hiddenLabel);

            const int row = d->hiddenTasksLayout->rowCount();
            widget->setParentItem(d->hiddenTasksWidget);
            //kDebug() << "putting" << task->name() << widget << "into" << row;
            QFontMetrics fm(font());
            d->hiddenTasksLayout->setRowFixedHeight(row, qMax(24, fm.height()));
            d->hiddenTasksLayout->addItem(widget, row, 0);
            d->hiddenTasksLayout->addItem(hiddenLabel, row, 1);
            adjustHiddenTasksWidget();
            if (!newWidget) {
                d->sizeHintChanged = true;
                d->delayedUpdateTimer->start();
            }

            d->hiddenRelayoutTimer->start();
        }

        widget->show();
        return false;
    }

    // the task is set to be shown
    removeFromHiddenArea(task);
    widget->setParentItem(this);
    //not really pretty, but for consistency attempts to put the notifications applet always in the same position
    if (task->typeId() == "notifications") {
        if (d->firstTasksLayout->count() == 0) {
            d->topLayout->insertItem(0, d->firstTasksLayout);
        }

        d->firstTasksLayout->insertItem(0, widget);
    } else if (task->order() == SystemTray::Task::First) {
        if (d->firstTasksLayout->count() == 0) {
            d->topLayout->insertItem(0, d->firstTasksLayout);
        }

        d->firstTasksLayout->addItem(widget);
    } else if (task->order() == SystemTray::Task::Normal) {
        int insertIndex = -1;
        for (int i = 0; i < d->normalTasksLayout->count(); ++i) {
            QGraphicsWidget *widget = static_cast<QGraphicsWidget *>(d->normalTasksLayout->itemAt(i));
            Task *otherTask = d->taskForWidget.value(widget);

            if (task->category() == Task::UnknownCategory) {
                insertIndex = i;
                break;
            } else if (otherTask && task->category() <= otherTask->category()) {
                insertIndex = i;
                break;
            }
        }

        if (insertIndex == -1) {
            insertIndex = d->normalTasksLayout->count();
        }

        d->normalTasksLayout->insertItem(insertIndex, widget);
    } else {
        d->lastTasksLayout->insertItem(0, widget);
    }

    //the applet could have to be repainted due to easement change
    d->sizeHintChanged = true;
    d->delayedUpdateTimer->start();

    widget->show();

    return true;
}

void TaskArea::delayedAppletUpdate()
{
    d->host->update();
    if (d->sizeHintChanged) {
        emit sizeHintChanged(Qt::PreferredSize);
        d->sizeHintChanged = false;
    }
}

void TaskArea::removeTask(Task *task)
{
    bool sizeChanged = removeFromHiddenArea(task);

    QGraphicsWidget *widget = task->widget(d->host, false);
    if (widget) {
        //try to remove from all three layouts, one will succeed
        d->firstTasksLayout->removeItem(widget);
        if (d->firstTasksLayout->count() == 0) {
            d->topLayout->removeItem(d->firstTasksLayout);
        }
        d->normalTasksLayout->removeItem(widget);
        d->lastTasksLayout->removeItem(widget);
        if (d->lastTasksLayout->count() == 0) {
            d->topLayout->removeItem(d->lastTasksLayout);
        }
        d->taskForWidget.remove(widget);
        d->taskCategories.remove(task);

        d->topLayout->invalidate();
        sizeChanged = true;
    }

    if (sizeChanged) {
        emit sizeHintChanged(Qt::PreferredSize);
    }
}

void TaskArea::relayoutHiddenTasks()
{
    for (int i = 0; i < d->hiddenTasksLayout->count(); ++i) {
         d->hiddenTasksLayout->removeAt(i);
    }

    for (int i = 0; i < d->hiddenTasksLayout->rowCount(); ++i) {
        d->hiddenTasksLayout->setRowFixedHeight(i, 0);
    }

    QHashIterator<SystemTray::Task *, HiddenTaskLabel *> it(d->hiddenTasks);
    QMultiMap<QString, SystemTray::Task *> sorted;
    while (it.hasNext()) {
        it.next();
        sorted.insertMulti(it.value()->text(), it.key());
    }

    QMapIterator<QString, SystemTray::Task *> sortedIt(sorted);
    int row = 0;
    while (sortedIt.hasNext()) {
        sortedIt.next();
        d->hiddenTasksLayout->addItem(sortedIt.value()->widget(d->host), row, 0);
        d->hiddenTasksLayout->addItem(d->hiddenTasks.value(sortedIt.value()), row, 1);
        d->hiddenTasksLayout->setRowFixedHeight(row, 24);
        ++row;
    }

    adjustHiddenTasksWidget();
}

void TaskArea::adjustHiddenTasksWidget()
{
    d->hiddenTasksLayout->invalidate();
    d->hiddenTasksWidget->resize(d->hiddenTasksWidget->effectiveSizeHint(Qt::PreferredSize));
}

int TaskArea::leftEasement() const
{
    if (d->firstTasksLayout->count() > 0) {
        QGraphicsLayoutItem *item = d->firstTasksLayout->itemAt(d->firstTasksLayout->count() - 1);

        if (d->topLayout->orientation() == Qt::Vertical) {
            return item->geometry().bottom() + d->topLayout->spacing()/2;
        } else if (QApplication::layoutDirection() == Qt::RightToLeft) {
            return size().width() - item->geometry().left() + d->topLayout->spacing()/2;
        } else {
            return item->geometry().right() + d->topLayout->spacing()/2;
        }
    } else {
        return 0;
    }
}

int TaskArea::rightEasement() const
{
    if (d->lastTasksLayout->count() > 0) {
        QGraphicsLayoutItem *item = d->lastTasksLayout->itemAt(0);

        if (d->topLayout->orientation() == Qt::Vertical) {
            return size().height() - item->geometry().top() + d->topLayout->spacing()/2;
        } else if (QApplication::layoutDirection() == Qt::RightToLeft) {
            return item->geometry().right() + d->topLayout->spacing()/2;
        } else {
            return size().width() - item->geometry().left() + d->topLayout->spacing()/2;
        }
    } else {
        return 0;
    }
}

void TaskArea::setOrientation(Qt::Orientation o)
{
    d->topLayout->setOrientation(o);

    updateUnhideToolIcon();

    syncTasks(d->host->manager()->tasks());
}

void TaskArea::updateUnhideToolIcon()
{
    if (!d->unhider) {
        return;
    }

    d->unhider->setPreferredIconSize(QSize(16,16));
    if (d->topLayout->orientation() == Qt::Horizontal) {
        d->unhider->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    } else {
        d->unhider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    }

    const bool showing = d->host->isPopupShowing();
    Plasma::ToolTipContent data;
    if (showing) {
        data.setSubText(i18n("Hide icons"));
    } else {
        data.setSubText(i18n("Show hidden icons"));
        d->itemBackground->hide();
    }
    Plasma::ToolTipManager::self()->setContent(d->unhider, data);

    switch(d->location) {
    case Plasma::LeftEdge:
        if (showing) {
            d->unhider->setSvg("widgets/arrows", "left-arrow");
        } else {
            d->unhider->setSvg("widgets/arrows", "right-arrow");
        }
        break;
    case Plasma::RightEdge:
        if (showing) {
            d->unhider->setSvg("widgets/arrows", "right-arrow");
        } else {
            d->unhider->setSvg("widgets/arrows", "left-arrow");
        }
        break;
    case Plasma::TopEdge:
        if (showing) {
            d->unhider->setSvg("widgets/arrows", "up-arrow");
        } else {
            d->unhider->setSvg("widgets/arrows", "down-arrow");
        }
        break;
    case Plasma::BottomEdge:
    default:
        if (showing) {
            d->unhider->setSvg("widgets/arrows", "down-arrow");
        } else {
            d->unhider->setSvg("widgets/arrows", "up-arrow");
        }
    }
}

bool TaskArea::checkUnhideTool()
{
    if (d->hiddenTasks.isEmpty()) {
        if (d->unhider) {
            // hide the show tool
            d->topLayout->removeItem(d->unhider);
            d->unhider->deleteLater();
            d->unhider = 0;
            return true;
        }
    } else if (!d->unhider) {
        d->unhider = new Plasma::IconWidget(this);
        updateUnhideToolIcon();

        d->topLayout->addItem(d->unhider);
        connect(d->unhider, SIGNAL(clicked()), this, SIGNAL(toggleHiddenItems()));
        return true;
    }

    return false;
}

void TaskArea::setLocation(Plasma::Location location)
{
    d->location = location;
    updateUnhideToolIcon();
}

}

#include "taskarea.moc"
#include "taskarea_p.moc"
