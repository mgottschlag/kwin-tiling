#include "layoutwidget.h"

//Taskmanager
#include <taskmanager/taskmanager.h>
#include <taskmanager/abstractgroupableitem.h>
#include <taskmanager/groupmanager.h>

// Qt
#include <QList>
#include <QGraphicsScene>
#include <QGraphicsGridLayout>
#include <QPainter>

#include <math.h>

#include "windowtaskitem.h"
#include "taskgroupitem.h"


//GroupItem Constructor
LayoutWidget::LayoutWidget(TaskGroupItem *parent, Tasks *applet)
    : QObject(parent),
      m_hasSpacer(false),
      m_spacer(0),
      m_groupItem(parent),
      m_rowSize(1),
      m_maxRows(1),
      m_forceRows(false),
      m_applet(applet),
      m_layout(0)
{
    init();
    //kDebug();
    foreach(AbstractTaskItem *item, m_groupItem->memberList()) {
        addTaskItem(item);
    }
}

LayoutWidget::~LayoutWidget()
{
    //kDebug();
}


void LayoutWidget::init()
{
    createLayout();
    //calculatePreferredRowSize();
}


void LayoutWidget::constraintsChanged(Plasma::Constraints constraints)
{
    Q_ASSERT(m_applet);
    //kDebug();

    if (constraints & Plasma::SizeConstraint) {
        layoutItems();
    }
}

void LayoutWidget::addTaskItem(AbstractTaskItem * item)
{
    //kDebug();
    if (!item) {
        kDebug() << "invalid item";
        return;
    }
    if (m_itemPositions.contains(item)) {
        kDebug() << "already in this layout";
        return;
    }
    if (item->abstractItem()) {
        if (item->abstractItem()->isGroupItem()) {
            connect(static_cast<TaskGroupItem*>(item), SIGNAL(changed()), this, SLOT(update())); //update on expanding group
        }
    }
    if (m_groupItem->scene() && !item->scene()) {
        //kDebug() << "layout widget got scene"<<m_groupItem->scene()<< "add item to scene" <<item->scene();
        m_groupItem->scene()->addItem(item);
        //kDebug() << "itemScene" << item->scene();
    }

   if (!insert(m_groupItem->indexOf(item), item)) {
        kDebug() << "error on  insert";
        return;
    }
    item->show();
    //kDebug() << "end";
}

void LayoutWidget::removeTaskItem(AbstractTaskItem * item)
{
    if (!remove(item)) {
        return;
    }

    //kDebug();

    if (m_groupItem->scene()) {
        //kDebug() << "got scene";
        m_groupItem->scene()->removeItem(item);
    } else {
        kDebug() << "No Scene available";
    }
    //kDebug() << "done";
}

bool LayoutWidget::insert(int index, AbstractTaskItem* item)
{
    //kDebug() << item->text() << index;
    if (!item ) {
        kDebug() << "error";
        return false;
    }
    int listIndex;
    for (listIndex = 0; listIndex < m_itemPositions.size(); listIndex++) {
        if (index <= m_groupItem->indexOf(m_itemPositions.at(listIndex))) {
            break;
        }
    }

    m_itemPositions.insert(listIndex, item);

    layoutItems();
    return true;
}

bool LayoutWidget::remove(AbstractTaskItem* item)
{
    if (!item) {
        kDebug() << "null Item";
    }

    m_itemPositions.removeAll(item);
    layoutItems();
    return true;
}

void LayoutWidget::update()
{
    //kDebug();
    layoutItems();
}


/** size including expanded groups*/
int LayoutWidget::size()
{
    int groupSize = 0;

    foreach (AbstractTaskItem *item, m_groupItem->memberList()) {
        if (!item->abstractItem()) { //this item is a startup task
            kDebug() << "Error, invalid item in groupMembers";
            continue;
        }

        if (item->abstractItem()->isGroupItem()) {
            TaskGroupItem *group = static_cast<TaskGroupItem*>(item);
            if (!group->collapsed()) {
                LayoutWidget *layout = dynamic_cast<LayoutWidget*>(group->layoutWidget());
                if (!layout) {
                    kDebug() << "Error group has no layout";
                    continue;
                }
                groupSize += layout->size();// increase number of items since expanded groups occupy several spaces
                continue;
            }
        }
        groupSize++;
    }

    //kDebug() << "group size" << groupSize;
    return groupSize;
}

