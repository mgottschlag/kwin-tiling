/*
  Copyright (c) 2008 Ambroz Bizjak <ambro@b4ever.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

#include <limits>

#include "itemspace.h"

ItemSpace::ItemSpace ()
  : spaceAlignment(Qt::AlignTop|Qt::AlignLeft),
    workingGeom(QSizeF()),
    placementSpacing(0),
    screenSpacing(0),
    shiftingSpacing(0)
{
}

void ItemSpace::addItem(bool pushBack, const QRectF &preferredGeom, const QRectF &lastGeom)
{
    ItemSpaceItem newItem;
    newItem.pushBack = pushBack;
    newItem.animateMovement = false;
    newItem.preferredGeometry = preferredGeom;
    newItem.lastGeometry = (lastGeom.isValid() ? lastGeom : preferredGeom);
    items.append(newItem);
}

void ItemSpace::removeAt (int itemIndex)
{
    items.removeAt(itemIndex);
}

int ItemSpace::count ()
{
    return items.count();
}

void ItemSpace::setWorkingArea (QSizeF area)
{
    if (workingGeom.isValid()) {
        // if the working area size changed and alignment includes right or bottom,
        // the difference is added to all positions to keep items in the same place
        // relative to the borders of alignment
        if (((spaceAlignment & Qt::AlignRight) || (spaceAlignment & Qt::AlignBottom)) &&
            (area.width() != workingGeom.width() || area.height() != workingGeom.height())) {
            offsetPositions(QPointF(area.width()-workingGeom.width(), area.height()-workingGeom.height()));
        }
    }
    workingGeom = area;
}

void ItemSpace::activate()
{
    for (int i=0; i<items.size(); i++) {
        ItemSpaceItem &item = items[i];

        qreal push;

        /*
          Push items intersecting working area borders inside.
          For borders adjunct to the corner of alignment, allow pushing
          over the opposite border (and items there may be temporarily placed).
        */

        // left border
        push = screenSpacing - item.lastGeometry.left();
        if (push > 0) {
            item.animateMovement = true;
            performPush(i, DirRight, push, (spaceAlignment & Qt::AlignLeft));
        }

        // right border
        push = item.lastGeometry.right()+screenSpacing - workingGeom.width();
        if (push > 0) {
            item.animateMovement = true;
            performPush(i, DirLeft, push, (spaceAlignment & Qt::AlignRight));
        }

        // top border
        push = screenSpacing - item.lastGeometry.top();
        if (push > 0) {
            item.animateMovement = true;
            performPush(i, DirDown, push, (spaceAlignment & Qt::AlignTop));
        }

        // bottom border
        push = item.lastGeometry.bottom()+screenSpacing - workingGeom.height();
        if (push > 0) {
            item.animateMovement = true;
            performPush(i, DirUp, push, (spaceAlignment & Qt::AlignBottom));
        }

        /*
          Push items back towards their perferred positions.
          Push is limited by working area borders.
        */
        if (item.pushBack) {
            // left/right
            push = item.preferredGeometry.left() - item.lastGeometry.left();
            if (push > 0) {
                performPush(i, DirRight, push, false);
            } else if (push < 0) {
                performPush(i, DirLeft, -push, false);
            }
            // up/down
            push = item.preferredGeometry.top() - item.lastGeometry.top();
            if (push > 0) {
                performPush(i, DirDown, push, false);
            } else if (push < 0) {
                performPush(i, DirUp, -push, false);
            }
        }
    }
}

qreal ItemSpace::positionVisibility (int itemIndex)
{
    ItemSpaceItem &item = items[itemIndex];
    QRectF visibleArea = QRectF(QPointF(), workingGeom);
    QRectF visibleItemPart = visibleArea.intersected(item.lastGeometry);
    qreal itemSurface = item.lastGeometry.width() * item.lastGeometry.height();
    qreal itemVisibleSurface = visibleItemPart.width() * visibleItemPart.height();
    return (itemVisibleSurface / itemSurface);
}

