/***************************************************************************
*   Copyright (C) 2007 by Robert Knight <robertknight@gmail.com>          *
*   Copyright (C) 2008 by Alexis Ménard <darktears31@gmail.com>           *
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

// Own
#include "taskgroupitem.h"
#include "layoutwidget.h"
#include "windowtaskitem.h"
#include "tasksmenu.h"

// Qt
#include <QGraphicsSceneContextMenuEvent>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsView>
#include <QTimer>
#include <QApplication>
#include <QGraphicsLinearLayout>
#include <QInputDialog>

// KDE
#include <KAuthorized>
#include <KDebug>
#include <KIcon>
#include <KLocalizedString>
#include <KGlobalSettings>
#include <KIconLoader>

#include <taskmanager/taskactions.h>
#include <taskmanager/taskmanager.h>
#include <taskmanager/taskgroup.h>
#include <taskmanager/abstractgroupingstrategy.h>

#include <Plasma/Theme>
#include <Plasma/FrameSvg>
#include <Plasma/ToolTipManager>
#include <Plasma/Corona>
#include <Plasma/Containment>

#include "tasks.h"
#include "layoutwidget.h"

TaskGroupItem::TaskGroupItem(QGraphicsWidget *parent, Tasks *applet, const bool showTooltip)
    : AbstractTaskItem(parent, applet, showTooltip),
      m_group(0),
      m_expandedLayout(0),
      m_popupMenuTimer(0),
      m_lastActivated(-1),
      m_activeTaskIndex(0),
      m_maximumRows(1),
      m_forceRows(false),
      m_splitPosition(0),
      m_parentSplitGroup(0),
      m_childSplitGroup(0)
{
    setAcceptDrops(true);
}


bool TaskGroupItem::isSplit()
{
    return m_childSplitGroup != 0;
}

void TaskGroupItem::setSplitGroup(TaskGroup *group)
{
    m_group = group;
    m_parentSplitGroup = dynamic_cast<TaskGroupItem*>(parentWidget());
    if (!m_parentSplitGroup) {
        kDebug() << "no parentSplit Group";
        return;
    }
    expand();
}

//FIXME verify if this really works correctly
void TaskGroupItem::unsplitGroup()
{
    kDebug();
    if (!m_childSplitGroup) {
        return;
    }
    m_childSplitGroup->deleteLater();
    m_childSplitGroup = 0;
    m_splitPosition = 0;
    reload();
}


TaskGroupItem * TaskGroupItem::splitGroup()
{
    return m_childSplitGroup;
}

void TaskGroupItem::setSplitIndex(int position)
{
    //kDebug() << position;
    Q_ASSERT(m_expandedLayout);
    Q_ASSERT(m_parentSplitGroup);

    for (int i = position ; i < m_parentSplitGroup->group()->members().size() ; i++) {
        //kDebug() << "add item to childSplitGroup" << i;
        AbstractGroupableItem *item = m_parentSplitGroup->group()->members().at(i);
        if (!m_groupMembers.contains(item)) {
            m_groupMembers.insert(item, m_parentSplitGroup->abstractItem(item));
        }
        m_expandedLayout->addTaskItem(abstractItem(item));
    }
    m_splitPosition = position;
}

TaskGroupItem * TaskGroupItem::splitGroup(int newSplitPosition)
{
    //kDebug() << "split position" << newSplitPosition;
    Q_ASSERT(m_expandedLayout);

    //remove all items which move to the splitgroup from the expandedLayout
    for (int i = newSplitPosition ; i < m_groupMembers.size() ; i++) {
        AbstractGroupableItem *item = group()->members().at(i);
        m_expandedLayout->removeTaskItem(abstractItem(item));
        //kDebug() << "remove from parentSplitGroup" << i;
    }
    //add items which arent in the splitgroup anymore and should be displayed again
    if (m_splitPosition) { //if 0 is the init value and shouldn't happen otherwise
        for (int i = m_splitPosition ; i < newSplitPosition ; i++) {
            AbstractGroupableItem *item = group()->members().at(i);
            m_expandedLayout->addTaskItem(abstractItem(item));
            //kDebug() << "add Item to parentSplitGroup" << i;
        }
    }

    if (!m_childSplitGroup) {
        //kDebug() << "Normal scene " << scene();
        m_childSplitGroup = new  TaskGroupItem(this, m_applet, true);
        m_childSplitGroup->setSplitGroup(m_group);
    }

    m_childSplitGroup->setSplitIndex(newSplitPosition);
    m_splitPosition = newSplitPosition;
    return m_childSplitGroup;
}

//scroll through items on every click
void TaskGroupItem::activate()
{
    /*m_activeTaskIndex++;
    if (m_activeTaskIndex >= m_groupMembers.size()) {
    m_activeTaskIndex = 0;
    }

    //    kDebug() << "Wheel event m_activeTaskIndex: " << m_activeTaskIndex << " of " << numberOfItems();
    AbstractTaskItem *item = m_groupMembers.at(m_activeTaskIndex);

    if (item) {
    item->activate();
    }//TODO: exclude group items? */

}

