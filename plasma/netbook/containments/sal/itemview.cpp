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

#include <QGraphicsSceneResizeEvent>
#include <QGraphicsScene>
#include <QTimer>

#include <Plasma/IconWidget>

ItemView::ItemView(QGraphicsWidget *parent)
    : Plasma::ScrollWidget(parent)
{
    setFocusPolicy(Qt::StrongFocus);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_itemContainer = new ItemContainer(this);
    setWidget(m_itemContainer);
    m_noActivateTimer = new QTimer(this);
    m_noActivateTimer->setSingleShot(true);
    m_itemContainer->installEventFilter(this);

    connect(m_itemContainer, SIGNAL(itemSelected(Plasma::IconWidget *)), this, SIGNAL(itemSelected(Plasma::IconWidget *)));
    connect(m_itemContainer, SIGNAL(itemActivated(Plasma::IconWidget *)), this, SIGNAL(itemActivated(Plasma::IconWidget *)));
    connect(m_itemContainer, SIGNAL(resetRequested()), this, SIGNAL(resetRequested()));
    connect(m_itemContainer, SIGNAL(itemSelected(Plasma::IconWidget *)), this, SLOT(selectItem(Plasma::IconWidget *)));
    connect(m_itemContainer, SIGNAL(itemReordered(Plasma::IconWidget *, int)), this, SIGNAL(itemReordered(Plasma::IconWidget *, int)));
    connect(m_itemContainer, SIGNAL(dragStartRequested(Plasma::IconWidget *)), this, SIGNAL(dragStartRequested(Plasma::IconWidget *)));

    connect(m_itemContainer, SIGNAL(dragMoveMouseMoved(const QPointF &)), this, SLOT(setScrollPositionFromDragPosition(const QPointF &)));
}

ItemView::~ItemView()
{}

void ItemView::selectItem(Plasma::IconWidget *icon)
{
    if (!m_noActivateTimer->isActive()) {
        ensureItemVisible(icon);
    }
}

void ItemView::setCurrentItem(Plasma::IconWidget *currentIcon)
{
    m_itemContainer->setCurrentItem(currentIcon);
}

Plasma::IconWidget *ItemView::currentItem() const
{
    return m_itemContainer->currentItem();
}

void ItemView::focusInEvent(QFocusEvent *event)
{
    m_itemContainer->setFocus();
    Plasma::ScrollWidget::focusInEvent(event);
}

void ItemView::insertItem(Plasma::IconWidget *icon, qreal weight)
{
    m_itemContainer->insertItem(icon, weight);
    icon->installEventFilter(this);
    registerAsDragHandle(icon);
}

void ItemView::addItem(Plasma::IconWidget *icon)
{
    m_itemContainer->addItem(icon);
    icon->installEventFilter(this);
    registerAsDragHandle(icon);
}

void ItemView::clear()
{
    Plasma::Animator::self()->stopScrollingWidget(this);
    QList<Plasma::IconWidget *>items = m_itemContainer->items();
    foreach (Plasma::IconWidget *item, items) {
        unregisterAsDragHandle(item);
    }
    m_itemContainer->clear();
}

int ItemView::count() const
{
    return m_itemContainer->count();
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

Plasma::IconWidget *ItemView::createItem()
{
    return m_itemContainer->createItem();
}

qreal ItemView::positionToWeight(const QPointF &point)
{
    return m_itemContainer->positionToWeight(point);
}

void ItemView::setScrollPositionFromDragPosition(const QPointF &point)
{
    const qreal xRatio = point.x() / size().width();
    const qreal yRatio = point.y() / size().height();

    QPointF newPos(scrollPosition());

    if (size().width() < contentsSize().width()) {
        qreal newXPos = xRatio * (size().width() - contentsSize().width());
        newPos.setX(qBound(0.0, -newXPos, contentsSize().width() - viewportGeometry().width()));
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
    Plasma::IconWidget *icon = qobject_cast<Plasma::IconWidget *>(watched);
    if (icon && event->type() == QEvent::GraphicsSceneHoverEnter) {
        if (icon) {
            m_itemContainer->setCurrentItem(icon);
        }
    } else if (watched == m_itemContainer && event->type() == QEvent::GraphicsSceneResize) {
        QGraphicsSceneResizeEvent *re = static_cast<QGraphicsSceneResizeEvent *>(event);

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

#include <itemview.moc>
