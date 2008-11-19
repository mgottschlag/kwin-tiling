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
#include <QtGui/QToolButton>
#include <QtGui/QGraphicsLinearLayout>

#include <KIcon>

#include <Plasma/Applet>
#include <Plasma/ToolButton>

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
          showingHidden(false),
          hasHiddenTasks(false)
    {
    }

    Plasma::Applet *host;
    Plasma::ToolButton *unhider;
    QGraphicsLinearLayout *topLayout;
    CompactLayout *taskLayout;
    QSet<QString> hiddenTypes;
    bool showingHidden;
    bool hasHiddenTasks;
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
        kDebug() << "checking" << task->name() << d->showingHidden;
        if (isHiddenType(task->typeId())) {
            d->hasHiddenTasks = true;
            QGraphicsWidget *widget = findWidget(task);
            if (widget) {
                d->taskLayout->removeItem(widget);
                d->topLayout->invalidate();
                delete widget;
            }
        } else {
            addWidgetForTask(task);
        }
    }

    checkUnhideTool();
}

void TaskArea::addTask(Task *task)
{
    addWidgetForTask(task);
    checkUnhideTool();
}

void TaskArea::addWidgetForTask(SystemTray::Task *task)
{
    if (task->isEmbeddable() && !findWidget(task)) {
        if (isHiddenType(task->typeId())) {
            d->hasHiddenTasks = true;
        } else {
            d->taskLayout->addItem(task->widget(d->host));
            emit sizeHintChanged(Qt::PreferredSize);
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
            d->taskLayout->removeItem(widget);
            d->topLayout->invalidate();
            emit sizeHintChanged(Qt::PreferredSize);
            break;
        }
    }
}

int TaskArea::easement() const
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

bool TaskArea::hasHiddenTasks() const
{
    return d->hasHiddenTasks;
}

void TaskArea::setOrientation(Qt::Orientation o)
{
    d->topLayout->setOrientation(o);

    if (d->unhider) {
        if (d->topLayout->orientation() == Qt::Horizontal) {
            d->unhider->nativeWidget()->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        } else {
            d->unhider->nativeWidget()->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        }
    }
}

void TaskArea::initUnhideTool()
{
    if (d->unhider) {
        return;
    }

    d->unhider = new Plasma::ToolButton(this);
    d->unhider->setMinimumSize(22, 22);
    updateUnhideToolIcon();

    if (d->topLayout->orientation() == Qt::Horizontal) {
        d->unhider->nativeWidget()->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    } else {
        d->unhider->nativeWidget()->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
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

    if (d->showingHidden || QApplication::layoutDirection() == Qt::RightToLeft) {
        d->unhider->nativeWidget()->setIcon(KIcon("arrow-right"));
    } else {
        d->unhider->nativeWidget()->setIcon(KIcon("arrow-left"));
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
        emit sizeHintChanged(Qt::PreferredSize);
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