void TaskGroupItem::close()
{
//  m_applet->removeGroupTask(m_group);
    m_group = 0;
}


void TaskGroupItem::updateTask(::TaskManager::TaskChanges changes)
{
    Q_ASSERT(m_group);

    bool needsUpdate = false;
    // task flags
    TaskFlags flags = m_flags;
    if (m_group->isActive()) {
        flags |= TaskHasFocus;
        emit activated(this);
    } else {
        flags &= ~TaskHasFocus;
    }

    if (m_group->demandsAttention()) {
        flags |= TaskWantsAttention;
    } else {
        flags &= ~TaskWantsAttention;
    }

    if (m_group->isMinimized()) {
        flags |= TaskIsMinimized;
    } else {
        flags &= ~TaskIsMinimized;
    }

    if (flags != m_flags) {
        needsUpdate = true;
        setTaskFlags(flags);
    }

    // basic title and icon
    if (changes & TaskManager::IconChanged) {
        needsUpdate = true;
        setIcon(m_group->icon());
    }

    if (changes & TaskManager::NameChanged) {
        needsUpdate = true;
        setText(m_group->name());
    }

    if (m_showingTooltip &&
        (changes & TaskManager::IconChanged ||
        changes & TaskManager::NameChanged ||
        changes & TaskManager::DesktopChanged)) {
        updateToolTip();
    }

    if (needsUpdate) {
        //redraw
        queueUpdate();
    }
}

void TaskGroupItem::updateToolTip()
{
    if (!m_group) {
        return;
    }

    Plasma::ToolTipContent data(m_group->name(),
                                i18nc("Which virtual desktop a window is currently on", "On %1",
                                    KWindowSystem::desktopName(m_group->desktop())));
//    data.image = m_group->icon().pixmap(QSize::small);
//    data.windowToPreview = m_task->window();

    Plasma::ToolTipManager::self()->setContent(this, data);
}


void TaskGroupItem::reload()
{
    kDebug();
    QList <AbstractItemPtr> itemsToRemove = m_groupMembers.keys();

    foreach (AbstractItemPtr item, group()->members()) {
        if (!item) {
            kDebug() << "invalid Item";
            continue;
        }
        itemsToRemove.removeAll(item);
        itemAdded(item);

        if (item->isGroupItem()) {
            TaskGroupItem *group = qobject_cast<TaskGroupItem *>(abstractItem(item));
            if (group) {
                group->reload();
            }
        }
    }
    foreach (AbstractItemPtr item, itemsToRemove) { //remove unused items
        if (!item) {
            kDebug() << "invalid Item";
            continue;
        }
        itemRemoved(item);
    }
}

