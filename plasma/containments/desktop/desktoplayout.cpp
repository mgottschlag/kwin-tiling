/*
  Copyright (c) 2008 Ambroz Bizjak <ambro@b4ever.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

#include <limits>

#include <QCoreApplication>
#include <QGraphicsWidget>
#include <QGraphicsProxyWidget>

#include <QWaitCondition>

#include <KDebug>

#include "desktoplayout.h"

DesktopLayout::DesktopLayout (QGraphicsLayoutItem *parent)
  : QGraphicsLayout(parent),
    screenGeom(QRectF(0, 0, -1, -1)),
    workingGeom(QRectF(0, 0, -1, -1)),
    previousWorkingGeom(QRectF(0, 0, -1, -1)),
    reassignPositions(false),
    autoWorkingArea(true),
    temporaryPlacement(false),
    layoutAlignment(Qt::AlignTop|Qt::AlignLeft),
    placementSpacing(0),
    screenSpacing(0),
    shiftingSpacing(0)
{
}

void DesktopLayout::addItem (QGraphicsLayoutItem *item, bool pushBack, const QRectF &preferredGeom, const QRectF &lastGeom)
{
    DesktopLayoutItem newItem;
    newItem.item = item;
    newItem.pushBack = pushBack;
    newItem.temporaryGeometry = QRectF(0, 0, -1, -1);
    newItem.preferredGeometry = preferredGeom;
    newItem.lastGeometry = (lastGeom.isValid() ? lastGeom : preferredGeom);
    items.append(newItem);

    //item->setGeometry(newItem.lastGeometry.translated(workingGeom.topLeft()));
    reassignPositions = true;
    invalidate();
}

void DesktopLayout::addItem (QGraphicsLayoutItem *item, bool pushBack, const QSizeF &size)
{
    QSizeF itemSize = ( size.isValid() ? size : item->effectiveSizeHint(Qt::PreferredSize) );
    QPointF newPos = positionVertically(itemSize, layoutAlignment, placementSpacing, placementSpacing, placementSpacing, placementSpacing);
    if (newPos == QPointF(-1, -1)) {
        newPos = QPointF(0, 0);
    }
    QRectF newGeom = QRectF(newPos, itemSize);
    addItem(item, pushBack, newGeom);
}

bool DesktopLayout::getPushBack (int index)
{
    return items[index].pushBack;
}

QRectF DesktopLayout::getPreferredGeometry (int index)
{
    return items[index].preferredGeometry;
}

QRectF DesktopLayout::getLastGeometry (int index)
{
    return items[index].lastGeometry;
}

void DesktopLayout::setPlacementSpacing (qreal spacing)
{
    placementSpacing = spacing;
}

void DesktopLayout::setScreenSpacing (qreal spacing)
{
    screenSpacing = spacing;
    invalidate();
}

void DesktopLayout::setShiftingSpacing (qreal spacing)
{
    shiftingSpacing = spacing;
    // NOTE: not wise to call that during operation yet
}

void DesktopLayout::setWorkingArea (QRectF area)
{
    if (workingGeom.isValid()) {
        // if the working area size changed and alignment includes right or bottom,
        // the difference is added to all positions to keep items in the same place
        // relative to the borders of alignment
        if (((layoutAlignment & Qt::AlignRight) || (layoutAlignment & Qt::AlignBottom)) &&
            (area.width() != workingGeom.width() || area.height() != workingGeom.height())) {
            offsetPositions(QPointF(area.width()-workingGeom.width(), area.height()-workingGeom.height()));
            reassignPositions = true;
        }
        if (area.x() != workingGeom.x() || area.y() != workingGeom.y()) {
            reassignPositions = true;
        }
    }
    workingGeom = area;
    invalidate();
}

void DesktopLayout::setAlignment (Qt::Alignment alignment)
{
    layoutAlignment = alignment;
    invalidate();
}

void DesktopLayout::setTemporaryPlacement (bool enabled)
{
    temporaryPlacement = enabled;
    invalidate();
}

void DesktopLayout::setAutoWorkingArea (bool value)
{
    autoWorkingArea = value;
}

int DesktopLayout::count () const
{
    return items.size();
}

QGraphicsLayoutItem *DesktopLayout::itemAt (int i) const
{
    return items.at(i).item;
}

void DesktopLayout::removeAt (int i)
{
    items.removeAt(i);
    invalidate();
}

void DesktopLayout::offsetPositions (const QPointF &offset)
{
    for (int i=0; i<items.size(); i++) {
        DesktopLayoutItem &item = items[i];
        item.preferredGeometry.adjust(offset.x(), offset.y(), offset.x(), offset.y());
        item.lastGeometry.adjust(offset.x(), offset.y(), offset.x(), offset.y());
    }
}

void DesktopLayout::performTemporaryPlacement (int itemIndex)
{
    DesktopLayoutItem &item = items[itemIndex];
    QRectF origGeom = item.lastGeometry;
    item.lastGeometry = QRectF(0, 0, -1, -1);
    QPointF newPos = positionVertically(origGeom.size(), layoutAlignment, placementSpacing, placementSpacing, placementSpacing, placementSpacing);
    if (newPos == QPointF(-1, -1)) {
        newPos = QPointF(0, 0);
    }
    kDebug() << "Temp placing" << itemIndex << "to" << newPos;
    item.lastGeometry = origGeom;
    item.temporaryGeometry = QRectF(newPos, origGeom.size());
    item.item->setGeometry(item.temporaryGeometry.translated(workingGeom.topLeft()));
}

void DesktopLayout::revertTemporaryPlacement (int itemIndex)
{
    kDebug() << "Reverting temp placing" << itemIndex;
    DesktopLayoutItem &item = items[itemIndex];
    item.temporaryGeometry = QRectF(0, 0, -1, -1);
    item.item->setGeometry(item.lastGeometry.translated(workingGeom.topLeft()));
}

qreal DesktopLayout::performPush (int itemIndex, Direction direction, qreal amount, qreal minAmount, bool ignoreBorder)
{
    QList<int> previous;
    qreal canPush = pushItem(itemIndex, direction, amount, &previous, false, ignoreBorder);
    if (canPush >= minAmount) {
        previous = QList<int>();
        pushItem(itemIndex, direction, canPush, &previous, true, ignoreBorder);
        return canPush;
    }
    return 0;
}

// locate a group of intersecting items
// calls itself on new intersecting items
void DesktopLayout::findPullGroup (int thisItem, QList<int> *currentItems)
{
    QRectF origGeom = items[thisItem].lastGeometry;
    QRectF fullGeom = origGeom.adjusted(-shiftingSpacing, -shiftingSpacing, shiftingSpacing, shiftingSpacing);
    currentItems->append(thisItem);
    for (int i=0; i<items.size(); i++) {
        if (currentItems->contains(i)) {
            continue;
        }
        DesktopLayoutItem &item = items[i];
        if (item.lastGeometry.intersects(fullGeom)) {
            findPullGroup(i, currentItems);
        }
    }
}

/*
    Push an item in the specified direction.

    A group of intersecting items is treated as one item.
    Non-intersecting items on the way will be recursively pushed away.

    To perform a clean push:
    - call this with 'doPush' false, return value indicates how much the item can actually be pushed,
    - repeat the call with 'doPush' true and 'amount' no more than the previously returned value.
*/
qreal DesktopLayout::pushItem (int itemIndex, Direction direction, qreal amount, const QList<int> *previousItems, bool doPush, bool ignoreBorder)
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
        DesktopLayoutItem &groupItem = items[i];
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
        for (int j=0; j<items.size(); j++) {
            if (currentItems.contains(j)) {
                continue;
            }
            DesktopLayoutItem &item = items[j];

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
            DesktopLayoutItem &groupItem = items[i];
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
            if (!groupItem.temporaryGeometry.isValid()) {
                groupItem.item->setGeometry(groupItem.lastGeometry.translated(workingGeom.topLeft()));
            }
        }
    }

    return amount;
}

