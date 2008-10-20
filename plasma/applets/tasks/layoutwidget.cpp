#include "layoutwidget.h"

//Taskmanager
#include <taskmanager/taskmanager.h>
#include <taskmanager/abstractgroupableitem.h>
#include <taskmanager/groupmanager.h>

// Qt
#include <QGraphicsSceneWheelEvent>
#include <QList>
#include <QGraphicsScene>
#include <QGraphicsGridLayout>
#include <QPainter>

#include "windowtaskitem.h"
#include "taskgroupitem.h"


//GroupItem Constructor
LayoutWidget::LayoutWidget(TaskGroupItem *parent, Tasks *applet, TaskGroup *group)
    : QObject(parent), QGraphicsGridLayout(parent),
      m_hasSpacer(false),
      m_spacer(0),
      m_group(group),
      m_groupItem(parent),
      m_rowSize(6), //TODO calculate a reasonable default value
      m_maxRows(1),
      m_applet(applet),
      m_columnWidth(10),
      m_rowHeight(10)
{
    init();
    //kDebug();
    foreach(AbstractTaskItem *item, m_groupItem->getMemberList()) {
        addTaskItem(item);
    }
}

LayoutWidget::~LayoutWidget()
{
    setParent(0);
    //kDebug();
}

void LayoutWidget::init()
{
    setContentsMargins(0,0,0,0);
    setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
    setMaximumSize(INT_MAX,INT_MAX);
    //setMinimumSize(300,40);//FIXME debugging
    setOrientation(Plasma::Horizontal);

    if (m_group->isRootGroup()) { //expanded groups shouldn't have a spacer
        //setSpacer(true);
    }
}


void LayoutWidget::constraintsChanged(Plasma::Constraints constraints)
{
    Q_ASSERT(m_applet);
    kDebug();
    //setOrientation(m_applet->formFactor());
    if (constraints & Plasma::LocationConstraint) {
        setOrientation(m_applet->formFactor());
        //avoid to make all tasks disappear for a wrong minimum size of the spacer
        if (m_spacer) {
            m_spacer->setMaximumSize(INT_MAX, INT_MAX);
        }
    }

    if (constraints & Plasma::SizeConstraint) {
        //adjustStretch();
        layoutItems();
    }
}