void ItemSpace::offsetPositions(const QPointF &offset)
{
    for (int i=0; i<items.size(); i++) {
        ItemSpaceItem &item = items[i];
        item.preferredGeometry.adjust(offset.x(), offset.y(), offset.x(), offset.y());
        item.lastGeometry.adjust(offset.x(), offset.y(), offset.x(), offset.y());
    }
}

qreal ItemSpace::performPush(int itemIndex, Direction direction, qreal amount, bool ignoreBorder)
{
    // perform movement calculation by creating the root item in the movement structure
    PushTreeItem rootItem(this, NULL, 0, itemIndex, direction, amount, ignoreBorder);
    rootItem.applyResults(this, direction);
    return rootItem.m_pushAvailable;
}

bool ItemSpace::positionedProperly(QRectF itemGeom)
{
    QRectF fullGeom = itemGeom.adjusted(-placementSpacing, -placementSpacing, placementSpacing, placementSpacing);
    return (QRectF(QPointF(), workingGeom).contains(fullGeom));
}

QList<QPointF> ItemSpace::positionVertically(
    const QSizeF &itemSize,
    Qt::Alignment align,
    bool limitedSpace,
    bool findAll
) const
{
    qreal spL = placementSpacing;
    qreal spR = placementSpacing;
    qreal spT = placementSpacing;
    qreal spB = placementSpacing;
    QList<QPointF> possiblePositions;

    // basically, position searching is done by repetedly looking for obstacles at
    // one position and looking for a next position to try

    // add spacing to the size
    QSizeF size = QSizeF(itemSize.width()+spL+spR, itemSize.height()+spT+spB);

    // the initial x coordinate to start testing
    // start either on the left or the right, and advance inside
    qreal x = ((align & Qt::AlignLeft) ? 0 : workingGeom.width()-size.width());
    // try different x coordinates
    while (1) {
        // stop testing if we're limited by the working area and positions at the next x would reach outside
        bool outOfX = ((align & Qt::AlignLeft) ? (x + size.width() > workingGeom.width()) : (x < 0));
        if (outOfX && limitedSpace) {
            break;
        }

        // the initial y coordinate to start testing heights at the current x
        // start either on the top or the bottom, and advance inside
        qreal y = ((align & Qt::AlignTop) ? 0 : workingGeom.height()-size.height());
        // try different y coordinates
        while (1) {
            // stop testing at this x if we're limited by the working area and positions at the next y would reach outside
            bool outOfY = ((align & Qt::AlignTop) ? (y + size.height() > workingGeom.height()) : (y < 0));
            if (outOfY && limitedSpace) {
                break;
            }

            // Z would come here :)

            // Check for intersecting items, or a new y coordinate to try.
            // Suppose we're aligning to top:
            // Find all items that intersect the region we're testing.
            // If no items were found, we have space.
            // Oterwise pick the one with the lowest bottom border
            // and use that border as the new y coordinate to try.
            // The logic is inverted when aligning to bottom.

            QRectF a;
            if ((align & Qt::AlignTop)) {
                a = itemInRegionEndingLastVert(QRectF(x, y, size.width(), size.height()));
            } else {
                a = itemInRegionStartingFirstVert(QRectF(x, y, size.width(), size.height()));
            }

            if (!a.isValid()) {
                // found a valid position
                possiblePositions.append(QPointF(x+spL, y+spT));
                if (!findAll) {
                    return possiblePositions;
                }
                // don't look at this X anymore, one position is enough
                break;
            }

            y = ((align & Qt::AlignTop) ? a.bottom() : a.y() - size.height());
        }

        // Find next possible x coordinate
        // Suppose we're aligning to left:
        // Take a vertical strap of the area we have been testing previously,
        // extending over the height of the working area.
        // Find all items that intersect the region we're testing.
        // If no items were found, stop all testing.
        // Otherwise, pick the one with the most-left right border
        // and use that border as the new x coordinate to try.
        // The logic is inverted when aligning to right.

        QRectF a;
        if ((align & Qt::AlignLeft)) {
            a = itemInRegionEndingFirstHoriz(QRectF(x, 0, size.width(), workingGeom.height()));
        } else {
            a = itemInRegionStartingLastHoriz(QRectF(x, 0, size.width(), workingGeom.height()));
        }

        if (!a.isValid()) {
            break;
        }

        x = ((align & Qt::AlignLeft) ? a.right() : a.x() - size.width());
    }

    return possiblePositions;
}