// update anything that needs updating
void DesktopLayout::setGeometry(const QRectF &rect)
{
    QGraphicsLayout::setGeometry(rect);

    if (autoWorkingArea || !workingGeom.isValid()) {
        setWorkingArea(rect);
    }

    for (int i=0; i<items.size(); i++) {
        DesktopLayoutItem &item = items[i];

        qreal push;

        /*
          Push items intersecting working area borders inside.
          For borders adjunct to the corner of alignment, allow pushing
          over the opposite border (and items there may be temporarily placed).
        */

        // left border
        push = screenSpacing - item.lastGeometry.left();
        if (push > 0) {
            performPush(i, DirRight, push, 0, (layoutAlignment & Qt::AlignLeft));
        }

        // right border
        push = item.lastGeometry.right()+screenSpacing - workingGeom.width();
        if (push > 0) {
            performPush(i, DirLeft, push, 0, (layoutAlignment & Qt::AlignRight));
        }

        // top border
        push = screenSpacing - item.lastGeometry.top();
        if (push > 0) {
            performPush(i, DirDown, push, 0, (layoutAlignment & Qt::AlignTop));
        }

        // bottom border
        push = item.lastGeometry.bottom()+screenSpacing - workingGeom.height();
        if (push > 0) {
            performPush(i, DirUp, push, 0, (layoutAlignment & Qt::AlignBottom));
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

    /*
      Temporarily place items that could not be pushed inside the working area.
      Put them back if they fit again.
    */
    for (int i=0; i<items.size(); i++) {
        DesktopLayoutItem &item = items[i];
        if (temporaryPlacement && !workingGeom.contains(item.lastGeometry)) {
            performTemporaryPlacement(i);
        } else if (item.temporaryGeometry.isValid()) {
            bool okX = (layoutAlignment==Qt::AlignLeft ?
                        item.lastGeometry.right()+screenSpacing <= workingGeom.width() :
                        item.lastGeometry.left() >= screenSpacing);
            bool okY = (layoutAlignment==Qt::AlignTop ?
                        item.lastGeometry.bottom()+screenSpacing <= workingGeom.height() :
                        item.lastGeometry.top() >= screenSpacing);
            if (okX && okY) {
                revertTemporaryPlacement(i);
            }
        }
    }

    /*
      Reset on-screen positions of items to where they should be.
      Used when the offset of the working area changes, and if
      bottom- or right-aligning, when the size of the working area changes.
    */
    if (reassignPositions) {
        for (int i=0; i<items.size(); i++) {
            DesktopLayoutItem &item = items[i];
            QRectF geom = (item.temporaryGeometry.isValid() ? item.temporaryGeometry : item.lastGeometry);
            item.item->setGeometry(geom.translated(workingGeom.topLeft()));
        }
        reassignPositions = false;
    }
}

// This should be called when the geometry of an item has been changed.
// If the change was made by the user, the new position is used as the preferred position.
void DesktopLayout::itemGeometryChanged (QGraphicsLayoutItem *layoutItem)
{
    if (isActivated()) return;

    for (int i=0; i<items.size(); i++) {
        DesktopLayoutItem &item = items[i];
        if (item.item == layoutItem) {
            QRectF currentRelative = item.item->geometry().translated(-workingGeom.topLeft());
            if (item.lastGeometry != currentRelative) {
                item.lastGeometry = currentRelative;
                item.preferredGeometry = currentRelative;
                kDebug() << "Repositioned" << i << "to" << currentRelative;
                invalidate();
            }
            break;
        }
    }
}

QSizeF DesktopLayout::sizeHint (Qt::SizeHint which, const QSizeF &constraint) const
{
    return QSizeF();
}

QPointF DesktopLayout::positionHorizontally(const QSizeF &itemSize, Qt::Alignment align, qreal spL, qreal spR, qreal spT, qreal spB) const
{
    QPointF final = QPointF(-1,-1);

    /* space needed for the applet - spacing added */
    QSizeF size = QSizeF(itemSize.width()+spL+spR, itemSize.height()+spT+spB);

    /* The algorithm works by placing a rectangle of needed size (above) somewhere
       on the screen and checking for obstacles. x and y hold the top-left
       coordinates of the rectangle being tested.
       Start with the rectangle in the requested corner of the screen. */
    qreal x = 0;
    qreal y = 0;
    if ((align & Qt::AlignRight)) {
        x = workingGeom.width()-size.width();
    }

    if ((align & Qt::AlignBottom)) {
        y = workingGeom.height()-size.height();
    }

    while (1) {
        bool outOfX, outOfY;

        /* Place items horizontally - first try positions at the same height.
           Once there are no more positions at the same height to try, try positions
           on a different height.

           If aligning left, X starts at 0 and increases as we try
           positions more right. We must skip to a new Y once the
           remaining X-space is less than the needed width (X + applet width > screen width).

           If aligining right, X starts with the x coordinate of the top-left point
           of the most-right rectangle and decreases as we try positions more left.
           Skip to a new Y when there is no more space on the left (X<0). */

        if ((align & Qt::AlignLeft)) {
            outOfX = (x + size.width() > workingGeom.width());
        } else {
            outOfX = (x < 0);
        }

        /* Every time we run out of X space, we try positions at a new hight.

           If aligning to top, Y starts at 0 and increases as lower positions are tried.
           If aligning to bottom, Y starts at the y coordinate of a most-bottom rectangle
           and decreases as higher positions are tried.
           If there is no more Y space, all possible positions have been tried,
           and there is no place for the applet.
        */

        if ((align & Qt::AlignBottom)) {
            outOfY = (y < 0);
        } else {
            outOfY = (y + size.height() > workingGeom.height());
        }

        if (outOfY) {
            break;
        }

        if (outOfX) {
            /* Jumping to the next height

               Take a rectangle ("strap") that expands horizontally over the whole screen,
               and vertically starts and ends with the same lines as the previously
               tested rectangles.

               The next height at which the applet could possibly be placed is obtained as follows:
               - if aligning to top, find the obstacle intersecting the strap that vertically
                 ends at the highest position, and take the height where it ends,
               - if aligning to bottom, find the obstacle intersecting the strap that vertically
                 starts at the lowest position, and take the height where is starts.
            */

            QRectF a;
            if ((align & Qt::AlignTop)) {
                a = itemInRegionEndingFirstVert(QRectF(0, y, workingGeom.width(), size.height()));
            } else {
                a = itemInRegionStartingLastVert(QRectF(0, y, workingGeom.width(), size.height()));
            }

            /* If there were no obstacles, the whole screen is too narrow */
            if (!a.isValid()) {
                break;
            }

            /* Jump to the new height */
            if ((align & Qt::AlignTop)) {
                y = a.y() + a.height();
            } else {
                y = a.y() - size.height();
            }

            /* Set the starting X position for the new height */
            if ((align & Qt::AlignLeft)) {
                x = 0;
            } else {
                x = workingGeom.width()-size.width();
            }
        } else {
            /* Check for obstacles in out rectangle and possibly try the next position
               on this height.
               If there are obstacles, the next X position
               where the applet could possibly be placed is
               obtained by:
               - if aligning to left, find the obstacle intersecting with our rectangle that
                 horizontally ends the farthest on the right
               - if aligining to right, find the obstacle intersecting with our rectangle
                 that horizontally starts the farthest on the left */

            QRectF a;
            if ((align & Qt::AlignLeft)) {
                a = itemInRegionEndingLastHoriz(QRectF(x, y, size.width(), size.height()));
            } else {
                a = itemInRegionStartingFirstHoriz(QRectF(x, y, size.width(), size.height()));
            }

            /* no obstacle, place the applet! */
            if (!a.isValid()) {
                final = QPointF(x+spL, y+spT);
                break;
            }

            /* try next position */
            if ((align & Qt::AlignLeft)) {
                x = a.x() + a.width();
            } else {
                x = a.x() - size.width();
            }
        }
    }
    return final;
}

/* Instead of placing horizontally, place vertically.
   So, try different heights and possibly jump to the next X */
QPointF DesktopLayout::positionVertically(const QSizeF &itemSize, Qt::Alignment align, qreal spL, qreal spR, qreal spT, qreal spB) const
{
    /* all the same here */

    QPointF final = QPointF(-1,-1);

    QSizeF size = QSizeF(itemSize.width()+spL+spR, itemSize.height()+spT+spB);

    qreal x = 0;
    qreal y = 0;
    if ((align & Qt::AlignRight))  x = workingGeom.width()-size.width();
    if ((align & Qt::AlignBottom)) y = workingGeom.height()-size.height();

    while (1) {
        bool outOfX, outOfY;

        /* These are unchanged */

        if ((align & Qt::AlignLeft)) {
            outOfX = (x + size.width() > workingGeom.width());
        } else {
            outOfX = (x < 0);
        }

        if ((align & Qt::AlignBottom)) {
            outOfY = (y < 0);
        } else {
            outOfY = (y + size.height() > workingGeom.height());
        }

        /* instead of stopping when all heights have been tried
           stop when all X positions have been tried */
        if (outOfX) {
            break;
        }

        if (outOfY) {
            /* Jumping to the next X position
               Take a vertical strap. */

            QRectF a;
            if ((align & Qt::AlignLeft)) {
                a = itemInRegionEndingFirstHoriz(QRectF(x, 0, size.width(), workingGeom.height()));
            } else {
                a = itemInRegionStartingLastHoriz(QRectF(x, 0, size.width(), workingGeom.height()));
            }

            /* Screen is not high enough */
            if (!a.isValid()) {
                break;
            }

            /* Jump to the new X */
            if ((align & Qt::AlignLeft)) {
                x = a.x() + a.width();
            } else {
                x = a.x() - size.width();
            }

            /* Set the starting height position for the new X */
            if ((align & Qt::AlignTop)) {
                y = 0;
            } else {
                y = workingGeom.height()-size.height();
            }
        } else {
            /* Instead look for obstacles vertically */

            QRectF a;
            if ((align & Qt::AlignTop)) {
                a = itemInRegionEndingLastVert(QRectF(x, y, size.width(), size.height()));
            } else {
                a = itemInRegionStartingFirstVert(QRectF(x, y, size.width(), size.height()));
            }

            /* no obstacle, place the applet! */
            if (!a.isValid()) {
                final = QPointF(x+spL, y+spT);
                break;
            }

            /* try next position */
            if ((align & Qt::AlignTop)) {
                y = a.y() + a.height();
            } else {
                y = a.y() - size.height();
            }
        }
    }
    return final;
}

QRectF DesktopLayout::itemInRegionStartingFirstHoriz(const QRectF &region) const
{
    QRectF ret = QRectF(0,0,-1,-1);
    qreal l = std::numeric_limits<qreal>::max();
    for (int i=0; i<items.count(); i++) {
      DesktopLayoutItem item = items[i];
      if (!item.lastGeometry.isValid()) {
        continue;
      }
      QRectF itemGeom = (item.temporaryGeometry.isValid() ? item.temporaryGeometry : item.lastGeometry);
      qreal cl = itemGeom.x();
      if (itemGeom.intersects(region) && cl < l) {
          ret = itemGeom;
          l = cl;
      }
    }
    return ret;
}

QRectF DesktopLayout::itemInRegionEndingLastHoriz(const QRectF &region) const
{
    QRectF ret = QRectF(0,0,-1,-1);
    qreal l = -1;
    for (int i=0; i<items.count(); i++) {
      DesktopLayoutItem item = items[i];
      if (!item.lastGeometry.isValid()) {
        continue;
      }
      QRectF itemGeom = (item.temporaryGeometry.isValid() ? item.temporaryGeometry : item.lastGeometry);
      qreal cl = itemGeom.x() + itemGeom.width();
      if (itemGeom.intersects(region) && cl > l) {
          ret = itemGeom;
          l = cl;
      }
    }
    return ret;
}

QRectF DesktopLayout::itemInRegionEndingFirstVert(const QRectF &region) const
{
    QRectF ret = QRectF(0,0,-1,-1);
    qreal l = std::numeric_limits<qreal>::max();
    for (int i=0; i<items.count(); i++) {
      DesktopLayoutItem item = items[i];
      if (!item.lastGeometry.isValid()) {
        continue;
      }
      QRectF itemGeom = (item.temporaryGeometry.isValid() ? item.temporaryGeometry : item.lastGeometry);
      qreal cl = itemGeom.y() + itemGeom.height();
      if (itemGeom.intersects(region) && cl < l) {
          ret = itemGeom;
          l = cl;
      }
    }
    return ret;
}

QRectF DesktopLayout::itemInRegionStartingLastVert(const QRectF &region) const
{
    QRectF ret = QRectF(0,0,-1,-1);
    qreal l = -1;
    for (int i=0; i<items.count(); i++) {
      DesktopLayoutItem item = items[i];
      if (!item.lastGeometry.isValid()) {
        continue;
      }
      QRectF itemGeom = (item.temporaryGeometry.isValid() ? item.temporaryGeometry : item.lastGeometry);
      qreal cl = itemGeom.y();
      if (itemGeom.intersects(region) && cl > l) {
          ret = itemGeom;
          l = cl;
      }
    }
    return ret;
}

QRectF DesktopLayout::itemInRegionStartingFirstVert(const QRectF &region) const
{
    QRectF ret = QRectF(0,0,-1,-1);
    qreal l = std::numeric_limits<qreal>::max();
    for (int i=0; i<items.count(); i++) {
      DesktopLayoutItem item = items[i];
      if (!item.lastGeometry.isValid()) {
        continue;
      }
      QRectF itemGeom = (item.temporaryGeometry.isValid() ? item.temporaryGeometry : item.lastGeometry);
      qreal cl = itemGeom.y();
      if (itemGeom.intersects(region) && cl < l) {
          ret = itemGeom;
          l = cl;
      }
    }
    return ret;
}

QRectF DesktopLayout::itemInRegionEndingLastVert(const QRectF &region) const
{
    QRectF ret = QRectF(0,0,-1,-1);
    qreal l = -1;
    for (int i=0; i<items.count(); i++) {
      DesktopLayoutItem item = items[i];
      if (!item.lastGeometry.isValid()) {
        continue;
      }
      QRectF itemGeom = (item.temporaryGeometry.isValid() ? item.temporaryGeometry : item.lastGeometry);
      qreal cl = itemGeom.y() + itemGeom.height();
      if (itemGeom.intersects(region) && cl > l) {
          ret = itemGeom;
          l = cl;
      }
    }
    return ret;
}

QRectF DesktopLayout::itemInRegionEndingFirstHoriz(const QRectF &region) const
{
    QRectF ret = QRectF(0,0,-1,-1);
    qreal l = std::numeric_limits<qreal>::max();
    for (int i=0; i<items.count(); i++) {
      DesktopLayoutItem item = items[i];
      if (!item.lastGeometry.isValid()) {
        continue;
      }
      QRectF itemGeom = (item.temporaryGeometry.isValid() ? item.temporaryGeometry : item.lastGeometry);
      qreal cl = itemGeom.x() + itemGeom.width();
      if (itemGeom.intersects(region) && cl < l) {
          ret = itemGeom;
          l = cl;
      }
    }
    return ret;
}

QRectF DesktopLayout::itemInRegionStartingLastHoriz(const QRectF &region) const
{
    QRectF ret = QRectF(0,0,-1,-1);
    qreal l = -1;
    for (int i=0; i<items.count(); i++) {
      DesktopLayoutItem item = items[i];
      if (!item.lastGeometry.isValid()) {
        continue;
      }
      QRectF itemGeom = (item.temporaryGeometry.isValid() ? item.temporaryGeometry : item.lastGeometry);
      qreal cl = itemGeom.x();
      if (itemGeom.intersects(region) && cl > l) {
          ret = itemGeom;
          l = cl;
      }
    }
    return ret;
}
