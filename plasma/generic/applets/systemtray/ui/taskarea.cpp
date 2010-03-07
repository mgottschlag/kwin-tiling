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

#include <Plasma/IconWidget>
#include <Plasma/ItemBackground>
#include <Plasma/Label>
#include <Plasma/ToolTipManager>

#include "../core/manager.h"
#include "../core/task.h"

#include "applet.h"
#include "compactlayout.h"

namespace SystemTray
{

struct ItemBackgroundStore
{
    ItemBackgroundStore() : useCount(0) {}
    int useCount;
    Plasma::ItemBackground itemBackground;
};

K_GLOBAL_STATIC(ItemBackgroundStore, s_bgStore)


class HiddenTaskLabel : public Plasma::Label
{
public:
    HiddenTaskLabel(QGraphicsWidget *taskIcon, const QString &label, QGraphicsWidget *parent = 0)
        : Plasma::Label(parent),
          m_taskIcon(taskIcon)
    {
        taskIcon->setMaximumHeight(48);
        taskIcon->setMinimumHeight(24);
        taskIcon->setMinimumWidth(24);

        setContentsMargins(0, 0, 0, 0);

        setWordWrap(false);
        setText(label);

        if (s_bgStore->useCount == 0) {
            scene()->addItem(&s_bgStore->itemBackground);
            s_bgStore->itemBackground.show();
            takeItemBackgroundOwnership();
        }
        s_bgStore->useCount++;
    }

    ~HiddenTaskLabel()
    {
        s_bgStore->useCount--;
    }

protected:
    void takeItemBackgroundOwnership()
    {
        if (m_taskIcon) {
            QRectF totalRect = geometry().united(m_taskIcon.data()->geometry());
            totalRect.moveTopLeft(QPoint(0,0));
            totalRect = m_taskIcon.data()->mapToScene(totalRect).boundingRect();
            qreal left, top, right, bottom;
            s_bgStore->itemBackground.getContentsMargins(&left, &top, &right, &bottom);
            totalRect.adjust(-left/2, -top/2, right/2, bottom/2);
            s_bgStore->itemBackground.setTarget(totalRect);
        }
    }

    template<class T> void forwardEvent(T *event)
    {
        if (m_taskIcon) {
            QGraphicsItem *item = scene()->itemAt(m_taskIcon.data()->scenePos() + QPoint(3, 3));
            if (item) {
                event->setPos(item->boundingRect().topLeft());
                scene()->sendEvent(item, event);
            }
        }
    }

    void mousePressEvent(QGraphicsSceneMouseEvent *event)
    {
        forwardEvent(event);
    }

    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
    {
        forwardEvent(event);
    }

    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
    {
        forwardEvent(event);
    }

    void wheelEvent(QGraphicsSceneWheelEvent *event)
    {
        forwardEvent(event);
    }

    void hoverEnterEvent ( QGraphicsSceneHoverEvent * event )
    {
        takeItemBackgroundOwnership();
        forwardEvent(event);
    }

    void hoverLeaveEvent ( QGraphicsSceneHoverEvent * event )
    {
        forwardEvent(event);
    }

    void hoverMoveEvent ( QGraphicsSceneHoverEvent * event)
    {
        forwardEvent(event);
    }

private:
    QWeakPointer<QGraphicsWidget> m_taskIcon;
};


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
          location(Plasma::BottomEdge),
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
    QGraphicsWidget *hiddenTasksWidget;
    QGraphicsGridLayout *hiddenTasksLayout;
    Plasma::Location location;

    QSet<QString> hiddenTypes;
    QSet<QString> alwaysShownTypes;
    QHash<SystemTray::Task*, HiddenTaskLabel *> hiddenTasks;
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

    d->hiddenTasksWidget = new QGraphicsWidget(this);
    d->hiddenTasksLayout = new QGraphicsGridLayout(d->hiddenTasksWidget);
}