QRectF ItemSpace::itemInRegionStartingFirstHoriz(const QRectF &region) const
{
    QRectF ret = QRectF(0,0,-1,-1);
    qreal l = std::numeric_limits<qreal>::max();
    for (int i=0; i<items.count(); i++) {
      ItemSpaceItem item = items[i];
      if (!item.lastGeometry.isValid()) {
        continue;
      }
      qreal cl = item.lastGeometry.x();
      if (item.lastGeometry.intersects(region) && cl < l) {
          ret = item.lastGeometry;
          l = cl;
      }
    }
    return ret;
}

QRectF ItemSpace::itemInRegionEndingLastHoriz(const QRectF &region) const
{
    QRectF ret = QRectF(0,0,-1,-1);
    qreal l = -1;
    for (int i=0; i<items.count(); i++) {
      ItemSpaceItem item = items[i];
      if (!item.lastGeometry.isValid()) {
        continue;
      }
      qreal cl = item.lastGeometry.x() + item.lastGeometry.width();
      if (item.lastGeometry.intersects(region) && cl > l) {
          ret = item.lastGeometry;
          l = cl;
      }
    }
    return ret;
}

QRectF ItemSpace::itemInRegionEndingFirstVert(const QRectF &region) const
{
    QRectF ret = QRectF(0,0,-1,-1);
    qreal l = std::numeric_limits<qreal>::max();
    for (int i=0; i<items.count(); i++) {
      ItemSpaceItem item = items[i];
      if (!item.lastGeometry.isValid()) {
        continue;
      }
      qreal cl = item.lastGeometry.y() + item.lastGeometry.height();
      if (item.lastGeometry.intersects(region) && cl < l) {
          ret = item.lastGeometry;
          l = cl;
      }
    }
    return ret;
}

QRectF ItemSpace::itemInRegionStartingLastVert(const QRectF &region) const
{
    QRectF ret = QRectF(0,0,-1,-1);
    qreal l = -1;
    for (int i=0; i<items.count(); i++) {
      ItemSpaceItem item = items[i];
      if (!item.lastGeometry.isValid()) {
        continue;
      }
      qreal cl = item.lastGeometry.y();
      if (item.lastGeometry.intersects(region) && cl > l) {
          ret = item.lastGeometry;
          l = cl;
      }
    }
    return ret;
}

QRectF ItemSpace::itemInRegionStartingFirstVert(const QRectF &region) const
{
    QRectF ret = QRectF(0,0,-1,-1);
    qreal l = std::numeric_limits<qreal>::max();
    for (int i=0; i<items.count(); i++) {
      ItemSpaceItem item = items[i];
      if (!item.lastGeometry.isValid()) {
        continue;
      }
      qreal cl = item.lastGeometry.y();
      if (item.lastGeometry.intersects(region) && cl < l) {
          ret = item.lastGeometry;
          l = cl;
      }
    }
    return ret;
}

QRectF ItemSpace::itemInRegionEndingLastVert(const QRectF &region) const
{
    QRectF ret = QRectF(0,0,-1,-1);
    qreal l = -1;
    for (int i=0; i<items.count(); i++) {
      ItemSpaceItem item = items[i];
      if (!item.lastGeometry.isValid()) {
        continue;
      }
      qreal cl = item.lastGeometry.y() + item.lastGeometry.height();
      if (item.lastGeometry.intersects(region) && cl > l) {
          ret = item.lastGeometry;
          l = cl;
      }
    }
    return ret;
}

QRectF ItemSpace::itemInRegionEndingFirstHoriz(const QRectF &region) const
{
    QRectF ret = QRectF(0,0,-1,-1);
    qreal l = std::numeric_limits<qreal>::max();
    for (int i=0; i<items.count(); i++) {
      ItemSpaceItem item = items[i];
      if (!item.lastGeometry.isValid()) {
        continue;
      }
      qreal cl = item.lastGeometry.x() + item.lastGeometry.width();
      if (item.lastGeometry.intersects(region) && cl < l) {
          ret = item.lastGeometry;
          l = cl;
      }
    }
    return ret;
}

