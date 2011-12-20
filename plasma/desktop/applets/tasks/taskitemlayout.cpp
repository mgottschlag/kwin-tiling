/***************************************************************************
 *   Copyright (C) 2008 by Christian Mollekopf chrigi_1@fastmail.fm        *
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

#include "taskitemlayout.h"

//Taskmanager
#include <taskmanager/taskmanager.h>
#include <taskmanager/abstractgroupableitem.h>
#include <taskmanager/groupmanager.h>

// Qt
#include <QGraphicsScene>
#include <QGraphicsGridLayout>

// KDE
#include <KDebug>

#include <math.h>

#include "windowtaskitem.h"
#include "taskgroupitem.h"


//GroupItem Constructor
TaskItemLayout::TaskItemLayout(TaskGroupItem *parent, Tasks *applet)
    : QGraphicsGridLayout(0),
      m_hasSpacer(false),
      m_spacer(0),
      m_groupItem(parent),
      m_rowSize(1),
      m_maxRows(1),
      m_forceRows(false),
      m_applet(applet),
      m_layoutOrientation(Qt::Horizontal)
{
    setContentsMargins(0,0,0,0);
    setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
    setMaximumSize(INT_MAX,INT_MAX);
    //kDebug();
    foreach (AbstractTaskItem *item, parent->members()) {
        addTaskItem(item);
    }
}

TaskItemLayout::~TaskItemLayout()
{
    //kDebug();
}

void TaskItemLayout::setOrientation(Plasma::FormFactor orientation)
{
    Qt::Orientation oldOrientation = m_layoutOrientation;

    if (orientation == Plasma::Vertical) {
        m_layoutOrientation = Qt::Vertical;
    } else {
        m_layoutOrientation = Qt::Horizontal;
    }

    if (m_layoutOrientation != oldOrientation) {
        layoutItems();
    }
}



void TaskItemLayout::addTaskItem(AbstractTaskItem * item)
{
    //kDebug();
    if (!item || !m_groupItem) {
        return;
    }

    if (m_itemPositions.contains(item)) {
        //kDebug() << "already in this layout";
        return;
    }

    if (m_groupItem.data()->scene() && !item->scene()) {
        //kDebug() << "layout widget got scene"<<m_groupItem->scene()<< "add item to scene" <<item->scene();
        m_groupItem.data()->scene()->addItem(item);
        //kDebug() << "itemScene" << item->scene();
    }

    if (!insert(m_groupItem.data()->indexOf(item, false), item)) {
        kDebug() << "error on  insert";
        return;
    }

    item->show();
    //kDebug() << "end";
}

void TaskItemLayout::removeTaskItem(AbstractTaskItem *item)
{
    if (!remove(item)) {
        return;
    }

    if (m_groupItem && m_groupItem.data()->scene()) {
        m_groupItem.data()->scene()->removeItem(item);
    }
}

bool TaskItemLayout::insert(int index, AbstractTaskItem *item)
{
    //kDebug() << item->text() << index;
    if (!item || !m_groupItem) {
        kDebug() << "error";
        return false;
    }

    int listIndex;
    for (listIndex = 0; listIndex < m_itemPositions.size(); listIndex++) {
        if (index <= m_groupItem.data()->indexOf(m_itemPositions.at(listIndex), false)) {
            break;
        }
    }

    if (m_itemPositions.removeAll(item) == 0) {
        connect(item, SIGNAL(destroyed(AbstractTaskItem*)), this, SLOT(remove(AbstractTaskItem*)));
    }

    m_itemPositions.insert(listIndex, item);

    layoutItems();
    return true;
}

bool TaskItemLayout::remove(AbstractTaskItem* item)
{
    if (item) {
        disconnect(item, 0, this, 0);
        m_itemPositions.removeAll(item);
    }

    layoutItems();
    return item != 0;
}


/** size including expanded groups*/
int TaskItemLayout::size()
{
    int groupSize = 0;
    if (!m_groupItem) {
        return 0;
    }

    foreach (AbstractTaskItem *item, m_groupItem.data()->members()) {
        if (!item->abstractItem()) {
            // this item is a startup task or the task no longer exists
            kDebug() << "Error, invalid item in groupMembers";
            continue;
        }

        if (item->abstractItem()->itemType() == TaskManager::GroupItemType) {
            TaskGroupItem *group = static_cast<TaskGroupItem*>(item);
            if (!group->collapsed()) {
                TaskItemLayout *layout = dynamic_cast<TaskItemLayout*>(group->tasksLayout());
                if (!layout) {
                    kDebug() << "Error group has no layout";
                    continue;
                }

                // increase number of items since expanded groups occupy several spaces
                groupSize += layout->size();
                continue;
            }
        }

        ++groupSize;
    }

    //kDebug() << "group size" << groupSize;
    return groupSize;
}

