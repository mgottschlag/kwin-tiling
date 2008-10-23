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


#ifndef TASKGROUPITEM_H
#define TASKGROUPITEM_H

#include "abstracttaskitem.h"
// Own
#include <taskmanager/taskmanager.h>
#include "tasks.h"
#include <QMap>
#include <QHash>

using TaskManager::TaskGroup;
using TaskManager::GroupPtr;
using TaskManager::TaskItem;
using TaskManager::AbstractGroupableItem;


class LayoutWidget;
class SplitGroupItem;
class QGraphicsLinearLayout;
typedef QMap<int, AbstractTaskItem*> Order;

/**
 * A task item for a TaskGroup. It can be displayed collapsed as single item or expanded as group.
 */
class TaskGroupItem : public AbstractTaskItem
{
    Q_OBJECT

public:
    /** Constructs a new representation for a taskgroup. */
    TaskGroupItem(QGraphicsWidget *parent, Tasks *applet, const bool showTooltip);

    /** Sets the group represented by this task. */
    void setGroup(TaskManager::GroupPtr);

    /** Returns the group represented by this task. */
    TaskManager::GroupPtr group() const;

    virtual void close();

    QList <AbstractTaskItem*> memberList() const;

    virtual bool isWindowItem() const;
    virtual bool isActive() const;

    bool collapsed() const;

    /** Returns Direct Member group if the passed item is in a subgroup */
    AbstractTaskItem *directMember(AbstractTaskItem *);
    
    /** Maximum number of Rows the group will have */
    int maxRows();
    //TODO remove and calculate reasonable value in the layout itself
    void setMaxRows(int);

    void setFillRows(bool);
    bool fillRows();

    /*
    *Why the split group works.
    *drag split group: in the drop event the getDirectMember function is used which retrieves the parent item of the split group
    *drop on split group: //on unsplitGroup the group adds all items from splitgroup / the grouping is done via the taskgroup in lib
    *collapse on split group: because windowTaskItems retrive its parentgroup via tasks
    */
    /** Splits the group into two items, returns the second, new part (splitgroup)*/
    TaskGroupItem * splitGroup();
    /** Splits the group on position and returns the splitgroup*/
    TaskGroupItem * splitGroup(int position);
    /** To remove the childSplitgroupItem and be one item again*/
    void unsplitGroup();
    /** To be called on SplitGroups instead of setGroup()*/
    void setSplitGroup(TaskGroup *group);
    bool isSplit();

    LayoutWidget *layoutWidget();

signals:
    /** Emitted when a window is selected for activation, minimization, iconification */
    void groupSelected(TaskGroupItem *);
    void sizeHintChanged(Qt::SizeHint);
    /** informs the LayoutWidget about changes */
    void changed();

public slots:
    virtual void activate();
    /**
    *   Reload all tasks
    */
    void reload();

    void expand();
    void collapse();
    void updatePreferredSize();

protected:
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    virtual void paint(QPainter *painter,const QStyleOptionGraphicsItem *option,QWidget *widget);

    virtual void dragEnterEvent(QGraphicsSceneDragDropEvent *event);
    virtual void dropEvent(QGraphicsSceneDragDropEvent *event);

    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);

public slots:
    void updateActive(AbstractTaskItem *);

protected:
    void updateToolTip();

protected slots:
    virtual void wheelEvent(QGraphicsSceneWheelEvent *event);

private slots:
    void updateTask(::TaskManager::TaskChanges changes);

    /** Stay informed about changes in group */
    void itemAdded(AbstractItemPtr);
    void itemRemoved(AbstractItemPtr);

    /** show a menu for editing group */
    void editGroup();
    /** Check if something has to be updated (postion, ....) */
    void itemChanged(AbstractItemPtr);

    void popupMenu();
    

private:
    void layoutTaskItem(AbstractTaskItem* item, const QPointF &pos);
    void setSplitIndex(int position);

    GroupPtr m_group;
    QList<AbstractTaskItem*> m_groupMembers;
    LayoutWidget *m_layoutWidget;
    QTimer *m_popupMenuTimer;
    QHash <int, Order> m_taskOrder;
    int m_lastActivated;
    int m_activeTaskIndex;
    int m_maximumRows;
    bool m_fillRows;
    bool m_isCollapsed;
    int m_splitPosition;
    TaskGroupItem *m_parentSplitGroup;
    TaskGroupItem *m_childSplitGroup;
};

#endif