void TaskGroupItem::setGroup(TaskManager::GroupPtr group)
{
    //kDebug();
    m_group = group;
    m_abstractItem = qobject_cast<AbstractItemPtr>(group);
    if (!m_abstractItem) {
        kDebug() << "error";
    }

    connect(m_group, SIGNAL(itemRemoved(AbstractItemPtr)), this, SLOT(itemRemoved(AbstractItemPtr)));
    connect(m_group, SIGNAL(itemAdded(AbstractItemPtr)), this, SLOT(itemAdded(AbstractItemPtr)));

    //connect(m_group, SIGNAL(destroyed()), this, SLOT(close()));

    connect(m_group, SIGNAL(changed(::TaskManager::TaskChanges)),
            this, SLOT(updateTask(::TaskManager::TaskChanges)));

    connect(m_group, SIGNAL(itemPositionChanged(AbstractItemPtr)), this, SLOT(itemPositionChanged(AbstractItemPtr)));
    connect(m_group, SIGNAL(groupEditRequest()), this, SLOT(editGroup()));

    //Add already existing items
    reload();
    updateTask(::TaskManager::EverythingChanged);

    //kDebug() << "Task added, isActive = " << task->isActive();
}

TaskManager::GroupPtr TaskGroupItem::group() const
{
    return m_group;
}

void TaskGroupItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *e)
{
    //kDebug();
    if (!KAuthorized::authorizeKAction("kwin_rmb") || !m_group) {
        QGraphicsWidget::contextMenuEvent(e);
        return;
    }
    Q_ASSERT(m_applet);
    //we are the master group item
    if (m_applet == parentWidget()) {
        e->ignore();
        return;
    }

    QAction *a;

    if (m_expandedLayout) {
        a = new QAction(i18n("Collapse Group"), this);
        connect(a, SIGNAL(triggered()), this, SLOT(collapse()));
    } else {
        a = new QAction(i18n("Expand Group"), this);
        connect(a, SIGNAL(triggered()), this, SLOT(expand()));
    }

    QList <QAction*> actionList;
    actionList.append(a);

    TaskManager::BasicMenu menu(qobject_cast<QWidget*>(this), m_group, &m_applet->groupManager(), actionList);
    menu.adjustSize();

    Q_ASSERT(m_applet->containment());
    Q_ASSERT(m_applet->containment()->corona());
    menu.exec(m_applet->containment()->corona()->popupPosition(this, menu.size()));
}

QList<AbstractTaskItem*> TaskGroupItem::memberList() const
{
    return m_groupMembers.values();
}

AbstractTaskItem *TaskGroupItem::createAbstractItem(TaskManager::AbstractItemPtr groupableItem)
{
    //kDebug() << "item to create" << groupableItem << endl;
    AbstractTaskItem *item = 0;

    if (m_groupMembers.contains(groupableItem)) {
        //kDebug() << "existing item found";
        return m_groupMembers.value(groupableItem);
    }

    if (groupableItem->isGroupItem()) {
        TaskGroupItem *groupItem = new TaskGroupItem(this, m_applet, m_applet->showTooltip());
        groupItem->setGroup(static_cast<TaskManager::TaskGroup*>(groupableItem));
        item = groupItem;
    } else { //it's a window task
        WindowTaskItem *windowItem = new WindowTaskItem(this, m_applet, m_applet->showTooltip());
        windowItem->setTask(static_cast<TaskManager::TaskItem*>(groupableItem));
        item = windowItem;
    }

    if (!item) {
        kDebug() << "invalid Item";
        return 0;
    }

    return item;
}



void TaskGroupItem::itemAdded(TaskManager::AbstractItemPtr groupableItem)
{
    //kDebug();
    if (!m_applet) {
        kDebug() << "No applet";
        return;
    }

    //returns the corresponding item or creates a new one
    AbstractTaskItem *item = createAbstractItem(groupableItem);

    if (!item) {
        kDebug() << "invalid Item";
        return;
    }

    m_groupMembers[groupableItem] = item;
    item->setParentItem(this);

    if (collapsed()) {
        item->hide();
    } else if (isSplit()) {
        splitGroup(m_splitPosition);
        //emit changed();
        //m_childSplitGroup->reload();
    } else {
        m_expandedLayout->addTaskItem(item);
    }

    if (item->isActive()) {
        //kDebug() << "item is Active" ;
        m_activeTaskIndex = m_group->members().indexOf(groupableItem);
    } else if (m_group->members().size() == 1) {
        m_activeTaskIndex = 0;
    }

    connect(item, SIGNAL(activated(AbstractTaskItem*)),
            this, SLOT(updateActive(AbstractTaskItem*)));

}