//return maximum colums set by the user unless the setting is to high and the items would get unusable
int TaskItemLayout::maximumRows()
{
    int maxRows;
    if (m_itemPositions.isEmpty() || !m_groupItem) {
        return 1;
    }

    if (m_forceRows) {
        return m_maxRows;
    }

    // in this case rows are columns, columns are rows...
    //TODO basicPreferredSize isn't the optimal source here because  it changes because of margins probably
    QSizeF itemSize = m_itemPositions.first()->basicPreferredSize();
    if (m_layoutOrientation == Qt::Vertical) {
        maxRows = qMin(qMax(1, int(m_groupItem.data()->geometry().width() / itemSize.width())), m_maxRows);
    } else {
        maxRows = qMin(qMax(1, int(m_groupItem.data()->geometry().height() / itemSize.height())), m_maxRows);
    }

    //kDebug() << "maximum rows: " << maxRows << m_maxRows << m_groupItem->geometry().height() << itemSize.height();
    return maxRows;
}

//returns a reasonable amount of columns
int TaskItemLayout::preferredColumns()
{
    if (m_forceRows || !m_groupItem) {
        m_rowSize = 1;
    } else {
        if (m_itemPositions.isEmpty()) {
            return 1;
        }

        //TODO basicPreferredSize isn't the optimal source here because  it changes because of margins probably
        QSizeF itemSize = m_itemPositions.first()->basicPreferredSize();
        //kDebug() << itemSize.width() << m_groupItem.data()->geometry().width();
        if (m_layoutOrientation == Qt::Vertical) {
            m_rowSize = qMax(1, int(m_groupItem.data()->geometry().height() / itemSize.height()));
        } else {
            //Launchers doesn't need the same space as task- and groupitems on horizontal Layouts so the size needs to be adjusted
            qreal horizontalSpace = m_groupItem.data()->geometry().width();
            int numberOflaunchers = 0;
            foreach (AbstractTaskItem *item, m_itemPositions) {
                if (item->abstractItem() && item->abstractItem()->itemType() == TaskManager::LauncherItemType) {
                    horizontalSpace -= item->preferredHeight(); //The icon is a square so we can use the height as width
                    numberOflaunchers++;
                }
            }
            m_rowSize = qMax(1, int(horizontalSpace / itemSize.width()));
            m_rowSize += numberOflaunchers;
        }
    }
    //kDebug() << "preferred columns: " << qMax(1, m_rowSize);
    return qMax(1, m_rowSize);
}

// <columns,rows>
QPair<int, int> TaskItemLayout::gridLayoutSize()
{
    int groupSize = size();
    //the basic settings
    int columns = preferredColumns();
    int maxRows = maximumRows();

    //check for adjustments on columns because there isnt room enough yet for all of the items
    while (ceil(static_cast<float>(groupSize)/static_cast<float>(columns)) > maxRows) {
        columns++;  // more rows needed than allowed so we add some columns instead
    }
    //kDebug() << "groupWidth" << columns << maxRows << m_maxRows;
    int rows;
    if (m_forceRows) {
        rows = maxRows;
    } else {
        rows = ceil(static_cast<float>(groupSize) / static_cast<float>(columns)); //actually needed rows
    }

    return QPair<int,int>(columns, rows);
}