void LayoutWidget::addTaskItem(AbstractTaskItem * item)
{
//    kDebug();
    if (!item) {
        kDebug() << "invalid item";
        return;
    }
    if (item->abstractItem()) {
        if (item->abstractItem()->isGroupItem()) {
            connect(static_cast<TaskGroupItem*>(item), SIGNAL(changed()), this, SLOT(update())); //update on expanding group
        }
    }
    if (m_groupItem->scene() && !item->scene()) {
        kDebug() << "layout widget got scene"<<m_groupItem->scene()<< "add item to scene" <<item->scene();
        m_groupItem->scene()->addItem(item);
        kDebug() << "itemScene" << item->scene();
    }
    //item->setParentItem(m_groupItem);

   // if (!insert(m_group->members().indexOf(item->abstractItem()), item)) {
   if (!insert(m_groupItem->getMemberList().indexOf(item), item)) {
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
  /*  if (item->abstractItem()->isGroupItem()) { 
        TaskGroupItem *groupItem = static_cast<TaskGroupItem*>(item);
        if (groupItem->isSplit()) {
            groupItem->unsplitGroup(); //TODO reasonable?
        }
    }*/

   // item->hide();
    if (m_groupItem->scene()) {
        //kDebug() << "got scene";
        m_groupItem->scene()->removeItem(item);
    } else {
        kDebug() << "No Scene available"; //happend once... and again
    }
    //kDebug() << "done";
}

void LayoutWidget::setOrientation(Plasma::FormFactor orientation)
{
    if (orientation == Plasma::Vertical) {
        m_orientation = Qt::Vertical;
    } else {
        m_orientation = Qt::Horizontal;
    }
}

bool LayoutWidget::insert(int index, AbstractTaskItem* item)
{
   // kDebug();
    if (!item ) {
        kDebug() << "error";
        return false;
    }

    if ((index <= numberOfItems()) && (index >= 0)) {
        m_itemPositions.insert(index, item);
    } else { /* if (index != -1) {
        kDebug() << "invalid index" << index;
        return false;
    } else { */
        m_itemPositions.insert(numberOfItems(), item);
    }

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
    kDebug();
    layoutItems();
}


/** size including expanded groups*/
int LayoutWidget::size()
{
    int groupSize = 0;// = m_groupItem->getMemberList().size();
    TaskGroupItem *group;
    foreach (AbstractTaskItem *item, m_groupItem->getMemberList()) {
        if(!item->abstractItem()) {
            kDebug() << "error";
            continue;
        }
        if (item->abstractItem()->isGroupItem()) {
           group = static_cast<TaskGroupItem*>(item);
            if (!group->collapsed()) { 
                LayoutWidget *layout = dynamic_cast<LayoutWidget*>(group->layout());
                if (!layout) {
                    kDebug() << "Error";
                    continue;
                }
                groupSize += group->getMemberList().size();//layout->size();// increase number of items since expanded groups occupy several spaces
                continue;
            }
        }
        groupSize++;
    }
    kDebug() << "group size" << groupSize;
    return groupSize;
}

/** width including expanded groups*/
int LayoutWidget::rowWidth()
{
    int columns = m_rowSize;
    if (!columns) {
        kDebug() << "divider collumns is 0!!!";
        return 0;
    }

    //kDebug() << geometry().height() << int(geometry().height() / 22) << m_maxRows;
    int maxRows;
    //in this cas rows are columns, columns are rows...
    //TODO: use not hardcoded values
    if (m_applet->formFactor() == Plasma::Vertical) {
        maxRows = qMin(qMax(1, int(geometry().width() / 150)), m_maxRows);
    } else {
        maxRows = qMin(qMax(1, int(geometry().height() / 22)), m_maxRows);
    }

    int totalSize = size();
  
    while ((totalSize / columns) > maxRows) {     
        columns++;  //more rows needed than allowed so we add some collumns instead
    }
    kDebug() << "groupWidth" << columns;
    return qMin(columns,totalSize);
}

void LayoutWidget::layoutItems()
{ //TODO optimize by only removing items from the item on that is inserted
    kDebug();
    while (numberOfItems()) { //clear the grid
        removeAt(0);
        //kDebug() << "remove";
    }

    int columns = rowWidth();

    if (columns) {
        m_columnWidth = geometry().size().width()/columns;
        kDebug() << "column width set to " << m_columnWidth;
    }
    m_rowHeight = geometry().size().height()/m_maxRows;

   int numberOfItems = 0;
   foreach (AbstractTaskItem *item, m_itemPositions) {
        int row;
        int col;
        if (m_applet->formFactor() == Plasma::Vertical) {
            row = numberOfItems % columns; 
            col = numberOfItems / columns;
        } else {
            row = numberOfItems / columns; 
            col = numberOfItems % columns;
        }

        setColumnPreferredWidth(col, m_columnWidth);//Somehow this line is absolutely crucial
        setRowPreferredHeight(row, m_rowHeight);//Somehow this line is absolutely crucial

        TaskGroupItem *group = static_cast<TaskGroupItem*>(item);
        if (item->abstractItem()->isGroupItem()) {
         //   kDebug() << "splitIndex" << splitIndex << "second Row: " << secondRow;
            if (!group->collapsed()) { 
                LayoutWidget *layout = dynamic_cast<LayoutWidget*>(group->layout());
                if (!layout) {
                    kDebug() << "Error";
                    continue;
                }
                int groupRowWidth = layout->rowWidth();
                int secondRow = ((groupRowWidth - 1 + numberOfItems)/columns); //row of the last item in the group 
             
                if (secondRow > row) {//we need to split the group TODO if (splitIndex < group->size())
                    m_itemPositions.removeAll(group->splitGroup());
                    int splitIndex = columns - col;//number of items in group that are on this row
                    TaskGroupItem *splitChild = group->splitGroup(splitIndex);
                    addItem(item, row, col, 1, splitIndex); //Add the normal item 
                    
                    for (int i = 1; i<splitIndex; i++) {
                      //  addItem(item, row, col+i, 1, 1); //Necessary to make sure the 
                    } 
                    kDebug() << "add normal item: split index = column span " << splitIndex;
                    if (splitChild) {
                        addItem(splitChild, secondRow, 0, 1, groupRowWidth - splitIndex);//also add the second part of the group if there is one
                    }
                    kDebug() << "add split item: column span " << groupRowWidth - splitIndex;
                } else  {
                    m_itemPositions.removeAll(group->splitGroup());
                    group->unsplitGroup();
                    addItem(item, row, col, 1, groupRowWidth); //Add the normal item 
                    kDebug() << "add unsplit expanded item over columns " << groupRowWidth;
                }
                numberOfItems += groupRowWidth - 1;
            } else {
                m_itemPositions.removeAll(group->splitGroup());
                group->unsplitGroup();
                addItem(item, row, col, 1, 1); 
            }

        } else {
            addItem(item, row, col, 1, 1); 
        }

        kDebug() << "addItem at: " << row  <<  col;
        numberOfItems++;
    }

    //invalidate();
    //adjustStretch();
    updatePreferredSize();
}


void LayoutWidget::updatePreferredSize()
{
    kDebug() << "column count: " << columnCount();
   /* if (columnCount()) {
        m_columnWidth = geometry().size().width()/columnCount();
        kDebug() << "column width set to " << m_columnWidth;
    }
    
    for(int i = 0; i<columnCount(); i++) {
        setColumnPreferredWidth(i, m_columnWidth);//Somehow this line is absolutely crucial
    }
    if (rowCount()) {
        m_rowHeight = geometry().size().height()/rowCount();
    }
    for(int i = 0; i<rowCount(); i++) {
        setRowPreferredHeight(i, m_rowHeight);//Somehow this line is absolutely crucial
    }*/

    if (count() > 0) {
        QGraphicsLayoutItem *item = itemAt(0);
        Q_ASSERT(item);
        if (orientation() == Qt::Vertical) {
            setPreferredSize(item->preferredSize().width()*rowCount(), item->preferredSize().height()*columnCount());
        } else {
            setPreferredSize(item->preferredSize().width()*columnCount(), item->preferredSize().height()*rowCount());
            //kDebug() << "horizontal size set";
        }
    //Empty taskbar, arbitrary small value
    } else {
        kDebug() << "Empty layout!!!!!!!!!!!!!!!!!!";
        if (orientation() == Qt::Vertical) {
            setPreferredSize(preferredSize().width(), 10);
        } else {
            setPreferredSize(10, preferredSize().height());
        }
    }
    emit sizeHintChanged(Qt::PreferredSize);
}

Qt::Orientation LayoutWidget::orientation()
{
    return m_orientation;
}

void LayoutWidget::setMaximumRows(int rows)
{
    m_maxRows = rows;
}

int LayoutWidget::getInsertIndex(const QPointF &pos)//FIXME implement vertical
{
    int insertIndex = -1;
    int row = 0;
    int col = 0;

    //if pos is (-1,-1) insert at the end of the panel
    if (pos.toPoint() == QPoint(-1, -1)) {
        kDebug() << "Error";
        return -1;
    } else {
        QRectF siblingGeometry;
        for (int i = 0; i < rowCount(); i++) {
            siblingGeometry = itemAt(i, 0)->geometry();//set geometry of single item
            if (pos.y() <= siblingGeometry.bottom()) {
                row = i;
                break;
            }
        }

        for (int i = 0; i < itemsInRow(row); i++) { //last item is a spacer
            siblingGeometry = itemAt(0, i)->geometry();//set geometry of single item
            qreal horizMiddle = (siblingGeometry.left() + siblingGeometry.right()) / 2.0;
            //kDebug() << "pos middle " << pos.x() << horizMiddle;
            if (pos.x() < horizMiddle) {
                col = i;
                break;
            } else if (pos.x() <= siblingGeometry.right()) {
                col = i + 1; //take next item if there is one
                break;
            }
        }

        //kDebug() << row << col;
    }

    insertIndex = row *columnCount() + col;

    if (insertIndex > numberOfItems()) {
        insertIndex--;
        //kDebug() << "correction";
    }

    //kDebug() << "insert Index" << insertIndex;
    return insertIndex;
}

int LayoutWidget::numberOfItems()
{
    //kDebug() << m_layout->count();
    if (m_hasSpacer) {
        //kDebug() << "spacer";
        return (count() - 1); //last item is a Spacer
    }
    return (count());
}

int LayoutWidget::numberOfRows()
{
    if (orientation() == Qt::Vertical) {
        return columnCount();
    } else {
        return rowCount();
    }
}

int LayoutWidget::itemsInRow(int row)
{
    //kDebug() << m_layout->count();
    if (row * m_rowSize < numberOfItems()) { //we are in a full row
        return columnCount();
    } else {
        int val;
        if (orientation() == Qt::Vertical) {
            val = numberOfItems() % rowCount();
            if (!val) {
                val = rowCount();
            }
        } else {
            val = numberOfItems() % columnCount();
            if (!val) {
                val = columnCount();
            }
        }
        return val;
    }
}

/*void LayoutWidget::setSpacer(bool state)
  {
    if (state && !m_hasSpacer) {
        m_spacer = new QGraphicsWidget(m_groupItem);
        m_spacer->setMinimumSize(QSizeF(0,0));
        addItem(m_spacer, 0, numberOfItems());
    } else if (!state && m_hasSpacer) {
        removeAt(count()-1);
    }
    m_hasSpacer = state;
}


void LayoutWidget::adjustStretch()
{
    if (!m_hasSpacer) {
        return;
    }

    if (count() < 2) {
        m_spacer->setMaximumSize(INT_MAX, INT_MAX);
        return;
    }

    QGraphicsLayoutItem *item = itemAt(0);

    //hiding spacer
    if (orientation() == Qt::Horizontal) {
        int itemSize = m_groupItem->size().width() / itemsInRow(numberOfRows());
        int prefSize = item->preferredSize().width();

        if (itemSize < prefSize) {
            m_spacer->setMaximumWidth(0);
        } else if (itemSize > prefSize + 10) {
            m_spacer->setMaximumWidth(INT_MAX);
        }
    } else {
        int itemSize = m_groupItem->size().height() / itemsInRow(numberOfRows());
        int prefSize = item->preferredSize().height();

        if (itemSize < prefSize) {
            m_spacer->setMaximumHeight(0);
        } else if (itemSize > prefSize + 10) {
            m_spacer->setMaximumHeight(INT_MAX);
        }
    }

}*/

#include "layoutwidget.moc"