void TaskGroupItem::itemRemoved(TaskManager::AbstractItemPtr groupableItem)
{
    //kDebug();
    if (!m_applet) {
        kDebug() << "No Applet";
        return;
    }
    AbstractTaskItem *item = m_groupMembers.take(groupableItem);;

    if (!item) {
        kDebug() << "Item not found";
        return;
    }

    disconnect(item, 0, 0, 0);

    if (m_expandedLayout) {
        m_expandedLayout->removeTaskItem(item);
    }

    item->close();
    item->deleteLater();
}


bool TaskGroupItem::isWindowItem() const
{
    return false;
}

bool TaskGroupItem::isActive() const
{
    kDebug() << "Not Implemented";
    return false;
}

void TaskGroupItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{ //TODO add delay so we can still drag group items
    if (event->buttons() & Qt::LeftButton) {
        if (m_applet->groupManager().sortingStrategy() == TaskManager::GroupManager::ManualSorting || m_applet->groupManager().groupingStrategy() == TaskManager::GroupManager::ManualGrouping) {
            if (!m_popupMenuTimer) {
                m_popupMenuTimer = new QTimer(this);
                m_popupMenuTimer->setSingleShot(true);
                m_popupMenuTimer->setInterval(300);
                connect(m_popupMenuTimer, SIGNAL(timeout()), this, SLOT(popupMenu()));
            }
            m_popupMenuTimer->start(300);
        } else {
            popupMenu();
        }
    }
    event->accept();

}

void TaskGroupItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::MidButton) {
        expand();
    }
    if (m_popupMenuTimer) {
        m_popupMenuTimer->stop();
    }
    AbstractTaskItem::mouseReleaseEvent(event);
}

void TaskGroupItem::popupMenu()
{
    if (!m_expandedLayout) {
        TaskManager::TasksMenu menu(qobject_cast<QWidget*>(this), m_group,  &m_applet->groupManager(), m_applet);
        menu.adjustSize();
        Q_ASSERT(m_applet->containment());
        Q_ASSERT(m_applet->containment()->corona());
        menu.exec(m_applet->containment()->corona()->popupPosition(this, menu.size()));
    }
}

void TaskGroupItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    //kDebug();
    if (m_popupMenuTimer) {
        m_popupMenuTimer->stop();
    } //Wait a bit before starting drag
    AbstractTaskItem::mouseMoveEvent(event);
}

void TaskGroupItem::expand()
{
    if (m_expandedLayout) {
        //kDebug() << "already expanded";
        return;
    }

    //kDebug();
    m_expandedLayout = new LayoutWidget(this, m_applet);
    m_expandedLayout->setMaximumRows(m_maximumRows);
    m_expandedLayout->setForceRows(m_forceRows);

    //setLayout(m_expandedLayout);

    connect(m_applet, SIGNAL(constraintsChanged(Plasma::Constraints)), m_expandedLayout, SLOT(constraintsChanged(Plasma::Constraints)));
    connect(m_expandedLayout, SIGNAL(sizeHintChanged(Qt::SizeHint)), this, SLOT(updatePreferredSize()));
    updatePreferredSize();
    emit changed();
    //kDebug() << "expanded";

}

LayoutWidget *TaskGroupItem::layoutWidget()
{
    return m_expandedLayout;
}