void TaskItemLayout::layoutItems()
{
    //kDebug();

    QPair<int,int> grid = gridLayoutSize();
    int columns = qMax(grid.first, 1);
    //int rows = qMax(grid.second, 1);

    //kDebug() << "Laying out with" << columns << rows;
    //kDebug() << "geometry" << m_groupItem.data()->geometry();
    //int rowHeight = qMax(1, int(m_groupItem.data()->geometry().height() / rows));
    //kDebug() << "rowHeight" << rowHeight;
    //int columnWidth = qMax(1, int(m_groupItem.data()->geometry().size().width() / columns));
    //kDebug() << "column width set to " << columnWidth;

    //FIXME: resetting column preferred sizesthey shouldn't be taken into account for inexistent ones but they are, probably upstream issue
    for (int i = 0; i < columnCount(); ++i) {
        setColumnMaximumWidth(i, 0);
        setColumnPreferredWidth(i, 0);
    }

    for (int i = 0; i < rowCount(); ++i) {
        setRowMaximumHeight(i, 0);
        setRowPreferredHeight(i, 0);
    }

    //clearLayout
    while (count()) {
        removeAt(0);
    }

    QSizeF maximumCellSize;
    if (!m_itemPositions.isEmpty()) {
        maximumCellSize = m_itemPositions.first()->basicPreferredSize() * 1.8;
    }

    setHorizontalSpacing(0);
    setVerticalSpacing(0);

    //go through all items of this layoutwidget and populate the layout with items
    int numberOfItems = 0;
    foreach (AbstractTaskItem *item, m_itemPositions) {
        int row;
        int col;
        if (m_layoutOrientation == Qt::Vertical) {
            row = numberOfItems % columns;
            col = numberOfItems / columns;
        } else {
            row = numberOfItems / columns;
            col = numberOfItems % columns;
        }

        //not good if we don't recreate the layout every time
        //m_layout->setColumnPreferredWidth(col, columnWidth);//Somehow this line is absolutely crucial
        //m_layout->setRowPreferredHeight(row, rowHeight);//Somehow this line is absolutely crucial


        //FIXME: this is a glorious hack
        if (maximumCellSize.isValid()) {
            if (m_layoutOrientation == Qt::Vertical) {
                setRowMaximumHeight(row, maximumCellSize.height());
                setColumnMaximumWidth(col, QWIDGETSIZE_MAX);
            } else {
                if (item->abstractItem() && item->abstractItem()->itemType() == TaskManager::LauncherItemType) {
                    setColumnFixedWidth(col, maximumCellSize.height()); //The Icon size is a sqare, so it needs the same width as height
                } else {
                    setColumnMaximumWidth(col, maximumCellSize.width());
                }
                setRowMaximumHeight(row, QWIDGETSIZE_MAX);
            }
            setRowPreferredHeight(row, maximumCellSize.height());
            setColumnPreferredWidth(col, maximumCellSize.width());
        }

        if (item->abstractItem() &&
            item->abstractItem()->itemType() == TaskManager::GroupItemType) {

            TaskGroupItem *group = static_cast<TaskGroupItem*>(item);
            if (group->collapsed()) {
                group->unsplitGroup();
                addItem(item, row, col, 1, 1);
                numberOfItems++;
            } else {
                TaskItemLayout *layout = group->tasksLayout();
                if (!layout) {
                    kDebug() << "group has no valid layout";
                    continue;
                }

                int groupRowWidth = m_layoutOrientation == Qt::Vertical ? layout->numberOfRows() : layout->numberOfColumns();

                if ((columns - col) < groupRowWidth) {
                    //we need to split the group
                    int splitIndex = columns - col;//number of items in group that are on this row
                    if (m_layoutOrientation == Qt::Vertical) {
                        addItem(item, row, col, splitIndex, 1);
                    } else {
                        addItem(item, row, col, 1, splitIndex);
                    }

                    //kDebug() << "add normal item: split index = column span " << splitIndex;
                    TaskGroupItem *splitChild = group->splitGroup(splitIndex);
                    if (splitChild) {
                        //also add the second part of the group if there is one
                        if (m_layoutOrientation == Qt::Vertical) {
                            addItem(splitChild, 0, col + 1, groupRowWidth - splitIndex, 1);
                        } else {
                            addItem(splitChild, row + 1, 0, 1, groupRowWidth - splitIndex);
                        }
                    }
                    //kDebug() << "add split item: column span " << groupRowWidth - splitIndex;
                } else  {
                    //Add the normal item
                    group->unsplitGroup();

                    if (m_layoutOrientation == Qt::Vertical) {
                        addItem(item, row, col, groupRowWidth, 1);
                    } else {
                        addItem(item, row, col, 1, groupRowWidth);
                    }
                    //kDebug() << "add unsplit expanded item over columns " << groupRowWidth;
                }

                numberOfItems += groupRowWidth;
            }
        } else {
            addItem(item, row, col, 1, 1);
            numberOfItems++;
        }

        //kDebug() << "addItem at: " << row  <<  col;
    }

    updatePreferredSize();
    //m_groupItem.data()->setLayout(m_layout);
}


