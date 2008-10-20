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


#ifndef LAYOUTWIDGET_H
#define LAYOUTWIDGET_H

//Own
#include "tasks.h"

// Qt
#include <QObject>
#include <QGraphicsGridLayout>
#include <QList>


class TaskGroupItem;
class AbstractTaskItem;
//class QList;

using TaskManager::StartupPtr;
using TaskManager::TaskPtr;
using TaskManager::GroupManager;


/**
 * A task item for a task which represents a expanded group.
 */
class LayoutWidget : public QObject, public QGraphicsGridLayout
{
    Q_OBJECT

public:
    LayoutWidget(TaskGroupItem * parent, Tasks *applet, TaskManager::TaskGroup *group);
    ~LayoutWidget();

    void addTaskItem(AbstractTaskItem*);
    void removeTaskItem(AbstractTaskItem*);
    bool insert(int index, AbstractTaskItem* item);
    void setOrientation(Plasma::FormFactor);
    void setSpacer(bool);

    int getInsertIndex(const QPointF &pos);
    void setMaximumRows(int);

    Qt::Orientation orientation();
    /** the calculated width according to size() and maxRows*/
    int rowWidth();
    /** the size including expanded groups*/
    int size();
    
public Q_SLOTS:
    void update();

private Q_SLOTS:
    void constraintsChanged(Plasma::Constraints);

Q_SIGNALS:
    void sizeHintChanged(Qt::SizeHint);

private:
    void init();

    bool remove(AbstractTaskItem* item);

    void adjustStretch();
    void updatePreferredSize();

    void layoutItems();

    int numberOfItems();
    int itemsInRow(int);
    int numberOfRows();

    bool m_hasSpacer;
    QGraphicsWidget *m_spacer;

    TaskManager::TaskGroup *m_group;
    TaskGroupItem *m_groupItem;
    QList <AbstractTaskItem*> m_itemPositions;

    Qt::Orientation m_orientation;
    /** Limit before row is full, more columns are added if maxRows is exeeded*/
    int m_rowSize;
    /** How many rows  should be used. -1 for autoexpanding until maxRows*/
    int m_maxRows;

    Tasks *m_applet;
    int m_columnWidth;
    int m_rowHeight;
};

#endif