//return maximum colums set by the user unless the setting is to high and the items would get unusable
int LayoutWidget::maximumRows()
{
    int maxRows;
    if (m_itemPositions.isEmpty()) {
        return 1;
    }

    if (m_forceRows) {
        return m_maxRows;
    }

    // in this case rows are columns, columns are rows...
    //TODO basicPreferredSize isn't the optimal source here because  it changes because of margins probably
    QSizeF itemSize = m_itemPositions.first()->basicPreferredSize();
    if (m_applet->formFactor() == Plasma::Vertical) {
        maxRows = qMin(qMax(1, int(m_groupItem->geometry().width() / itemSize.width())), m_maxRows);
    } else {
        maxRows = qMin(qMax(1, int(m_groupItem->geometry().height() / itemSize.height())), m_maxRows);
    }
    //kDebug() << "maximum rows: " << maxRows << m_maxRows << m_groupItem->geometry().height() << itemSize.height();
    return maxRows;
}

//returns a reasonable amount of columns
int LayoutWidget::preferredColumns()
{
    if (m_forceRows) {
        m_rowSize = 1;
    } else {
        if (m_itemPositions.isEmpty()) {
            return 1;
        }

        //TODO basicPreferredSize isn't the optimal source here because  it changes because of margins probably
        QSizeF itemSize = m_itemPositions.first()->basicPreferredSize();
        //kDebug() << itemSize.width() << m_groupItem->geometry().width();
        if (m_applet->formFactor() == Plasma::Vertical) {
            m_rowSize = qMax(1, int(m_groupItem->geometry().height() / itemSize.height()));
        } else {
            m_rowSize = qMax(1, int(m_groupItem->geometry().width() / itemSize.width()));
        }
    }
    //kDebug() << "preferred columns: " << qMax(1, qMin(m_rowSize, size()));
    return qMax(1, qMin(m_rowSize, size()));
}
// <columns,rows>
QPair<int, int> LayoutWidget::gridLayoutSize()
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

    return QPair <int,int> (columns, rows);
}

void LayoutWidget::createLayout()
{
    m_layout = new QGraphicsGridLayout(m_groupItem);
    m_layout->setContentsMargins(0,0,0,0);
    m_layout->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
    m_layout->setMaximumSize(INT_MAX,INT_MAX);
}

void LayoutWidget::layoutItems()
{
    //kDebug();

    QPair <int,int> grid = gridLayoutSize();
    int columns = grid.first;
    int rows = grid.second;

    //kDebug() << "Laying out with" << columns << rows;
    //kDebug() << "geometry" << m_groupItem->geometry();
    int rowHeight = qMax(1, int(m_groupItem->geometry().height() / rows));
    //kDebug() << "rowHeight" << rowHeight;
    int columnWidth = qMax(1, int(m_groupItem->geometry().size().width() / columns));
    //kDebug() << "column width set to " << columnWidth;

    QSizeF maximumCellSize;
    if (!m_itemPositions.isEmpty()) {
        maximumCellSize = m_itemPositions.first()->basicPreferredSize() * 1.8;
    }

    createLayout(); //its a shame that we have to create a new layout every time but the QGraphicsGridLayout is just to buggy yet

    if (m_applet->formFactor() == Plasma::Vertical) {
        m_layout->setHorizontalSpacing(0);
        m_layout->setVerticalSpacing(2);
    } else {
        m_layout->setHorizontalSpacing(2);
        m_layout->setVerticalSpacing(0);
    }

    //go through all items of this layoutwidget and populate the layout with items
    int numberOfItems = 0;
    foreach (AbstractTaskItem *item, m_itemPositions) {
        int row;
        int col;
        if (!m_forceRows) {
            if (m_applet->formFactor() == Plasma::Vertical) {
                row = numberOfItems % columns;
                col = numberOfItems / columns;
            } else {
                row = numberOfItems / columns;
                col = numberOfItems % columns;
            }

        } else {
            if (m_applet->formFactor() == Plasma::Vertical) {
                row = numberOfItems / rows;
                col = numberOfItems % rows;
            } else {
                row = numberOfItems % rows;
                col = numberOfItems / rows;
            }
        }


        m_layout->setColumnPreferredWidth(col, columnWidth);//Somehow this line is absolutely crucial
        m_layout->setRowPreferredHeight(row, rowHeight);//Somehow this line is absolutely crucial

        if (maximumCellSize.isValid()) {
            if (m_applet->formFactor() == Plasma::Vertical) {
                m_layout->setRowMaximumHeight(row, maximumCellSize.height());
            } else {
                m_layout->setColumnMaximumWidth(col, maximumCellSize.width());
            }
        }

        if (item->abstractItem() && item->abstractItem()->isGroupItem()) {
            TaskGroupItem *group = static_cast<TaskGroupItem*>(item);
            if (group->collapsed()) {
                group->unsplitGroup();
                m_layout->addItem(item, row, col, 1, 1);
            } else {
                LayoutWidget *layout = group->layoutWidget();
                if (!layout) {
                    kDebug() << "group has no valid layout";
                    continue;
                }
                int groupRowWidth = layout->numberOfColumns();

                if ((columns-col) < groupRowWidth) {//we need to split the group
                    int splitIndex = columns - col;//number of items in group that are on this row
                    TaskGroupItem *splitChild = group->splitGroup(splitIndex);
                    m_layout->addItem(item, row, col, 1, splitIndex); //Add the normal item
                    //kDebug() << "add normal item: split index = column span " << splitIndex;
                    if (splitChild) {
                       m_layout->addItem(splitChild, row + 1, 0, 1, groupRowWidth - splitIndex);//also add the second part of the group if there is one
                    }
                    //kDebug() << "add split item: column span " << groupRowWidth - splitIndex;
                } else  {
                    group->unsplitGroup();
                    m_layout->addItem(item, row, col, 1, groupRowWidth); //Add the normal item
                    //kDebug() << "add unsplit expanded item over columns " << groupRowWidth;
                }

                numberOfItems += groupRowWidth - 1;
            }
        } else {
            m_layout->addItem(item, row, col, 1, 1);
        }

        //kDebug() << "addItem at: " << row  <<  col;
        numberOfItems++;
    }

    updatePreferredSize();
    m_groupItem->setLayout(m_layout);
}