QRectF ItemSpace::itemInRegionStartingLastHoriz(const QRectF &region) const
{
    QRectF ret = QRectF(0,0,-1,-1);
    qreal l = -1;
    for (int i=0; i<items.count(); i++) {
      ItemSpaceItem item = items[i];
      if (!item.lastGeometry.isValid()) {
        continue;
      }
      qreal cl = item.lastGeometry.x();
      if (item.lastGeometry.intersects(region) && cl > l) {
          ret = item.lastGeometry;
          l = cl;
      }
    }
    return ret;
}

ItemSpace::PushTreeItem::PushTreeItem(
    ItemSpace *itemSpace,
    PushTreeItem *parent,
    qreal parentPushRequested,
    int firstItem,
    Direction direction,
    qreal amount,
    bool ignoreBorder
)
  : m_pushAvailable(amount),
    m_parent(parent),
    m_parentPushRequested(parentPushRequested),
    m_pushRequested(amount)
{
    findGroup(itemSpace, firstItem);
    // HACK not using DAG
    m_initialBoundingRect = boundingRect(itemSpace);

    // look for obstacles for every item in the group
    foreach (const int i, m_groupItems) {
        ItemSpaceItem &groupItem = itemSpace->items[i];
        QRectF origGeom = groupItem.lastGeometry;
        QRectF fullGeom = origGeom.adjusted(-itemSpace->shiftingSpacing, -itemSpace->shiftingSpacing,
                                            itemSpace->shiftingSpacing, itemSpace->shiftingSpacing);

        // limit push by screen boundaries
        if (!ignoreBorder) {
            qreal limit;
            switch (direction) {
                case DirLeft:
                    limit = origGeom.left() - itemSpace->screenSpacing;
                    break;
                case DirRight:
                    limit = itemSpace->workingGeom.width() - itemSpace->screenSpacing - origGeom.right();
                    break;
                case DirUp:
                    limit = origGeom.top() - itemSpace->screenSpacing;
                    break;
                case DirDown:
                    limit = itemSpace->workingGeom.height() - itemSpace->screenSpacing - origGeom.bottom();
                    break;
            }
            m_pushAvailable = qMin(m_pushAvailable, limit);
            if (m_pushAvailable <= 0) {
                return;
            }
        }

        // look for items in the way
        for (int j = 0; j < itemSpace->items.size(); ++j) {
            if (itemIsCurrentOrAbove(j)) {
                continue;
            }
            ItemSpaceItem &item = itemSpace->items[j];

            QRectF newlyTakenSpace;
            qreal push;
            switch (direction) {
                case DirLeft:
                    newlyTakenSpace = QRectF(fullGeom.left()-m_pushAvailable, fullGeom.top(), m_pushAvailable, fullGeom.height());
                    push = item.lastGeometry.right()-newlyTakenSpace.left();
                    break;
                case DirRight:
                    newlyTakenSpace = QRectF(fullGeom.right(), fullGeom.top(), m_pushAvailable, fullGeom.height());
                    push = newlyTakenSpace.right()-item.lastGeometry.left();
                    break;
                case DirUp:
                    newlyTakenSpace = QRectF(fullGeom.left(), fullGeom.top()-m_pushAvailable, fullGeom.width(), m_pushAvailable);
                    push = item.lastGeometry.bottom()-newlyTakenSpace.top();
                    break;
                case DirDown:
                    newlyTakenSpace = QRectF(fullGeom.left(), fullGeom.bottom(), fullGeom.width(), m_pushAvailable);
                    push = newlyTakenSpace.bottom()-item.lastGeometry.top();
                    break;
            }

            // check if the item is in the way
            if (item.lastGeometry.intersects(newlyTakenSpace)) {
                // create the child, causing recursive calculation of available push
                PushTreeItem *child = new PushTreeItem(itemSpace, this, m_pushAvailable, j, direction, push, ignoreBorder);
                m_obstacles.append(child);

                // limit our push by how much the obstacle can actually move
                if (child->m_pushAvailable < push) {
                    m_pushAvailable -= push - child->m_pushAvailable;
                    if (m_pushAvailable <= 0) {
                        return;
                    }
                }
            }
        }
    }
}

