/*
 *   Copyright 2009 by Marco Martin <notmart@gmail.com>
 *
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2,
 *   or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "itemcontainer.h"
#include "itemview.h"
#include "resultwidget.h"
#include "models/commonmodel.h"
#include "iconactioncollection.h"

#include <cmath>

#include <QGraphicsGridLayout>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>

#include <QTimer>
#include <QWeakPointer>
#include <QAbstractItemModel>
#include <QPropertyAnimation>
#include <QAction>

#include <KDebug>
#include <KIconLoader>
#include <KIcon>

#include <Plasma/Applet>
#include <Plasma/ItemBackground>
#include <Plasma/Theme>
#include <Plasma/ToolTipContent>
#include <Plasma/ToolTipManager>

ItemContainer::ItemContainer(ItemView *parent)
    : QGraphicsWidget(parent),
      m_orientation(Qt::Vertical),
      m_currentIconIndexX(-1),
      m_currentIconIndexY(-1),
      m_iconSize(-1),
      m_spacerIndex(-1),
      m_firstRelayout(true),
      m_dragAndDropMode(ItemContainer::NoDragAndDrop),
      m_dragging(false),
      m_model(0),
      m_itemView(parent)
{
    m_positionAnimation = new QPropertyAnimation(this, "pos", this);
    m_positionAnimation->setEasingCurve(QEasingCurve::InOutQuad);
    m_positionAnimation->setDuration(250);

    setIconSize(KIconLoader::SizeHuge);

    QGraphicsItem *pi = parent->parentItem();
    Plasma::Applet *applet = 0;
    //FIXME: this way to find the Applet is quite horrible
    while (pi) {
        applet = dynamic_cast<Plasma::Applet *>(pi);
        if (applet) {
            break;
        } else {
            pi = pi->parentItem();
        }
    }
    m_iconActionCollection = new IconActionCollection(applet, this);

    setFocusPolicy(Qt::StrongFocus);
    setAcceptHoverEvents(true);
    m_hoverIndicator = new Plasma::ItemBackground(this);
    m_hoverIndicator->setZValue(-100);
    m_hoverIndicator->hide();

    m_relayoutTimer = new QTimer(this);
    m_relayoutTimer->setSingleShot(true);
    connect(m_relayoutTimer, SIGNAL(timeout()), this, SLOT(relayout()));

    m_setCurrentTimer = new QTimer(this);
    m_setCurrentTimer->setSingleShot(true);
    connect(m_setCurrentTimer, SIGNAL(timeout()), this, SLOT(syncCurrentItem()));

    m_hideUsedItemsTimer = new QTimer(this);
    m_hideUsedItemsTimer->setSingleShot(true);
    connect(m_hideUsedItemsTimer, SIGNAL(timeout()), this, SLOT(hideUsedItems()));
}

ItemContainer::~ItemContainer()
{}

void ItemContainer::setCurrentItem(ResultWidget *currentIcon)
{
    if (m_relayoutTimer->isActive()) {
        m_setCurrentTimer->start(400);
        m_currentIcon = currentIcon;
        return;
    }

    QWeakPointer<ResultWidget> currentWeakIcon = currentIcon;
    //m_currentIcon.clear();

    if (m_currentIcon.data() != currentIcon) {
        const int nColumns = qMax(1, (int)ceil(size().width() / m_cellSize.width()));

        for (int i = 0; i < m_model->rowCount(); ++i) {
            QModelIndex index = m_model->index(i, 0, m_rootIndex);
            ResultWidget *item = m_items.value(index);

            if (item == currentIcon) {
                m_currentIconIndexX = i % nColumns;
                m_currentIconIndexY = i / nColumns;
                break;
            }
        }
    }

    m_hoverIndicator->setTargetItem(currentIcon);
}

void ItemContainer::syncCurrentItem()
{
    setCurrentItem(m_currentIcon.data());
}

ResultWidget *ItemContainer::currentItem() const
{
    return m_currentIcon.data();
}

//FIXME: remove
QList<ResultWidget *> ItemContainer::items() const
{
    return m_items.values();
}

ResultWidget *ItemContainer::createItem(QModelIndex index)
{
    ResultWidget *item;
    if (!m_usedItems.isEmpty()) {
        int key = m_usedItems.keys().first();
        item = m_usedItems.values(key).first();
        m_usedItems.remove(key, item);
    } else {
        item = new ResultWidget(this);
        item->setCacheMode(QGraphicsItem::ItemCoordinateCache);
        item->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        item->hide();
        item->setPos(boundingRect().center().x(), size().height());
    }

    item->installEventFilter(m_itemView);

    if (index.isValid()) {
        item->setPreferredIconSize(QSizeF(m_iconSize, m_iconSize));
        item->setMaximumIconSize(QSizeF(m_iconSize, m_iconSize));
        item->setMinimumIconSize(QSizeF(m_iconSize, m_iconSize));
        item->setIcon(index.data(Qt::DecorationRole).value<QIcon>());
        item->setText(index.data(Qt::DisplayRole).value<QString>());

        Plasma::ToolTipContent toolTipData = Plasma::ToolTipContent();
        toolTipData.setAutohide(true);
        toolTipData.setMainText(index.data(Qt::DisplayRole).value<QString>());
        toolTipData.setSubText(index.data(CommonModel::Description).value<QString>());
        toolTipData.setImage(index.data(Qt::DecorationRole).value<QIcon>());

        Plasma::ToolTipManager::self()->registerWidget(this);
        Plasma::ToolTipManager::self()->setContent(item, toolTipData);

        CommonModel::ActionType actionType = (CommonModel::ActionType)index.data(CommonModel::ActionTypeRole).value<int>();
        if (actionType != CommonModel::NoAction) {
            QAction *action = new QAction(item);
            if (actionType == CommonModel::AddAction) {
                action->setIcon(KIcon("favorites"));
            } else {
                action->setIcon(KIcon("list-remove"));
            }
            item->addIconAction(action);
            m_iconActionCollection->addAction(action);
            connect(action, SIGNAL(triggered()), this, SLOT(actionTriggered()));
        }

        qreal left, top, right, bottom;
        m_hoverIndicator->getContentsMargins(&left, &top, &right, &bottom);
        item->setContentsMargins(left, top, right, bottom);

        item->setMinimumSize(m_cellSize);
        item->setMaximumSize(m_cellSize);
        item->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        connect(item, SIGNAL(clicked()), this, SLOT(itemClicked()));
        connect(item, SIGNAL(dragStartRequested(ResultWidget*)), this, SLOT(itemRequestedDrag(ResultWidget*)));
    }

    return item;
}

void ItemContainer::disposeItem(ResultWidget *icon)
{
    if (m_usedItems.count() < 40) {
        icon->removeIconAction(0);
        disconnect(icon, 0, 0, 0);

        int row = m_itemToIndex.value(icon).row();
        row = icon->pos().x() + icon->pos().y()/10 * size().width();

        m_usedItems.insert(row, icon);
        icon->removeEventFilter(m_itemView);

        //if they will be immediately recycled they won't be hidden at all
        m_hideUsedItemsTimer->start(500);
    } else {
        icon->deleteLater();
    }
}

void ItemContainer::setOrientation(Qt::Orientation orientation)
{
    m_orientation = orientation;
    if (orientation == Qt::Horizontal) {
        setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    } else {
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    }
    askRelayout();
}

Qt::Orientation ItemContainer::orientation() const
{
    return m_orientation;
}

void ItemContainer::setIconSize(int size)
{
    if (size == m_iconSize) {
        return;
    }

    m_iconSize = size;

    QFontMetrics fm(Plasma::Theme::defaultTheme()->font(Plasma::Theme::DesktopFont));
    int cellSize = m_iconSize + fm.height()*2 + 40;
    m_cellSize = QSize(cellSize, cellSize);

    foreach (ResultWidget *icon, m_items) {
        icon->setPreferredIconSize(QSizeF(size, size));
        icon->setMaximumIconSize(QSizeF(size, size));
        icon->setMinimumIconSize(QSizeF(size, size));
    }
}

int ItemContainer::iconSize() const
{
    return m_iconSize;
}

void ItemContainer::setDragAndDropMode(DragAndDropMode mode)
{
    m_dragAndDropMode = mode;
}

ItemContainer::DragAndDropMode ItemContainer::dragAndDropMode() const
{
    return m_dragAndDropMode;
}

void ItemContainer::askRelayout()
{
    if (!m_relayoutTimer->isActive()) {
        m_relayoutTimer->start(200);
    }
}

void ItemContainer::relayout()
{
    if (!m_model) {
        return;
    }


    //FIXME: reserve a random fixed size for the scrollbars, regardless they're present or not
    QSizeF availableSize(m_itemView->size() - QSizeF(30, 30));


    int nColumns = qMax(1, (int)availableSize.width() / m_cellSize.width());
    int nRows = qMax(1, (int)availableSize.height() / m_cellSize.height());

    nColumns = qMin(m_model->rowCount(), nColumns);
    nRows = qMin(m_model->rowCount(), nRows);

    setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);

    if (m_orientation == Qt::Vertical) {
        nRows = qMax(1, (int)ceil((qreal)m_model->rowCount() / nColumns));
        for (int i = 0; i <= m_model->rowCount() - 1; i++) {

            int actualIndex = i;
            if (m_spacerIndex > -1) {
                if (i >= m_spacerIndex) {
                    actualIndex = i+1;
                }
                if (i >= m_draggingIndex.row()) {
                    --actualIndex;
                }
            }

            const int row = actualIndex / nColumns;
            const int column = actualIndex % nColumns;

            QModelIndex index = m_model->index(i, 0, m_rootIndex);
            if (index == m_draggingIndex) {
                continue;
            }

            ResultWidget *icon = m_items.value(index);
            if (icon) {
                icon->animatePos(QPoint(column*m_cellSize.width(), row*m_cellSize.height()));
                icon->show();
            }
        }
    } else {
        nColumns = qMax(1, (int)ceil((qreal)m_model->rowCount() / nRows));
        for (int i = 0; i <= m_model->rowCount() - 1; i++) {

            int actualIndex = i;
            if (m_spacerIndex > -1) {
                if (i >= m_spacerIndex) {
                    actualIndex = i+1;
                }
                if (i >= m_draggingIndex.row()) {
                    --actualIndex;
                }
            }

            const int row = actualIndex % nRows;
            const int column = actualIndex / nRows;

            QModelIndex index = m_model->index(i, 0, m_rootIndex);
            if (index == m_draggingIndex) {
                continue;
            }
            ResultWidget *icon = m_items.value(index);
            if (icon) {
                icon->animatePos(QPoint(column*m_cellSize.width(), row*m_cellSize.height()));
                icon->show();
            }
        }
    }


    m_itemView->setSnapSize(QSizeF(m_cellSize.width(), m_cellSize.height()) + QSizeF(0,0)); //TODO: items spacing?

    QSizeF newSize = sizeHint(Qt::MinimumSize, QSizeF());
    newSize = QSizeF(nColumns*m_cellSize.width(), nRows*m_cellSize.height());

    setMaximumSize(newSize);
    setPreferredSize(newSize);
    resize(newSize);
    m_relayoutTimer->stop();
    m_firstRelayout = false;
}

void ItemContainer::itemRequestedDrag(ResultWidget *icon)
{
    if (m_dragging || dragAndDropMode() == NoDragAndDrop) {
        return;
    }

    if (dragAndDropMode() == MoveDragAndDrop) {
        m_dragging = true;
        icon->setZValue(900);
        icon->installEventFilter(this);
        m_draggingIndex = m_itemToIndex.value(icon);
        //ugly but necessary to don't make it clipped
        icon->setParentItem(0);
    }

    QModelIndex index = m_itemToIndex.value(icon);
    if (index.isValid()) {
        emit dragStartRequested(index);
    }
}

void ItemContainer::keyPressEvent(QKeyEvent *event)
{
    const int nColumns = qMax(1, (int)ceil(size().width() / m_cellSize.width()));
    const int nRows = qMax(1, (int)ceil(size().height() / m_cellSize.height()));

    switch (event->key()) {
    case Qt::Key_Left: {
        m_currentIcon.clear();
        while (!m_currentIcon.data()) {
            m_currentIconIndexX = (nColumns + m_currentIconIndexX - 1) % nColumns;
            m_currentIcon = m_items.value(m_model->index(nColumns*(m_currentIconIndexY)+m_currentIconIndexX, 0, m_rootIndex));
        }
        m_hoverIndicator->setTargetItem(m_currentIcon.data());
        emit itemSelected(m_currentIcon.data());
        break;
    }
    case Qt::Key_Right: {
        m_currentIcon.clear();
        while (!m_currentIcon.data()) {
            m_currentIconIndexX = (m_currentIconIndexX + 1) % nColumns;
            m_currentIcon = m_items.value(m_model->index(nColumns*(m_currentIconIndexY)+m_currentIconIndexX, 0, m_rootIndex));
        }
        m_hoverIndicator->setTargetItem(m_currentIcon.data());
        emit itemSelected(m_currentIcon.data());
        break;
    }
    case Qt::Key_Up: {
        m_currentIcon.clear();
        while (!m_currentIcon) {
            m_currentIconIndexY = (nRows + m_currentIconIndexY - 1) % nRows;
            m_currentIcon = m_items.value(m_model->index(nColumns*(m_currentIconIndexY)+m_currentIconIndexX, 0, m_rootIndex));
        }
        m_hoverIndicator->setTargetItem(m_currentIcon.data());
        emit itemSelected(m_currentIcon.data());
        break;
    }
    case Qt::Key_Down: {
        m_currentIcon.clear();
        while (!m_currentIcon.data()) {
            m_currentIconIndexY = (m_currentIconIndexY + 1) % nRows;
            m_currentIcon = m_items.value(m_model->index(nColumns*(m_currentIconIndexY)+m_currentIconIndexX, 0, m_rootIndex));
        }
        m_hoverIndicator->setTargetItem(m_currentIcon.data());
        emit itemSelected(m_currentIcon.data());
        break;
    }
    case Qt::Key_Enter:
    case Qt::Key_Return:
        if (m_currentIcon) {
            QModelIndex index = m_itemToIndex.value(m_currentIcon.data());
            if (index.isValid()) {
                emit itemActivated(m_model->index(index.row(), 0, m_rootIndex));
            }
        }
        break;
    case Qt::Key_Backspace:
    case Qt::Key_Home:
        emit resetRequested();
        break;
    default:
        break;
    }
}

QVariant ItemContainer::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemPositionChange) {
        QPointF newPos = value.toPointF();
        if (m_dragging) {
            return pos();
        }
    }

    return QGraphicsWidget::itemChange(change, value);
}

void ItemContainer::showSpacer(const QPointF &pos)
{
    if (pos == QPointF()) {
        m_spacerIndex = -1;
    } else {
        m_spacerIndex = rowForPosition(pos);
        //FIXME: this is not pretty, but rowForPosition should never return a number bigger than the model rows
        if ((m_orientation == Qt::Horizontal && pos.x() > size().width()-m_cellSize.width()/2) ||
            (m_orientation == Qt::Vertical && pos.y() > size().height()-m_cellSize.height()/2)) {
            m_spacerIndex++;
        }
    }
    askRelayout();
}

bool ItemContainer::eventFilter(QObject *watched, QEvent *event)
{
    ResultWidget *icon = qobject_cast<ResultWidget *>(watched);

    if (event->type() == QEvent::GraphicsSceneMouseMove) {
        QGraphicsSceneMouseEvent *me = static_cast<QGraphicsSceneMouseEvent *>(event);

        icon->setPos(icon->mapToParent(me->pos()) - icon->boundingRect().center());
        m_dragging = false;

        m_itemView->setScrollPositionFromDragPosition(icon->mapToParent(me->pos()));
        m_dragging = true;

        showSpacer(mapFromScene(me->scenePos()));

        askRelayout();
    } else if (event->type() == QEvent::GraphicsSceneMouseRelease) {
        QGraphicsSceneMouseEvent *me = static_cast<QGraphicsSceneMouseEvent *>(event);
        m_dragging = false;
        icon->setZValue(10);
        icon->removeEventFilter(this);
        icon->setPos(icon->mapToItem(this, QPoint(0,0)));
        icon->setParentItem(this);

        QModelIndex index = m_itemToIndex.value(icon);
        if (index.isValid()) {
            emit itemAskedReorder(index, mapFromScene(me->scenePos()));
        }

        m_spacerIndex = -1;
        m_draggingIndex = QModelIndex();
        askRelayout();
    }

    return false;
}


int ItemContainer::rowForPosition(const QPointF &point)
{
    const int nColumns = qMax(1, (int)ceil(size().width() / m_cellSize.width()));
    const int nRows = qMax(1, (int)ceil(size().height() / m_cellSize.height()));

    int row = qMin(nRows-1, (int)round(point.y()/m_cellSize.height()));
    int column = qMin(nColumns-1, (int)round(point.x()/m_cellSize.width()));

    kDebug() << "The item will be put at" << row;

    int modelRow = qMin(m_model->rowCount(), row*nColumns + qBound(0, column, nColumns));

    kDebug() << "Corresponding to the model row" << modelRow;

    return modelRow;
}

void ItemContainer::focusInEvent(QFocusEvent *event)
{
    Q_UNUSED(event)

    if (m_model->rowCount() > 0 && m_currentIconIndexX == -1) {
        m_currentIconIndexX = 0;
        m_currentIconIndexY = 0;
        QModelIndex index = m_model->index(0, 0, m_rootIndex);
        ResultWidget *item = m_items.value(index);
        emit itemSelected(item);
        setCurrentItem(item);
    } else {
        setCurrentItem(m_currentIcon.data());
    }
}

void ItemContainer::focusOutEvent(QFocusEvent *event)
{
    Q_UNUSED(event)

    m_hoverIndicator->hide();
}

void ItemContainer::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event)

    if (!hasFocus()) {
        m_hoverIndicator->hide();
    }
}

void ItemContainer::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    QGraphicsWidget *pw = parentWidget();
    if (pw) {
        QRectF parentRect = pw->boundingRect();
        QPointF newPos(pos());
        if (size().width() < parentRect.size().width()) {
            newPos.setX(parentRect.center().x() - size().width()/2);
        } else {
            newPos.setX(qMin(pos().x(), (qreal)0.0));
        }
        if (size().height() < parentRect.size().height()) {
            newPos.setY(parentRect.center().y() - size().height()/2);
        } else {
            newPos.setY(qMin(pos().y(), (qreal)0.0));
        }
        if (m_positionAnimation->state() == QAbstractAnimation::Running) {
            m_positionAnimation->stop();
        }
        if (m_firstRelayout) {
            setPos(newPos.toPoint());
        } else {
            m_positionAnimation->setEndValue(newPos.toPoint());
            m_positionAnimation->start();
        }
    }

    if ((m_orientation == Qt::Vertical && !qFuzzyCompare(event->newSize().width(), event->oldSize().width())) ||
        (m_orientation == Qt::Horizontal && !qFuzzyCompare(event->newSize().height(), event->oldSize().height()))) {
        m_relayoutTimer->start(300);
    }
}

void ItemContainer::setModel(QAbstractItemModel *model)
{
    if (m_model) {
        disconnect(m_model, 0, this, 0);
        reset();
    }

    m_model = model;
    connect (m_model, SIGNAL(modelAboutToBeReset()), this, SLOT(reset()));
    connect (m_model, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(generateItems(QModelIndex,int,int)));
    connect (m_model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)), this, SLOT(removeItems(QModelIndex,int,int)));

    generateItems(m_rootIndex, 0, m_model->rowCount());
}

QAbstractItemModel *ItemContainer::model() const
{
    return m_model;
}

void ItemContainer::reset()
{
    m_hoverIndicator->setTargetItem(0);


    foreach (ResultWidget *icon, m_items) {
        disposeItem(icon);
    }
    m_items.clear();
    m_itemToIndex.clear();
    askRelayout();
}

void ItemContainer::generateItems(const QModelIndex &parent, int start, int end)
{
    if (parent != m_rootIndex) {
        return;
    }

    for (int i = start; i <= end; i++) {
        QModelIndex index = m_model->index(i, 0, m_rootIndex);

        if (index.isValid()) {
            ResultWidget *icon = createItem(index);

            m_items.insert(QPersistentModelIndex(index), icon);
            m_itemToIndex.insert(icon, QPersistentModelIndex(index));
        }
    }
    m_relayoutTimer->start(500);
}

void ItemContainer::actionTriggered()
{
    ResultWidget *icon = static_cast<ResultWidget*>(sender()->parent());
    QModelIndex index = m_itemToIndex.value(icon);

    CommonModel::ActionType actionType = (CommonModel::ActionType)index.data(CommonModel::ActionTypeRole).value<int>();

    if (actionType == CommonModel::RemoveAction) {
        m_model->removeRow(index.row());
    } else if (actionType == CommonModel::AddAction) {
        emit addActionTriggered(index);
    }
}

void ItemContainer::removeItems(const QModelIndex &parent, int start, int end)
{
    if (m_rootIndex != parent) {
        return;
    }

    for (int i = start; i <= end; i++) {
        QModelIndex index = m_model->index(i, 0, m_rootIndex);
        ResultWidget *icon = m_items.value(index);
        disposeItem(icon);
        m_items.remove(index);
        m_itemToIndex.remove(icon);
    }
    m_relayoutTimer->start(500);
}

void ItemContainer::itemClicked()
{
    ResultWidget *icon = qobject_cast<ResultWidget *>(sender());

    if (icon) {
        QModelIndex i = m_itemToIndex.value(icon);
        if (i.isValid()) {
            QModelIndex index = m_model->index(i.row(), 0, m_rootIndex);
            emit itemActivated(index);
        }
    }
}

void ItemContainer::setRootIndex(QModelIndex index)
{
    m_rootIndex = index;
    reset();
}

QModelIndex ItemContainer::rootIndex() const
{
    return m_rootIndex;
}

void ItemContainer::hideUsedItems()
{
    QMapIterator<int, ResultWidget *> i(m_usedItems);
    while (i.hasNext()) {
        i.next();
        foreach (ResultWidget *icon, m_usedItems.values(i.key())) {
            icon->animateHide();
        }
    }
}

#include <itemcontainer.moc>