void TaskGroupItem::collapse()
{
    //kDebug() << (int)this;
    if (!m_expandedLayout) {
        //kDebug() << "already collapsed";
        return;
    }

    if (m_parentSplitGroup) {
        m_parentSplitGroup->collapse();
    }

    //kDebug();
    unsplitGroup();
    foreach (AbstractTaskItem *member, m_groupMembers) {
        m_expandedLayout->removeTaskItem(member);
    }
    setLayout(0);
    //kDebug();
    //delete m_expandedLayout;
    m_expandedLayout = 0;
    updatePreferredSize();
    //kDebug();
    emit changed();
}

bool TaskGroupItem::collapsed() const
{
    return m_expandedLayout == 0;
}

void TaskGroupItem::updatePreferredSize()
{
    if (layout()) {
        setPreferredSize(layout()->preferredSize());
        layout()->invalidate();
        //kDebug() << "expanded group" << layout()->preferredSize();
    } else {
        //FIXME: copypaste from abstracttaskitem: to be done better with proper sizeHint()
        setPreferredSize(basicPreferredSize());
    }
    //kDebug() << preferredSize();
    emit sizeHintChanged(Qt::PreferredSize);
}



AbstractTaskItem *TaskGroupItem::directMember(AbstractTaskItem *item)
{
    Q_ASSERT(item);
    Q_ASSERT(m_group);
    TaskManager::AbstractItemPtr directMember = m_group->directMember(item->abstractItem());
    if (!directMember) {
        kDebug() << "Error" << item->abstractItem();
    }
    return abstractItem(directMember);
}

void TaskGroupItem::paint(QPainter *painter,
                            const QStyleOptionGraphicsItem *option,
                            QWidget *widget)
{
    if (!m_expandedLayout) {
        AbstractTaskItem::paint(painter,option,widget);
    }/* else {
        if (m_group) {
            //painter->fillRect(geometry(), m_group->color());
        }
    }*/

    //kDebug() << "painter()";
    //painter->setBrush(QBrush(background));
}


// TODO provide a way to edit all group properties
void TaskGroupItem::editGroup()
{
    //it could look like the popup of the device notifier or the calendar of the clock..
    //kDebug();
    Q_ASSERT(m_group);
    Q_ASSERT(m_applet);
    if (m_applet->groupManager().taskGrouper()->editableGroupProperties() & TaskManager::AbstractGroupingStrategy::Name) {
        bool ok;
        QString text = QInputDialog::getText(qobject_cast<QWidget*>(this), tr("Edit Group"),
                                            tr("New Group Name: "), QLineEdit::Normal,
                                            m_group->name(), &ok);
        if (ok && !text.isEmpty()) {
            m_group->setName(text);
        }
    }
}


void  TaskGroupItem::itemPositionChanged(AbstractItemPtr item)
{
    //kDebug();
    if (!m_expandedLayout) {
        return;
    }

    Q_ASSERT(item);

    if (item->isGroupItem()) {
        TaskGroupItem *groupItem = static_cast<TaskGroupItem*>(abstractItem(item));
        if (groupItem) {
            groupItem->unsplitGroup();
        }
    }

    AbstractTaskItem *taskItem = abstractItem(item);

    m_expandedLayout->removeTaskItem(taskItem);
    m_expandedLayout->insert(m_group->members().indexOf(item), taskItem);
}


void TaskGroupItem::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    //kDebug()<<"Drag enter";
    if (event->mimeData()->hasFormat(TaskManager::Task::mimetype()) && !m_expandedLayout) {
        event->acceptProposedAction();
        //kDebug()<<"Drag enter accepted";
    } else {
        event->accept();
        if (!m_popupMenuTimer) {
            m_popupMenuTimer = new QTimer(this);
            m_popupMenuTimer->setSingleShot(true);
            m_popupMenuTimer->setInterval(300);
            connect(m_popupMenuTimer, SIGNAL(timeout()), this, SLOT(popupMenu()));
        }
        m_popupMenuTimer->start(300);
    }
}

