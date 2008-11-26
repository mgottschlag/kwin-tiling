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

#include <Plasma/Applet>
#include <Plasma/IconWidget>

#include "../core/manager.h"
#include "../core/task.h"
#include "compactlayout.h"


namespace SystemTray
{


class TaskArea::Private
{
public:
    Private(Plasma::Applet *h)
        : host(h),
          unhider(0),
          topLayout(new QGraphicsLinearLayout(Qt::Horizontal)),
          taskLayout(new CompactLayout()),
          lastItemCount(0),
          showingHidden(false),
          hasHiddenTasks(false)
    {
    }

    Plasma::Applet *host;
    Plasma::IconWidget *unhider;
    QGraphicsLinearLayout *topLayout;
    CompactLayout *taskLayout;
    QSet<QString> hiddenTypes;
    int lastItemCount;
    bool showingHidden : 1;
    bool hasHiddenTasks : 1;
};


TaskArea::TaskArea(Plasma::Applet *parent)
    : QGraphicsWidget(parent),
      d(new Private(parent))
{
    setLayout(d->topLayout);
    d->topLayout->addItem(d->taskLayout);
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


bool TaskArea::isHiddenType(const QString &typeId) const
{
    return !d->showingHidden && d->hiddenTypes.contains(typeId);
}

void TaskArea::syncTasks(const QList<SystemTray::Task*> &tasks)
{
    d->hasHiddenTasks = false;
    foreach (Task *task, tasks) {
        //kDebug() << "checking" << task->name() << d->showingHidden;
        if (isHiddenType(task->typeId())) {
            d->hasHiddenTasks = true;
            QGraphicsWidget *widget = findWidget(task);
            if (widget) {
                d->taskLayout->removeItem(widget);
                d->topLayout->invalidate();
                //TODO: we shouldn't delete these, just don't show them!
                delete widget;
            }
        } else {
            addWidgetForTask(task);
        }
    }

    checkUnhideTool();
    emit sizeHintChanged(Qt::PreferredSize);
}

void TaskArea::addTask(Task *task)
{
    addWidgetForTask(task);
    checkUnhideTool();
    emit sizeHintChanged(Qt::PreferredSize);
}

void TaskArea::addWidgetForTask(SystemTray::Task *task)
{
    if (task->isEmbeddable() && !findWidget(task)) {
        if (isHiddenType(task->typeId())) {
            d->hasHiddenTasks = true;
        } else {
            switch (task->order()) {
                case SystemTray::Task::First:
                    d->taskLayout->insertItem(0, task->widget(d->host));
                    break;
                case SystemTray::Task::Normal:
                    d->taskLayout->insertItem(d->taskLayout->count() - d->lastItemCount, task->widget(d->host));
                    break;
                case SystemTray::Task::Last:
                    ++d->lastItemCount;
                    d->taskLayout->addItem(task->widget(d->host));
                    break;
            }
        }
    }
}

void TaskArea::checkSizes()
{
    d->taskLayout->updateGeometry();
    d->topLayout->updateGeometry();

    // this bit of braindamage is due to the "quirks" of QGrahics[Linear]Layout
    QSizeF s = d->taskLayout->effectiveSizeHint(Qt::PreferredSize);
    if (d->unhider) {
        if (d->topLayout->orientation() == Qt::Horizontal) {
            s.setWidth(s.width() + d->unhider->size().width());
        } else {
            s.setHeight(s.height() + d->unhider->size().height());
        }
    }

    setPreferredSize(s);
}

void TaskArea::removeTask(Task *task)
{
    foreach (QGraphicsWidget *widget, task->associatedWidgets()) {
        if (d->taskLayout->containsItem(widget)) {
            if (task->order() == Task::Last) {
                --d->lastItemCount;
            }

            d->taskLayout->removeItem(widget);
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
    return d->lastItemCount > 0 ? d->lastItemCount * 24 : 0;
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
            d->unhider->setMaximumSize(26, QWIDGETSIZE_MAX);
            d->unhider->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
        } else {
            d->unhider->setMaximumSize(QWIDGETSIZE_MAX, 26);
            d->unhider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
        }
    }
}

void TaskArea::initUnhideTool()
{
    if (d->unhider) {
        return;
    }

    d->unhider = new Plasma::IconWidget(this);
    d->unhider->setMinimumSize(22, 22);
    updateUnhideToolIcon();

    if (d->topLayout->orientation() == Qt::Horizontal) {
        d->unhider->setMaximumSize(26, QWIDGETSIZE_MAX);
        d->unhider->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    } else {
        d->unhider->setMaximumSize(QWIDGETSIZE_MAX, 26);
        d->unhider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    }

    d->topLayout->removeItem(d->taskLayout);
    //d->topLayout->insertItem(0, d->unhider);
    d->topLayout->addItem(d->unhider);
    d->topLayout->addItem(d->taskLayout);
    connect(d->unhider, SIGNAL(clicked()), this, SLOT(toggleHiddenItems()));

    emit sizeHintChanged(Qt::PreferredSize);
}

void TaskArea::updateUnhideToolIcon()
{
    if (!d->unhider) {
        return;
    }

    if (d->topLayout->orientation() == Qt::Vertical) {
        d->unhider->setSvg("widgets/systemtray", "expander-up");
    } else if (d->showingHidden || QApplication::layoutDirection() == Qt::RightToLeft) {
        d->unhider->setSvg("widgets/systemtray", "expander-right");
    } else {
        d->unhider->setSvg("widgets/systemtray", "expander-left");
    }
}

void TaskArea::toggleHiddenItems()
{
    d->showingHidden = !d->showingHidden;
    updateUnhideToolIcon();
    syncTasks(Manager::self()->tasks());
    emit sizeHintChanged(Qt::PreferredSize);
}

void TaskArea::checkUnhideTool()
{
    if (d->showingHidden || d->hasHiddenTasks) {
        initUnhideTool();
    } else {
        // hide the show tool
        d->topLayout->removeItem(d->unhider);
        delete d->unhider;
        d->unhider = 0;
    }
}

QGraphicsWidget* TaskArea::findWidget(Task *task)
{
    foreach (QGraphicsWidget *widget, task->associatedWidgets()) {
        if (d->taskLayout->containsItem(widget)) {
            return widget;
        }
    }

    return 0;
}


}

#include "taskarea.moc"