void TaskItemLayout::updatePreferredSize()
{
    //kDebug() << "column count: " << m_layout->columnCount();

    if (count() > 0) {
        QSizeF s = itemAt(0)->preferredSize();
        //kDebug() << s << columnCount();
        setPreferredSize(s.width() * columnCount(), s.height() * rowCount());
    } else {
        //Empty taskbar, arbitrary small value
        kDebug() << "Empty layout!!!!!!!!!!!!!!!!!!";
        if (m_layoutOrientation == Qt::Vertical) {
            setPreferredSize(/*m_layout->preferredSize().width()*/10, 10); //since we recreate the layout we don't have the previous values
        } else {
            setPreferredSize(10, /*m_layout->preferredSize().height()*/10);
        }
    }
    //kDebug() << "preferred size: " << m_layout->preferredSize();
    if (m_groupItem) {
        m_groupItem.data()->updatePreferredSize();
    }
}

void TaskItemLayout::setMaximumRows(int rows)
{
    if (rows != m_maxRows) {
        m_maxRows = rows;
        layoutItems();
    }
}

void TaskItemLayout::setForceRows(bool forceRows)
{
    m_forceRows = forceRows;
}

int TaskItemLayout::insertionIndexAt(const QPointF &pos)
{
    int insertIndex = -1;
    int row = numberOfRows();
    int col = numberOfColumns();

    //if pos is (-1,-1) insert at the end of the panel
    if (pos.toPoint() == QPoint(-1, -1)) {
        kDebug() << "Error";
        return -1;
    } else {
        QRectF siblingGeometry;

        //get correct row
        for (int i = 0; i < numberOfRows(); i++) {
            if (m_layoutOrientation == Qt::Vertical) {
                siblingGeometry = itemAt(0, i)->geometry();//set geometry of single item
                if (pos.x() <= siblingGeometry.right()) {
                    row = i;
                    break;
                }
            } else {
                siblingGeometry = itemAt(i, 0)->geometry();//set geometry of single item
                if (pos.y() <= siblingGeometry.bottom()) {
                    row = i;
                    break;
                }
            }
        }
        //and column
        for (int i = 0; i < numberOfColumns(); i++) {
            if (m_layoutOrientation == Qt::Vertical) {
                siblingGeometry = itemAt(i, 0)->geometry();//set geometry of single item
                qreal vertMiddle = (siblingGeometry.top() + siblingGeometry.bottom()) / 2.0;
                if (pos.y() < vertMiddle) {
                    col = i;
                    break;
                }

            } else if (itemAt(0, i)) {
                siblingGeometry = itemAt(0, i)->geometry();//set geometry of single item
                qreal horizMiddle = (siblingGeometry.left() + siblingGeometry.right()) / 2.0;
                //kDebug() << "pos middle " << pos.x() << horizMiddle;
                if (pos.x() < horizMiddle) {
                    col = i;
                    break;
                }
            }
        }
    }

    //kDebug() << row << col;

    insertIndex = row * numberOfColumns() + col;

    if (insertIndex > count()) {
        insertIndex--;
        //kDebug() << "correction";
    }

    //kDebug() << "insert Index" << insertIndex;
    return insertIndex;
}

int TaskItemLayout::numberOfRows()
{
    if (m_layoutOrientation == Qt::Vertical) {
        return columnCount();
    } else {
        return rowCount();
    }
}

int TaskItemLayout::numberOfColumns()
{
    if (m_layoutOrientation == Qt::Vertical) {
        return rowCount();
    } else {
        return columnCount();
    }
}

#include "taskitemlayout.moc"