AbstractTaskItem *TaskGroupItem::taskItemForWId(WId id)
{
    QHashIterator<AbstractItemPtr, AbstractTaskItem*> it(m_groupMembers);

    while (it.hasNext()) {
        it.next();
        AbstractTaskItem *item = it.value();
        TaskGroupItem *group = qobject_cast<TaskGroupItem*>(item);

        if (group) {
            item = group->taskItemForWId(id);
            if (item) {
                return item;
            }
        } else {
            TaskManager::TaskItem *task = qobject_cast<TaskManager::TaskItem*>(it.key());
            if (task && task->task() && task->task()->window() == id) {
                return item;
            }
        }
    }

    return 0;
}

void TaskGroupItem::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    //kDebug() << "LayoutWidget dropEvent";
    if (event->mimeData()->hasFormat(TaskManager::Task::mimetype())) {
        bool ok;
        WId id = TaskManager::Task::idFromMimeData(event->mimeData(), &ok);

        if (!ok) {
            event->ignore();
            return;
        }

        AbstractTaskItem *taskItem = m_applet->rootGroupItem()->taskItemForWId(id);

        if (!taskItem) {
            kDebug() << "Invalid TaskItem";
            event->ignore();
            return;
        }

        //grouping stuff
        //if (m_taskItems.contains(taskItem)) {
            // Q_ASSERT(m_groupItem);
            // kDebug()<< "Task has Group";

                bool noTargetTask = false;
                AbstractTaskItem *targetTask = 0;

                targetTask = dynamic_cast<AbstractTaskItem *>(scene()->itemAt(mapToScene(event->pos())));

                //  kDebug() << "Pos: " << event->pos() << mapToScene(event->pos()) << "item" << scene()->itemAt(mapToScene(event->pos())) << "target Task " << dynamic_cast<QGraphicsItem *>(targetTask);

                // kDebug() << "first item: " << dynamic_cast<QGraphicsItem*>(m_taskItems.first()) << "layout widget" << dynamic_cast<QGraphicsItem*>(this);

                if (!targetTask) {
                    //kDebug() << "no targetTask";
                    noTargetTask = true;
                }
                if (!taskItem->parentGroup()) {
                    //kDebug() << "group invalid";
                    return;
                }

                TaskManager::GroupPtr group = taskItem->parentGroup()->group();

                if ((event->modifiers() == m_applet->groupModifierKey()) &&
                    m_applet->groupManager().groupingStrategy() == TaskManager::GroupManager::ManualGrouping) {
                    //kDebug() << "groupModifiaction";

                    if (!noTargetTask) {
                        if (targetTask->isWindowItem() && (group == m_group)) { //Both Items in same group
                            //Group Items together
                            int targetIndex = m_group->members().indexOf(targetTask->abstractItem());
                            int sourceIndex = m_group->members().indexOf(taskItem->abstractItem());
                            TaskManager::ItemList members;
                            members.append(targetTask->abstractItem());
                            members.append(taskItem->abstractItem());
                            if (!m_applet->groupManager().manualGroupingRequest(members)) {
                                //kDebug() << "Couldn't create Group";
                                return;
                            }
                            if (sourceIndex < targetIndex) {
                                targetIndex--; //fix because the taskItem is removed so the index of the group should be targetIndex - 1
                            }
                            m_applet->groupManager().manualSortingRequest(taskItem->abstractItem()->parentGroup(), targetIndex);//move group to appropriate index if possible
                            event->acceptProposedAction(); // We do not care about the type of action
                            //kDebug() << "Group Created";
                            return;
                        } else if (!targetTask->isWindowItem()) { //Drop on collapsed group item
                            //kDebug() << "Add item to Group";
                            m_applet->groupManager().manualGroupingRequest(taskItem->abstractItem(), dynamic_cast<TaskManager::GroupPtr>(targetTask->abstractItem()));
                            event->acceptProposedAction(); // We do not care about the type of action
                            return;
                        }
                    }
                    //add item to this group
                    m_applet->groupManager().manualGroupingRequest(taskItem->abstractItem(), m_group);

                } else { //Move action
                    if((m_applet->groupManager().sortingStrategy() == TaskManager::GroupManager::ManualSorting)){
                        if (group == m_group) { //same group
                            //kDebug() << "Drag within group";
                            layoutTaskItem(taskItem, event->pos());
                        } else { //task item was dragged outside of group -> group move
                            AbstractTaskItem *directMember = abstractItem(m_group->directMember(group));
                            if (directMember) {
                                layoutTaskItem(directMember, event->pos()); //we need to get the group right under the receiver group
                            } else { //group isn't a member of this Group, this is the case if a task is dragged into a expanded group
                                event->ignore();
                                return;
                            }
                        }
                    }//group Sorting
                }//Move action
        event->acceptProposedAction(); // We do not care about the type of action
    } else {
        event->ignore();
    }
    //kDebug() << "LayoutWidget dropEvent done";
}

