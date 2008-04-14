/***************************************************************************
 *   Copyright (C) 2007 by Robert Knight <robertknight@gmail.com>          *
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

// Own
#include "abstracttaskitem.h"

/**
 * A graphics item which holds a group of tasks.
 * To add a task to a group, set the AbstractTaskItem's parent
 * to the TaskGroupItem instance.
 *
 * Task groups can also contain other task groups.
 */
class TaskGroupItem : public AbstractTaskItem
{
    Q_OBJECT

public:
    /** Constructs a new task group with the specified parent. */
    TaskGroupItem(QGraphicsItem *parent, QObject *parentObject);

    /**
     * Specifies whether this group may contain sub-groups.
     * Defaults to true.
     */
    void setAllowSubGroups(bool subGroups);
    bool allowSubGroups() const;

    /**
     * Returns the group's tasks in the order in which they are laid out,
     * top-to-bottom for vertical groups and left-to-right for horizontal
     * groups.
     */
    QList<AbstractTaskItem*> tasks() const;

    /**
     * Inserts a new task item into the group.  The task is
     * removed from its existing group (if any).
     *
     * @param item The task item to insert into the group
     * @param index The position in which to insert the item.
     * If this is 0 the item will be the first in the group.
     * If this is tasks().count() or -1 the item will be
     * the last in the group.
     */
    void insertTask(AbstractTaskItem *item, int index = -1);

    /** Removes a task item from the group. */
    void removeTask(AbstractTaskItem *item);

    /**
     * Reorders a task item within a group.
     *
     * @param from The current position of the task to move
     * @param to The new position of the task to move
     */
    void reorderTasks(int from, int to);

    /**
     * The enum describes the available styles for the border
     * which can surround the group.
     */
    enum BorderStyle
    {
        NoBorder,
        CaptionBorder
    };
    /** Sets the style of the border which surrounds the group. */
    void setBorderStyle(BorderStyle style);
    /** Returns the style of the border which surrounds the group. */
    BorderStyle borderStyle() const;

    /**
     * Sets the color of the group.  This is used to render
     * the border and tint the background.
     */
    void setColor(const QColor &color);

    /** Returns the color of the group.  See setColor() */
    QColor color() const;

    /**
    * Sets the direction in which tasks are laid out.
    * @param dir direction
    */
//    void setOrientation(Qt::Orientation dir);

   /**
    * @returns direction in which tasks are laid out, default RightToLeft
    */
    //Qt::Orientation orientation();

    /**
     * Sets whether the task group is collapsed.
     * TODO: Document me
     */
    void setCollapsed(bool collapsed);
    /** Returns whether the task group is collapsed. */
    bool collapsed() const;

    /**
     * Cycle the active task in a circular behaviour
     *
     * @param delta  if >0 go to the previous one, else go to the successive one
     */
    void cycle(int delta);

    // reimplemented
    virtual void paint(QPainter *painter,
                       const QStyleOptionGraphicsItem *option,
                       QWidget *widget);

    virtual void activate();
    virtual void close();
    virtual QSizeF maximumSize() const;

    /** Event compression **/
    void queueGeometryUpdate();

public slots:
    void updateActive(AbstractTaskItem *task);

protected:
    /** Reimplemented **/
    virtual void timerEvent(QTimerEvent *event);
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

private:
    enum DropAction
    {
        NoAction,
        InsertTaskAction, // insert the dropped task into the group
        GroupTasksAction  // group the dropped task together with the
                          // task underneath it and insert the
                          // group at the event's position
    };
    DropAction dropAction(const QPointF &pos) const;

    void drawBorder(QPainter *painter,
                    const QStyleOptionGraphicsItem *option,
                    const QRectF &area);
    qreal titleHeight() const;
    static QFont titleFont();

    class TaskEntry
    {
    public:
        explicit TaskEntry(AbstractTaskItem *taskItem,
                           const QRectF &taskArea = QRectF())
        : task(taskItem),
          rect(taskArea)
        {}

        bool operator==(const TaskEntry entry) const { return entry.task == task; }

        AbstractTaskItem *task;
        QRectF rect;
    };
    QList<TaskEntry> _tasks;
    int _activeTask;
    BorderStyle _borderStyle;
    QColor _color;
    //DropAction _potentialDropAction;
    int _caretIndex;
    bool _allowSubGroups;
    int m_geometryUpdateTimerId;
    QGraphicsLinearLayout *m_layout;

    static const int CaretWidth = 5;
    static const int GroupBorderWidth = 16;
};

#endif