ItemSpace::PushTreeItem::~PushTreeItem()
{
    // delete children recursively
    foreach (PushTreeItem *child, m_obstacles) {
        delete child;
    }
}

void ItemSpace::PushTreeItem::applyResults(ItemSpace *itemSpace, Direction direction)
{
    if (m_parent) {
        // the final wanted move of this group can be lower than what
        // was initially requested, as the group that asked us to move
        // could have had another obstacle that could not move
        // enough; thus compare how much the parent group wanted to move
        // when we were created, and how much it still wants to move now,
        // and take that into account
        qreal pushLost = m_parentPushRequested - m_parent->m_pushAvailable;
        m_pushRequested -= pushLost;
        m_pushAvailable = qMin(m_pushAvailable, m_pushRequested);
    }

    // HACK not using DAG
    QRectF currentBoundingRect = boundingRect(itemSpace);
    qreal alreadyMoved;
    switch (direction) {
        case DirLeft:
            alreadyMoved = m_initialBoundingRect.left() - currentBoundingRect.left();
            break;
        case DirRight:
            alreadyMoved = currentBoundingRect.left() - m_initialBoundingRect.left();
            break;
        case DirUp:
            alreadyMoved = m_initialBoundingRect.top() - currentBoundingRect.top();
            break;
        case DirDown:
            alreadyMoved = currentBoundingRect.top() - m_initialBoundingRect.top();
            break;
    }
    qreal toPush = m_pushAvailable - alreadyMoved;
    if (toPush > 0) {
        foreach (const int i, m_groupItems) {
            ItemSpaceItem &groupItem = itemSpace->items[i];
            switch (direction) {
                case DirLeft:
                    groupItem.lastGeometry = groupItem.lastGeometry.adjusted(-toPush, 0, -toPush, 0);
                    break;
                case DirRight:
                    groupItem.lastGeometry = groupItem.lastGeometry.adjusted(toPush, 0, toPush, 0);
                    break;
                case DirUp:
                    groupItem.lastGeometry = groupItem.lastGeometry.adjusted(0, -toPush, 0, -toPush);
                    break;
                case DirDown:
                    groupItem.lastGeometry = groupItem.lastGeometry.adjusted(0, toPush, 0, toPush);
                    break;
            }
        }
    }

    foreach (PushTreeItem *child, m_obstacles) {
        child->applyResults(itemSpace, direction);
    }
}

bool ItemSpace::PushTreeItem::itemIsCurrentOrAbove(int item)
{
    ItemSpace::PushTreeItem *node = this;
    while (node) {
        if (node->m_groupItems.contains(item)) {
            return true;
        }
        node = node->m_parent;
    }
    return false;
}

void ItemSpace::PushTreeItem::findGroup(ItemSpace *itemSpace, int thisItem)
{
    m_groupItems.append(thisItem);

    QRectF origGeom = itemSpace->items[thisItem].lastGeometry;
    QRectF fullGeom = origGeom.adjusted(-itemSpace->shiftingSpacing, -itemSpace->shiftingSpacing,
                                        itemSpace->shiftingSpacing, itemSpace->shiftingSpacing);

    for (int i = 0; i < itemSpace->items.size(); i++) {
        if (m_groupItems.contains(i)) {
            continue;
        }
        ItemSpaceItem &item = itemSpace->items[i];
        if (item.lastGeometry.intersects(fullGeom)) {
            findGroup(itemSpace, i);
        }
    }
}

QRectF ItemSpace::PushTreeItem::boundingRect(ItemSpace *itemSpace)
{
    QRectF rect;
    foreach (int i, m_groupItems) {
        ItemSpaceItem &item = itemSpace->items[i];
        QRectF origGeom = item.lastGeometry;
        QRectF fullGeom = origGeom.adjusted(-itemSpace->shiftingSpacing, -itemSpace->shiftingSpacing,
                                            itemSpace->shiftingSpacing, itemSpace->shiftingSpacing);
        rect = rect.united(fullGeom);
    }
    return rect;
}
