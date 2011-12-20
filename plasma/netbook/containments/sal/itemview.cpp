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

#include "itemview.h"
#include "itemcontainer.h"
#include "resultwidget.h"

#include <QGraphicsSceneResizeEvent>
#include <QGraphicsScene>
#include <QTimer>


ItemView::ItemView(QGraphicsWidget *parent)
    : Plasma::ScrollWidget(parent)
{
    setFocusPolicy(Qt::StrongFocus);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_itemContainer = new ItemContainer(this);
    setAlignment(Qt::AlignCenter);
    setWidget(m_itemContainer);
    m_noActivateTimer = new QTimer(this);
    m_noActivateTimer->setSingleShot(true);
    m_itemContainer->installEventFilter(this);

    connect(m_itemContainer, SIGNAL(itemSelected(ResultWidget*)), this, SIGNAL(itemSelected(ResultWidget*)));
    connect(m_itemContainer, SIGNAL(itemActivated(QModelIndex)), this, SIGNAL(itemActivated(QModelIndex)));
    connect(m_itemContainer, SIGNAL(resetRequested()), this, SIGNAL(resetRequested()));
    connect(m_itemContainer, SIGNAL(itemSelected(ResultWidget*)), this, SLOT(selectItem(ResultWidget*)));
    connect(m_itemContainer, SIGNAL(itemAskedReorder(QModelIndex,QPointF)), this, SIGNAL(itemAskedReorder(QModelIndex,QPointF)));
    connect(m_itemContainer, SIGNAL(dragStartRequested(QModelIndex)), this, SIGNAL(dragStartRequested(QModelIndex)));

    connect(m_itemContainer, SIGNAL(addActionTriggered(QModelIndex)), this, SIGNAL(addActionTriggered(QModelIndex)));
}

ItemView::~ItemView()
{}

void ItemView::selectItem(ResultWidget *icon)
{
    if (!m_noActivateTimer->isActive()) {
        ensureItemVisible(icon);
    }
}

void ItemView::setCurrentItem(ResultWidget *currentIcon)
{
    m_itemContainer->setCurrentItem(currentIcon);
}

ResultWidget *ItemView::currentItem() const
{
    return m_itemContainer->currentItem();
}

void ItemView::focusInEvent(QFocusEvent *event)
{
    m_itemContainer->setFocus();
    Plasma::ScrollWidget::focusInEvent(event);
}


void ItemView::setOrientation(Qt::Orientation orientation)
{
    m_itemContainer->setOrientation(orientation);
}

Qt::Orientation ItemView::orientation() const
{
    return m_itemContainer->orientation();
}

void ItemView::setIconSize(int size)
{
    m_itemContainer->setIconSize(size);
}

QList<ResultWidget *> ItemView::items() const
{
    return m_itemContainer->items();
}

int ItemView::iconSize() const
{
    return m_itemContainer->iconSize();
}

void ItemView::setDragAndDropMode(ItemContainer::DragAndDropMode mode)
{
    m_itemContainer->setDragAndDropMode(mode);
}

ItemContainer::DragAndDropMode ItemView::dragAndDropMode() const
{
    return m_itemContainer->dragAndDropMode();
}

void ItemView::setScrollPositionFromDragPosition(const QPointF &point)
{
    const qreal xRatio = point.x() / size().width();
    const qreal yRatio = point.y() / size().height();

    QPointF newPos(scrollPosition());

    if (size().width() < contentsSize().width()) {
        qreal newXPos = xRatio * (size().width() - contentsSize().width());
        newPos.setX(qBound(qreal(0.0), -newXPos, contentsSize().width() - viewportGeometry().width()));
    }
    if (size().height() < contentsSize().height()) {
        newPos.setY(-(yRatio * (size().height() - contentsSize().height())));
    }

    setScrollPosition(newPos);
}

void ItemView::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    QRectF rect = boundingRect();
    QPointF newPos(m_itemContainer->pos());
    if (m_itemContainer->size().width() < rect.size().width()) {
        newPos.setX(rect.center().x() - m_itemContainer->size().width()/2);
    } else {
        newPos.setX(qMin(m_itemContainer->pos().x(), (qreal)0.0));
    }
    if (m_itemContainer->size().height() < rect.size().height()) {
        newPos.setY(rect.center().y() - m_itemContainer->size().height()/2);
    } else {
        newPos.setY(qMin(m_itemContainer->pos().y(), (qreal)0.0));
    }
    m_itemContainer->setPos(newPos.toPoint());
    m_itemContainer->askRelayout();

    Plasma::ScrollWidget::resizeEvent(event);
}

bool ItemView::eventFilter(QObject *watched, QEvent *event)
{
    ResultWidget *icon = qobject_cast<ResultWidget *>(watched);
    if (icon && event->type() == QEvent::GraphicsSceneHoverEnter) {
        if (icon) {
            m_itemContainer->setCurrentItem(icon);
        }
    } else if (watched == m_itemContainer && event->type() == QEvent::GraphicsSceneResize) {

        ScrollBarFlags scrollBars = NoScrollBar;
        if (m_itemContainer->pos().x() < 0 || m_itemContainer->geometry().right() > size().width()) {
            scrollBars |= HorizontalScrollBar;
        }
        if (m_itemContainer->pos().y() < 0 || m_itemContainer->geometry().bottom() > size().height()) {
            scrollBars |= VerticalScrollBar;
        }
        emit scrollBarsNeededChanged(scrollBars);
    } else if (watched == m_itemContainer && event->type() == QEvent::GraphicsSceneMove) {
        m_noActivateTimer->start(300);
        ScrollBarFlags scrollBars = NoScrollBar;
        if (m_itemContainer->pos().x() < 0 || m_itemContainer->geometry().right() > size().width()) {
            scrollBars |= HorizontalScrollBar;
        }
        if (m_itemContainer->pos().y() < 0 || m_itemContainer->geometry().bottom() > size().height()) {
            scrollBars |= VerticalScrollBar;
        }
        emit scrollBarsNeededChanged(scrollBars);
    }

    return Plasma::ScrollWidget::eventFilter(watched, event);
}

void ItemView::showSpacer(const QPointF &pos)
{
    m_itemContainer->showSpacer(pos);
}

void ItemView::setModel(QAbstractItemModel *model)
{
    m_itemContainer->setModel(model);
}

QAbstractItemModel *ItemView::model() const
{
    return m_itemContainer->model();
}

void ItemView::setRootIndex(QModelIndex index)
{
    m_itemContainer->setRootIndex(index);
}

QModelIndex ItemView::rootIndex() const
{
    return m_itemContainer->rootIndex();
}

int ItemView::rowForPosition(const QPointF &point)
{
    return m_itemContainer->rowForPosition(point);
}

#include <itemview.moc>