void TaskGroupItem::layoutTaskItem(AbstractTaskItem* item, const QPointF &pos)
{
    if (!m_expandedLayout) {
        return;
    }

    int insertIndex = m_expandedLayout->insertionIndexAt(pos);
// kDebug() << "Item inserting at: " << insertIndex << "of: " << numberOfItems();
    if (insertIndex == -1) {
        m_applet->groupManager().manualSortingRequest(item->abstractItem(), -1);
    } else {
        if (!m_parentSplitGroup) {
            m_applet->groupManager().manualSortingRequest(item->abstractItem(), insertIndex);
        } else {
            m_applet->groupManager().manualSortingRequest(item->abstractItem(), insertIndex +
                                                          m_parentSplitGroup->m_groupMembers.size());
        }
    }
}


void TaskGroupItem::updateActive(AbstractTaskItem *task)
{
    if (!m_expandedLayout) {
        return;
    }

    m_activeTaskIndex = indexOf(task);
}

int TaskGroupItem::indexOf (AbstractTaskItem *task)
{
    if (!m_group || !task) {
        kDebug() << "Error";
        return -1;
    }

    return m_group->members().indexOf(task->abstractItem());
}


void TaskGroupItem::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    //zero or one tasks don't cycle
    if (m_groupMembers.size() < 1) {
        return;
    }

    //mouse wheel down
    if (event->delta() < 0) {
        m_activeTaskIndex++;
        if (m_activeTaskIndex >= m_groupMembers.size()) {
            m_activeTaskIndex = 0;
        }
        //mouse wheel up
    } else {
        m_activeTaskIndex--;
        if (m_activeTaskIndex < 0) {
            m_activeTaskIndex = m_groupMembers.size() - 1; //last item is a spacer
        }
    }

    //    kDebug() << "Wheel event m_activeTaskIndex: " << m_activeTaskIndex << " of " << numberOfItems();
    AbstractGroupableItem *item = group()->members().at(m_activeTaskIndex);
    AbstractTaskItem *taskItem = abstractItem(item);
    if (taskItem) {
        taskItem->activate();
    }
}

int TaskGroupItem::maxRows()
{
    return m_maximumRows;
}

void TaskGroupItem::setMaxRows(int rows)
{
    m_maximumRows = rows;
    if (m_expandedLayout) {
        m_expandedLayout->setMaximumRows(m_maximumRows);
    }
}

bool TaskGroupItem::forceRows()
{
    return m_forceRows;
}

void TaskGroupItem::setForceRows(bool forceRows)
{
    m_forceRows = forceRows;
    if (m_expandedLayout) {
        m_expandedLayout->setForceRows(m_forceRows);
    }
}

int TaskGroupItem::optimumCapacity()
{
    if (m_expandedLayout) {
        return m_expandedLayout->maximumRows() * m_expandedLayout->preferredColumns();
    }

    return 1;
}

AbstractTaskItem* TaskGroupItem::abstractItem(AbstractItemPtr item)
{
    if (m_groupMembers.contains(item)) {
        return m_groupMembers.value(item);
    }
    //kDebug() << "item not found";
    return 0;
}

void TaskGroupItem::setAdditionalMimeData(QMimeData* mimeData)
{
    if (m_group) {
        m_group->addMimeData(mimeData);
    }
}

#include "taskgroupitem.moc"