TaskArea::~TaskArea()
{
    delete d->firstTasksLayout;
    delete d->normalTasksLayout;
    delete d->lastTasksLayout;
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
    //TODO: this is completely brute force; we shouldn't be redoing the
    //      layout or re-adding widgets unless there's actual change imho
    d->hasTasksThatCanHide = false;
    d->hasHiddenTasks = false;
    foreach (Task *task, tasks) {
        //kDebug() << "checking" << task->name() << d->showingHidden;
        if (d->hiddenTypes.contains(task->typeId())) {
            task->setHidden(task->hidden()|Task::UserHidden);
            d->hasHiddenTasks = true;
        } else if (d->alwaysShownTypes.contains(task->typeId())) {
            task->setHidden(task->hidden() & ~Task::UserHidden);
            task->setHidden(task->hidden() & ~Task::AutoHidden);
        } else if (task->hidden() & Task::UserHidden) {
            task->setHidden(task->hidden() & ~Task::UserHidden);
            d->hasHiddenTasks = true;
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
        task->setHidden(task->hidden() | Task::UserHidden);
    }

    addWidgetForTask(task);

    checkUnhideTool();
    emit sizeHintChanged(Qt::PreferredSize);
}

void TaskArea::addWidgetForTask(SystemTray::Task *task)
{
    if (!task->isEmbeddable(d->host)) {
        //kDebug() << "task is not embeddable, so FAIL" << task->name();
        return;
    }

    QGraphicsWidget *widget = task->widget(d->host);

    if (!widget) {
        //kDebug() << "embeddable, but we received no widget?!";
        return;
    }

    //check if it's not necessary to move the icon
    if (task->hidden() == Task::NotHidden &&
        d->host->shownCategories().contains(task->category()) &&
        ((d->firstTasksLayout->containsItem(widget) && task->order() == SystemTray::Task::First) ||
         (d->normalTasksLayout->containsItem(widget) && task->order() == SystemTray::Task::Normal) ||
         (d->lastTasksLayout->containsItem(widget) && task->order() == SystemTray::Task::Last))) {
        return;
    }

    if (widget) {
        //kDebug() << "widget already exists, trying to reposition it";
        d->firstTasksLayout->removeItem(widget);
        d->normalTasksLayout->removeItem(widget);
        d->lastTasksLayout->removeItem(widget);
        if (d->firstTasksLayout->count() == 0) {
            d->topLayout->removeItem(d->firstTasksLayout);
        }
    }

    //If the applet doesn't want to show FDO tasks, remove (not just hide) any of them
    //if the dbus icon has a category that the applet doesn't want to show remove it
    if (!d->host->shownCategories().contains(task->category())) {
        if (widget) {
            widget->deleteLater();
        }
        return;
    }

    if (!widget) {
        widget = task->widget(d->host);
    }

    // keep track of the hidden tasks
    // needs to be done because if a tasks is added multiple times (like when coming out of sleep)
    // it may be autohidden for a while until the final one which will not be hidden
    // therefore we need a way to track the hidden tasks
    // if the task appears in the hidden list, then we know there are hidden tasks
    if (task->hidden() == Task::NotHidden) {
        if (d->hiddenTasks.contains(task)) {
            widget->setParentItem(d->host);
            d->hiddenTasks.value(task)->deleteLater();
            d->hiddenTasks.remove(task);
        }
    } else if (!d->hiddenTasks.contains(task)) {
        HiddenTaskLabel *hiddenWidget = new HiddenTaskLabel(widget, task->name(), d->hiddenTasksWidget);
        d->hiddenTasks.insert(task, hiddenWidget);

        if (widget) {
            const int row = d->hiddenTasksLayout->rowCount();
            kDebug() << "putting" << task->name() << "into" << row;
            d->hiddenTasksLayout->setRowMinimumHeight(row, 24);
            d->hiddenTasksLayout->addItem(widget, row, 0);
            d->hiddenTasksLayout->addItem(d->hiddenTasks.value(task), row, 1);
        }

    }

    d->hasTasksThatCanHide = !d->hiddenTasks.isEmpty();

    if (widget) {
        if (task->hidden() == Task::NotHidden) {
            for (int i = 0; i < d->hiddenTasksLayout->count(); ++i) {
                if (d->hiddenTasksLayout->itemAt(i) == widget) {
                    d->hiddenTasksLayout->removeAt(i);
                    break;
                }
            }

            switch (task->order()) {
                case SystemTray::Task::First:
                if (d->firstTasksLayout->count() == 0) {
                    d->topLayout->addItem(d->firstTasksLayout);
                }
                d->firstTasksLayout->addItem(widget);
                break;
            case SystemTray::Task::Normal:
                d->normalTasksLayout->addItem(widget);
                break;
            case SystemTray::Task::Last:
                //not really pretty, but for consistency attempts to put the extender expander always in the last position
                if (task->typeId() == "notifications") {
                    d->lastTasksLayout->addItem(widget);
                } else {
                    d->lastTasksLayout->insertItem(0, widget);
                }
                break;
            }
        }

        widget->show();
    }

    //the applet could have to be repainted due to easement change
    QGraphicsWidget *applet = dynamic_cast<QGraphicsWidget *>(parentItem());

    if (applet) {
        applet->update();
    }
}

void TaskArea::removeTask(Task *task)
{
    d->hiddenTasks.remove(task);
    d->hasTasksThatCanHide = !d->hiddenTasks.isEmpty();

    QGraphicsWidget *widget = task->widget(d->host, false);

    if (widget) {
        //try to remove from all three layouts, one will succeed
        d->firstTasksLayout->removeItem(widget);
        d->normalTasksLayout->removeItem(widget);
        d->lastTasksLayout->removeItem(widget);
    }
    relayout();
}

void TaskArea::relayout()
{
    d->topLayout->invalidate();
    emit sizeHintChanged(Qt::PreferredSize);
}

int TaskArea::leftEasement() const
{
    if (d->unhider) {
        if (d->topLayout->orientation() == Qt::Horizontal) {
            return d->unhider->size().width();
        } else {
            return d->unhider->size().height();
        }
    }

    return 0;
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
            d->unhider->setMinimumSize(KIconLoader::SizeSmall, KIconLoader::SizeSmall);
            d->unhider->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
        } else {
            d->unhider->setMaximumSize(QWIDGETSIZE_MAX, KIconLoader::SizeSmallMedium);
            d->unhider->setMinimumSize(KIconLoader::SizeSmall, KIconLoader::SizeSmall);
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
            d->topLayout->setSpacing(right*2);
        } else {
            d->topLayout->setSpacing(bottom*2);
        }
    }
    syncTasks(d->host->manager()->tasks());
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
        d->unhider->setMinimumSize(KIconLoader::SizeSmall, KIconLoader::SizeSmall);
        d->unhider->setPreferredSize(KIconLoader::SizeSmallMedium, KIconLoader::SizeSmallMedium);
        d->unhider->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    } else {
        d->unhider->setMaximumSize(QWIDGETSIZE_MAX, KIconLoader::SizeSmallMedium);
        d->unhider->setMinimumSize(KIconLoader::SizeSmall, KIconLoader::SizeSmall);
        d->unhider->setPreferredSize(KIconLoader::SizeSmallMedium, KIconLoader::SizeSmallMedium);
        d->unhider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    }

    d->topLayout->insertItem(0, d->unhider);
    connect(d->unhider, SIGNAL(clicked()), this, SIGNAL(toggleHiddenItems()));

    emit sizeHintChanged(Qt::PreferredSize);
}

