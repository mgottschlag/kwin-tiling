/*
  Copyright (c) 2008 Ambroz Bizjak <ambro@b4ever.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

#include <limits>

#include <KDebug>

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
    // create root item group
    RootItemGroup rootGroup(this, ignoreBorder, direction);
    rootGroup.populateGroup(itemIndex);
    // post a move request
    rootGroup.addRequest(RootItemGroup::Request(NULL, 0, amount));
    // perform the move
    rootGroup.applyResults(NULL);
    return rootGroup.m_pushAvailable;
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

ItemSpace::RootItemGroup::RootItemGroup(
    ItemSpace *itemSpace,
    bool ignoreBorder,
    Direction direction
)
  : ItemGroup(this),
    m_itemSpace(itemSpace),
    m_ignoreBorder(ignoreBorder),
    m_direction(direction)
{
}

ItemSpace::ItemGroup::ItemGroup(RootItemGroup *rootGroup)
  : m_largestPushRequested(0),
    m_pushAvailable(std::numeric_limits<qreal>::max()),
    m_root(rootGroup)
{
}

ItemSpace::ItemGroup::~ItemGroup()
{
    // delete obstacles recursively
    foreach (ItemGroup *child, m_obstacles) {
        // remove our requests
        for (int i = 0; i < child->m_requests.count();) {
            if (child->m_requests[i].m_sourceGroup == this) {
                child->m_requests.removeAt(i);
            } else {
                i++;
            }
        }
        // delete the group if it has no requests left
        if (child->m_requests.count() == 0) {
            delete child;
        }
    }
}

void ItemSpace::ItemGroup::addRequest (const class Request &request)
{
    m_requests.append(request);
    m_requests[m_requests.count()-1].activate(this);
}

ItemSpace::ItemGroup::Request::Request(
    ItemGroup *sourceGroup,
    qreal sourceGroupPushRequested,
    qreal pushRequested
)
  : m_sourceGroup(sourceGroup),
    m_sourceGroupPushRequested(sourceGroupPushRequested),
    m_pushRequested(pushRequested),
    m_applied(false)
{
}

void ItemSpace::ItemGroup::Request::activate (ItemGroup *group)
{
    //kDebug() << "group" << group->m_groupItems << "request" << m_pushRequested;

    // don't do anything if the group was already asked to move at
    // least as much as we ask
    if (group->m_largestPushRequested >= m_pushRequested) {
        return;
    }

    qreal largest = group->m_largestPushRequested;
    // record our request as the largest
    group->m_largestPushRequested = m_pushRequested;
    // don't do anything if the group already hit an unmovable obstacle
    if (group->m_pushAvailable < largest) {
        return;
    }

    // set the available push to our requested value
    // and limit it as obstacles are found
    group->m_pushAvailable = m_pushRequested;

    // look for obstacles for every item in the group
    foreach (const int i, group->m_groupItems) {
        ItemSpaceItem &groupItem = group->m_root->m_itemSpace->items[i];
        QRectF origGeom = groupItem.lastGeometry;
        QRectF fullGeom = origGeom.adjusted(-group->m_root->m_itemSpace->shiftingSpacing, -group->m_root->m_itemSpace->shiftingSpacing,
                                            group->m_root->m_itemSpace->shiftingSpacing, group->m_root->m_itemSpace->shiftingSpacing);

        // limit push by screen boundaries
        if (!group->m_root->m_ignoreBorder) {
            qreal limit;
            switch (group->m_root->m_direction) {
                case DirLeft:
                    limit = origGeom.left() - group->m_root->m_itemSpace->screenSpacing;
                    break;
                case DirRight:
                    limit = group->m_root->m_itemSpace->workingGeom.width() - group->m_root->m_itemSpace->screenSpacing - origGeom.right();
                    break;
                case DirUp:
                    limit = origGeom.top() - group->m_root->m_itemSpace->screenSpacing;
                    break;
                case DirDown:
                    limit = group->m_root->m_itemSpace->workingGeom.height() - group->m_root->m_itemSpace->screenSpacing - origGeom.bottom();
                    break;
            }
            group->m_pushAvailable = qMax(0.0, qMin(group->m_pushAvailable, limit));
            if (group->m_pushAvailable == 0) {
                break;
            }
        }

        // look for items in the way
        for (int j = 0; j < group->m_root->m_itemSpace->items.size(); ++j) {
            if (group->itemIsCurrentOrAbove(j)) {
                continue;
            }
            ItemSpaceItem &item = group->m_root->m_itemSpace->items[j];

            QRectF newlyTakenSpace;
            qreal push;
            switch (group->m_root->m_direction) {
                case DirLeft:
                    newlyTakenSpace = QRectF(fullGeom.left()-group->m_pushAvailable, fullGeom.top(), group->m_pushAvailable, fullGeom.height());
                    push = item.lastGeometry.right()-newlyTakenSpace.left();
                    break;
                case DirRight:
                    newlyTakenSpace = QRectF(fullGeom.right(), fullGeom.top(), group->m_pushAvailable, fullGeom.height());
                    push = newlyTakenSpace.right()-item.lastGeometry.left();
                    break;
                case DirUp:
                    newlyTakenSpace = QRectF(fullGeom.left(), fullGeom.top()-group->m_pushAvailable, fullGeom.width(), group->m_pushAvailable);
                    push = item.lastGeometry.bottom()-newlyTakenSpace.top();
                    break;
                case DirDown:
                    newlyTakenSpace = QRectF(fullGeom.left(), fullGeom.bottom(), fullGeom.width(), group->m_pushAvailable);
                    push = newlyTakenSpace.bottom()-item.lastGeometry.top();
                    break;
            }

            // check if the item is in the way
            if (item.lastGeometry.intersects(newlyTakenSpace)) {
                // create the item's group if it doesn't exist
                ItemGroup *childGroup = group->findGroup(j);
                if (!childGroup) {
                    childGroup = new ItemGroup(group->m_root);
                    childGroup->populateGroup(j);
                }

                //kDebug() << "group" << group->m_groupItems << "requesting" << childGroup->m_groupItems << "push" << push;

                // post a move request to the obstacle
                if (!group->m_obstacles.contains(childGroup)) {
                    group->m_obstacles.append(childGroup);
                }
                childGroup->addRequest(Request(group, group->m_pushAvailable, push));

                // limit our push by how much the obstacle can actually move
                if (childGroup->m_pushAvailable < push) {
                    group->m_pushAvailable = qMax(0.0, group->m_pushAvailable - (push - childGroup->m_pushAvailable));
                    if (group->m_pushAvailable == 0) {
                        break;
                    }
                }
            }
        }
    }
    //kDebug() << "group" << group->m_groupItems << "available" << group->m_pushAvailable;
}

void ItemSpace::ItemGroup::applyResults(ItemGroup *cameFrom)
{
    bool notComplete = false;
    for (int i = 0; i < m_requests.size(); i++) {
        Request &request = m_requests[i];
        if (!request.m_sourceGroup) {
            continue;
        }

        if (request.m_sourceGroup == cameFrom) {
            qreal pushLost = request.m_sourceGroupPushRequested - cameFrom->m_pushAvailable;
            request.m_pushRequested -= pushLost;
            request.m_applied = true;
        } else if (!request.m_applied) {
            notComplete = true;
        }
    }

    if (notComplete) {
        return;
    }

    qreal totalPushRequired = 0;
    for (int i = 0; i < m_requests.size(); i++) {
        Request &request = m_requests[i];
        totalPushRequired = qMax(totalPushRequired, request.m_pushRequested);
    }
    m_pushAvailable = qMin(m_pushAvailable, totalPushRequired);

    foreach (const int i, m_groupItems) {
        ItemSpaceItem &groupItem = m_root->m_itemSpace->items[i];
        switch (m_root->m_direction) {
            case DirLeft:
                groupItem.lastGeometry = groupItem.lastGeometry.adjusted(-m_pushAvailable, 0, -m_pushAvailable, 0);
                break;
            case DirRight:
                groupItem.lastGeometry = groupItem.lastGeometry.adjusted(m_pushAvailable, 0, m_pushAvailable, 0);
                break;
            case DirUp:
                groupItem.lastGeometry = groupItem.lastGeometry.adjusted(0, -m_pushAvailable, 0, -m_pushAvailable);
                break;
            case DirDown:
                groupItem.lastGeometry = groupItem.lastGeometry.adjusted(0, m_pushAvailable, 0, m_pushAvailable);
                break;
        }
    }

    foreach (ItemGroup *child, m_obstacles) {
        child->applyResults(this);
    }
}

bool ItemSpace::ItemGroup::itemIsCurrentOrAbove(int item)
{
    QList<ItemGroup *> visited;
    return itemIsCurrentOrAboveRecurser(&visited, this, item);
}

bool ItemSpace::ItemGroup::itemIsCurrentOrAboveRecurser(QList<ItemGroup *> *visited, ItemGroup *current, int item)
{
    if (visited->contains(current)) {
        return FALSE;
    }
    if (current->m_groupItems.contains(item)) {
        return TRUE;
    }
    visited->append(current);

    foreach (Request request, current->m_requests) {
        if (request.m_sourceGroup && itemIsCurrentOrAboveRecurser(visited, request.m_sourceGroup, item)) {
            return TRUE;
        }
    }

    return FALSE;
}

void ItemSpace::ItemGroup::populateGroup(int thisItem)
{
    m_groupItems.append(thisItem);

    QRectF origGeom = m_root->m_itemSpace->items[thisItem].lastGeometry;
    QRectF fullGeom = origGeom.adjusted(-m_root->m_itemSpace->shiftingSpacing, -m_root->m_itemSpace->shiftingSpacing,
                                        m_root->m_itemSpace->shiftingSpacing, m_root->m_itemSpace->shiftingSpacing);

    for (int i = 0; i < m_root->m_itemSpace->items.size(); i++) {
        if (m_groupItems.contains(i)) {
            continue;
        }
        ItemSpaceItem &item = m_root->m_itemSpace->items[i];
        if (item.lastGeometry.intersects(fullGeom)) {
            populateGroup(i);
        }
    }
}

ItemSpace::ItemGroup * ItemSpace::ItemGroup::findGroup(int item)
{
    QList<ItemGroup *> visited;
    return findGroupRecurser(&visited, m_root, item);
}

ItemSpace::ItemGroup * ItemSpace::ItemGroup::findGroupRecurser(QList<ItemGroup *> *visited, ItemGroup *current, int item)
{
    if (visited->contains(current)) {
        return NULL;
    }
    if (current->m_groupItems.contains(item)) {
        return current;
    }
    visited->append(current);

    foreach (ItemGroup *child, current->m_obstacles) {
        ItemGroup *result = findGroupRecurser(visited, child, item);
        if (result) {
            return result;
        }
    }

    return NULL;
}