void LayoutWidget::updatePreferredSize()
{
    //kDebug() << "column count: " << m_layout->columnCount();

    if (m_layout->count() > 0) {
        AbstractTaskItem *item = dynamic_cast<AbstractTaskItem *>(m_layout->itemAt(0));
        Q_ASSERT(item);
        m_layout->setPreferredSize(item->basicPreferredSize().width()*m_layout->columnCount(), item->basicPreferredSize().height()*m_layout->rowCount());
    //Empty taskbar, arbitrary small value
    } else {
        kDebug() << "Empty layout!!!!!!!!!!!!!!!!!!";
        if (m_applet->formFactor() == Plasma::Vertical) {
            m_layout->setPreferredSize(/*m_layout->preferredSize().width()*/10, 10); //since we recreate the layout we don't have the previous values
        } else {
            m_layout->setPreferredSize(10, /*m_layout->preferredSize().height()*/10);
        }
    }
    //kDebug() << "preferred size: " << m_layout->preferredSize();
    emit sizeHintChanged(Qt::PreferredSize);
}

void LayoutWidget::setMaximumRows(int rows)
{
    m_maxRows = rows;
}

void LayoutWidget::setForceRows(bool forceRows)
{
    m_forceRows = forceRows;
}

int LayoutWidget::insertionIndexAt(const QPointF &pos)
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
            if (m_applet->formFactor() == Plasma::Vertical) {
                siblingGeometry = m_layout->itemAt(0, i)->geometry();//set geometry of single item
                if (pos.x() <= siblingGeometry.right()) {
                    row = i;
                    break;
                }
            } else {
                siblingGeometry = m_layout->itemAt(i, 0)->geometry();//set geometry of single item
                if (pos.y() <= siblingGeometry.bottom()) {
                    row = i;
                    break;
                }
            }
        }
        //and column
        for (int i = 0; i < numberOfColumns(); i++) {
            if (m_applet->formFactor() == Plasma::Vertical) {
                siblingGeometry = m_layout->itemAt(i, 0)->geometry();//set geometry of single item
                qreal vertMiddle = (siblingGeometry.top() + siblingGeometry.bottom()) / 2.0;
                if (pos.y() < vertMiddle) {
                    col = i;
                    break;
                }

            } else {
                siblingGeometry = m_layout->itemAt(0, i)->geometry();//set geometry of single item
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

    insertIndex = row *numberOfColumns() + col;

    if (insertIndex > m_layout->count()) {
        insertIndex--;
        //kDebug() << "correction";
    }

    //kDebug() << "insert Index" << insertIndex;
    return insertIndex;
}

int LayoutWidget::numberOfRows()
{
    if (m_applet->formFactor() == Plasma::Vertical) {
        return m_layout->columnCount();
    } else {
        return m_layout->rowCount();
    }
}

int LayoutWidget::numberOfColumns()
{
    if (m_applet->formFactor() == Plasma::Vertical) {
        return m_layout->rowCount();
    } else {
        return m_layout->columnCount();
    }
}


#include "layoutwidget.moc"