void TaskArea::updateUnhideToolIcon()
{
    if (!d->unhider) {
        return;
    }

    Plasma::ToolTipContent data;
    if (d->showingHidden) {
        data.setSubText(i18n("Hide icons"));
    } else {
        data.setSubText(i18n("Show hidden icons"));
    }
    Plasma::ToolTipManager::self()->setContent(d->unhider, data);

    switch(d->location) {
    case Plasma::LeftEdge:
        if (d->showingHidden) {
            d->unhider->setSvg("widgets/systemtray", "expander-left");
        } else {
            d->unhider->setSvg("widgets/systemtray", "expander-right");
        }
        break;
    case Plasma::RightEdge:
        if (d->showingHidden) {
            d->unhider->setSvg("widgets/systemtray", "expander-right");
        } else {
            d->unhider->setSvg("widgets/systemtray", "expander-left");
        }
        break;
    case Plasma::TopEdge:
        if (d->showingHidden) {
            d->unhider->setSvg("widgets/systemtray", "expander-up");
        } else {
            d->unhider->setSvg("widgets/systemtray", "expander-down");
        }
        break;
    case Plasma::BottomEdge:
    default:
        if (d->showingHidden) {
            d->unhider->setSvg("widgets/systemtray", "expander-down");
        } else {
            d->unhider->setSvg("widgets/systemtray", "expander-up");
        }
    }
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

void TaskArea::setShowHiddenItems(bool show)
{
    d->showingHidden = show;
    updateUnhideToolIcon();
}

void TaskArea::setLocation(Plasma::Location location)
{
    d->location = location;
    updateUnhideToolIcon();
}

}

#include "taskarea.moc"
