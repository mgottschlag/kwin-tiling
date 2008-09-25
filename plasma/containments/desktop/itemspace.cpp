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
            performPush(i, DirRight, push, 0, (spaceAlignment & Qt::AlignLeft));
        }

        // right border
        push = item.lastGeometry.right()+screenSpacing - workingGeom.width();
        if (push > 0) {
            performPush(i, DirLeft, push, 0, (spaceAlignment & Qt::AlignRight));
        }

        // top border
        push = screenSpacing - item.lastGeometry.top();
        if (push > 0) {
            performPush(i, DirDown, push, 0, (spaceAlignment & Qt::AlignTop));
        }

        // bottom border
        push = item.lastGeometry.bottom()+screenSpacing - workingGeom.height();
        if (push > 0) {
            performPush(i, DirUp, push, 0, (spaceAlignment & Qt::AlignBottom));
        }

        /*
          Push items back towards their perferred positions.
          Push is limited by working area borders.
        */
        if (item.pushBack) {
            // left/right
            push = item.preferredGeometry.left() - item.lastGeometry.left();
            if (push > 0) {
                performPush(i, DirRight, push, 0, false);
            } else if (push < 0) {
                performPush(i, DirLeft, -push, 0, false);
            }
            // up/down
            push = item.preferredGeometry.top() - item.lastGeometry.top();
            if (push > 0) {
                performPush(i, DirDown, push, 0, false);
            } else if (push < 0) {
                performPush(i, DirUp, -push, 0, false);
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

qreal ItemSpace::performPush(int itemIndex, Direction direction, qreal amount, qreal minAmount, bool ignoreBorder)
{
    QList<int> previous;
    qreal canPush = pushItem(itemIndex, direction, amount, &previous, false, ignoreBorder);
    if (canPush >= minAmount) {
        previous = QList<int>();
        //FIXME: we are calling pushItem twice here; it should call once, cache the results
        //       and then perform the push
        pushItem(itemIndex, direction, canPush, &previous, true, ignoreBorder);
        return canPush;
    }
    return 0;
}

void ItemSpace::findPullGroup(int thisItem, QList<int> *currentItems)
{
    QRectF origGeom = items[thisItem].lastGeometry;
    QRectF fullGeom = origGeom.adjusted(-shiftingSpacing, -shiftingSpacing, shiftingSpacing, shiftingSpacing);
    currentItems->append(thisItem);
    for (int i=0; i<items.size(); i++) {
        if (currentItems->contains(i)) {
            continue;
        }
        ItemSpaceItem &item = items[i];
        if (item.lastGeometry.intersects(fullGeom)) {
            findPullGroup(i, currentItems);
        }
    }
}

qreal ItemSpace::pushItem(int itemIndex, Direction direction, qreal amount, const QList<int> *previousItems, bool doPush, bool ignoreBorder)
{
    QList<int> pullGroup;

    // find all intersecting items
    findPullGroup(itemIndex, &pullGroup);

    // create a list of previous items for use when recursing
    QList<int> currentItems = *previousItems;
    foreach (const int i, pullGroup) {
        currentItems.append(i);
    }

    // look for obstacles for every item in the group
    foreach (const int i, pullGroup) {
        ItemSpaceItem &groupItem = items[i];
        QRectF origGeom = groupItem.lastGeometry;
        QRectF fullGeom = origGeom.adjusted(-shiftingSpacing, -shiftingSpacing, shiftingSpacing, shiftingSpacing);

        // limit push by screen boundaries
        if (!ignoreBorder) {
            qreal limit;
            switch (direction) {
                case DirLeft:
                    limit = origGeom.left() - screenSpacing;
                    break;
                case DirRight:
                    limit = workingGeom.width() - screenSpacing - origGeom.right();
                    break;
                case DirUp:
                    limit = origGeom.top() - screenSpacing;
                    break;
                case DirDown:
                    limit = workingGeom.height() - screenSpacing - origGeom.bottom();
                    break;
            }
            amount = qMin(amount, limit);
            if (amount <= 0) {
                return 0;
            }
        }

        // look for items in the way
        for (int j = 0; j < items.size(); ++j) {
            if (currentItems.contains(j)) {
                continue;
            }
            ItemSpaceItem &item = items[j];

            QRectF newlyTakenSpace;
            qreal push;
            switch (direction) {
                case DirLeft:
                    newlyTakenSpace = QRectF(fullGeom.left()-amount, fullGeom.top(), amount, fullGeom.height());
                    push = item.lastGeometry.right()-newlyTakenSpace.left();
                    break;
                case DirRight:
                    newlyTakenSpace = QRectF(fullGeom.right(), fullGeom.top(), amount, fullGeom.height());
                    push = newlyTakenSpace.right()-item.lastGeometry.left();
                    break;
                case DirUp:
                    newlyTakenSpace = QRectF(fullGeom.left(), fullGeom.top()-amount, fullGeom.width(), amount);
                    push = item.lastGeometry.bottom()-newlyTakenSpace.top();
                    break;
                case DirDown:
                    newlyTakenSpace = QRectF(fullGeom.left(), fullGeom.bottom(), fullGeom.width(), amount);
                    push = newlyTakenSpace.bottom()-item.lastGeometry.top();
                    break;
            }

            // check if the item is in the way
            if (item.lastGeometry.intersects(newlyTakenSpace)) {
                // try to push the item, limit our push by the result
                qreal pushed = pushItem(j, direction, push, &currentItems, doPush, ignoreBorder);
                if (pushed < push) {
                    amount -= push-pushed;
                    if (amount <= 0) {
                        return 0;
                    }
                }
            }
        }
    }

    // move items in the group
    if (doPush) {
        foreach (const int i, pullGroup) {
            ItemSpaceItem &groupItem = items[i];
            switch (direction) {
                case DirLeft:
                    groupItem.lastGeometry = groupItem.lastGeometry.adjusted(-amount, 0, -amount, 0);
                    break;
                case DirRight:
                    groupItem.lastGeometry = groupItem.lastGeometry.adjusted(amount, 0, amount, 0);
                    break;
                case DirUp:
                    groupItem.lastGeometry = groupItem.lastGeometry.adjusted(0, -amount, 0, -amount);
                    break;
                case DirDown:
                    groupItem.lastGeometry = groupItem.lastGeometry.adjusted(0, amount, 0, amount);
                    break;
            }
        }
    }

    return amount;
}

bool ItemSpace::positionedProperly(QRectF itemGeom)
{
    QRectF fullGeom = itemGeom.adjusted(-placementSpacing, -placementSpacing, placementSpacing, placementSpacing);
    return (QRectF(QPointF(), workingGeom).contains(fullGeom));
}

// TODO: explain stuff
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

    QSizeF size = QSizeF(itemSize.width()+spL+spR, itemSize.height()+spT+spB);

    qreal x = ((align & Qt::AlignLeft) ? 0 : workingGeom.width()-size.width());
    while (1) {
        bool outOfX = ((align & Qt::AlignLeft) ? (x + size.width() > workingGeom.width()) : (x < 0));
        if (outOfX && limitedSpace) {
            break;
        }

        qreal y = ((align & Qt::AlignTop) ? 0 : workingGeom.height()-size.height());
        while (1) {
            bool outOfY = ((align & Qt::AlignTop) ? (y + size.height() > workingGeom.height()) : (y < 0));
            if (outOfY && limitedSpace) {
                break;
            }

            QRectF a;
            if ((align & Qt::AlignTop)) {
                a = itemInRegionEndingLastVert(QRectF(x, y, size.width(), size.height()));
            } else {
                a = itemInRegionStartingFirstVert(QRectF(x, y, size.width(), size.height()));
            }

            if (!a.isValid()) {
                possiblePositions.append(QPointF(x+spL, y+spT));
                if (!findAll) {
                    return possiblePositions;
                }
                break;
            }

            y = ((align & Qt::AlignTop) ? a.bottom() : a.y() - size.height());
        }

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
