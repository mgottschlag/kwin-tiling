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

ItemSpace::ItemSpace()
  : spaceAlignment(Qt::AlignTop|Qt::AlignLeft),
    workingGeom(QSizeF()),
    placementSpacing(0),
    screenSpacing(0),
    shiftingSpacing(0)
{
}

void ItemSpace::setWorkingArea(QSizeF area)
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
    for (int groupId = 0; groupId < m_groups.size(); groupId++) {
        ItemGroup &group = m_groups[groupId];

        for (int itemId = 0; itemId < group.m_groupItems.size(); itemId++) {
            ItemSpaceItem &item = group.m_groupItems[itemId];

            qreal push;
            PushPower power;

            /*
              Push items intersecting working area borders inside.
              For borders adjunct to the corner of alignment, allow pushing
              over the opposite border (and items there may be temporarily placed).
            */

            // left border
            push = screenSpacing - item.lastGeometry.left();
            if (push > 0) {
                item.animateMovement = true;
                power = PushAwayFromPreferred;
                if ((spaceAlignment & Qt::AlignLeft)) {
                    power |= PushOverBorder;
                }
                performPush(groupId, DirRight, push, power);
            }

            // right border
            push = item.lastGeometry.right()+screenSpacing - workingGeom.width();
            if (push > 0) {
                item.animateMovement = true;
                power = PushAwayFromPreferred;
                if ((spaceAlignment & Qt::AlignRight)) {
                    power |= PushOverBorder;
                }
                performPush(groupId, DirLeft, push, power);
            }

            // top border
            push = screenSpacing - item.lastGeometry.top();
            if (push > 0) {
                item.animateMovement = true;
                power = PushAwayFromPreferred;
                if ((spaceAlignment & Qt::AlignTop)) {
                    power |= PushOverBorder;
                }
                performPush(groupId, DirDown, push, power);
            }

            // bottom border
            push = item.lastGeometry.bottom()+screenSpacing - workingGeom.height();
            if (push > 0) {
                item.animateMovement = true;
                power = PushAwayFromPreferred;
                if ((spaceAlignment & Qt::AlignBottom)) {
                    power |= PushOverBorder;
                }
                performPush(groupId, DirUp, push, power);
            }

            /*
              Push items back towards their perferred positions.
              Cannot push items out of the working area,
              cannot push items away from their preferred positions.
            */
            if (item.pushBack) {
                // left/right
                push = item.preferredGeometry.left() - item.lastGeometry.left();
                if (push > 0) {
                    performPush(groupId, DirRight, push, NoPower);
                } else if (push < 0) {
                    performPush(groupId, DirLeft, -push, NoPower);
                }
                // up/down
                push = item.preferredGeometry.top() - item.lastGeometry.top();
                if (push > 0) {
                    performPush(groupId, DirDown, push, NoPower);
                } else if (push < 0) {
                    performPush(groupId, DirUp, -push, NoPower);
                }
            }
        }
    }
}

qreal ItemSpace::positionVisibility (QRectF geom)
{
    QRectF visibleArea = QRectF(QPointF(), workingGeom);
    QRectF visibleItemPart = visibleArea.intersected(geom);
    qreal itemSurface = geom.width() * geom.height();
    qreal itemVisibleSurface = visibleItemPart.width() * visibleItemPart.height();
    return (itemVisibleSurface / itemSurface);
}

void ItemSpace::offsetPositions(const QPointF &offset)
{
    for (int groupId = 0; groupId < m_groups.size(); groupId++) {
        ItemGroup &group = m_groups[groupId];

        for (int itemId = 0; itemId < group.m_groupItems.size(); itemId++) {
            ItemSpaceItem &item = group.m_groupItems[itemId];

            item.preferredGeometry.adjust(offset.x(), offset.y(), offset.x(), offset.y());
            item.lastGeometry.adjust(offset.x(), offset.y(), offset.x(), offset.y());
        }
    }
}

qreal ItemSpace::performPush(int groupId, Direction direction, qreal amount, PushPower power)
{
    ItemGroup &group = m_groups[groupId];

    preparePush(direction, power);
    group.addRequest(this, ItemGroup::Request(-1, 0, amount));
    group.applyResults(this, -1);
    return group.m_pushAvailable;
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

QRectF ItemSpace::itemInRegionStartingFirstVert(const QRectF &region) const
{
    QRectF ret = QRectF(0,0,-1,-1);
    qreal l = std::numeric_limits<qreal>::max();

    for (int groupId = 0; groupId < m_groups.size(); groupId++) {
        const ItemGroup &group = m_groups[groupId];

        for (int itemId = 0; itemId < group.m_groupItems.size(); itemId++) {
            const ItemSpaceItem &item = group.m_groupItems[itemId];

            if (!item.lastGeometry.isValid()) {
              continue;
            }
            qreal cl = item.lastGeometry.y();
            if (item.lastGeometry.intersects(region) && cl < l) {
                ret = item.lastGeometry;
                l = cl;
            }
        }
    }
    return ret;
}

QRectF ItemSpace::itemInRegionEndingLastVert(const QRectF &region) const
{
    QRectF ret = QRectF(0,0,-1,-1);
    qreal l = -1;

    for (int groupId = 0; groupId < m_groups.size(); groupId++) {
        const ItemGroup &group = m_groups[groupId];

        for (int itemId = 0; itemId < group.m_groupItems.size(); itemId++) {
            const ItemSpaceItem &item = group.m_groupItems[itemId];

            if (!item.lastGeometry.isValid()) {
              continue;
            }
            qreal cl = item.lastGeometry.y() + item.lastGeometry.height();
            if (item.lastGeometry.intersects(region) && cl > l) {
                ret = item.lastGeometry;
                l = cl;
            }
        }
    }
    return ret;
}

QRectF ItemSpace::itemInRegionEndingFirstHoriz(const QRectF &region) const
{
    QRectF ret = QRectF(0,0,-1,-1);
    qreal l = std::numeric_limits<qreal>::max();

    for (int groupId = 0; groupId < m_groups.size(); groupId++) {
        const ItemGroup &group = m_groups[groupId];

        for (int itemId = 0; itemId < group.m_groupItems.size(); itemId++) {
            const ItemSpaceItem &item = group.m_groupItems[itemId];

            if (!item.lastGeometry.isValid()) {
              continue;
            }
            qreal cl = item.lastGeometry.x() + item.lastGeometry.width();
            if (item.lastGeometry.intersects(region) && cl < l) {
                ret = item.lastGeometry;
                l = cl;
            }
        }
    }
    return ret;
}

QRectF ItemSpace::itemInRegionStartingLastHoriz(const QRectF &region) const
{
    QRectF ret = QRectF(0,0,-1,-1);
    qreal l = -1;

    for (int groupId = 0; groupId < m_groups.size(); groupId++) {
        const ItemGroup &group = m_groups[groupId];

        for (int itemId = 0; itemId < group.m_groupItems.size(); itemId++) {
            const ItemSpaceItem &item = group.m_groupItems[itemId];

            if (!item.lastGeometry.isValid()) {
              continue;
            }
            qreal cl = item.lastGeometry.x();
            if (item.lastGeometry.intersects(region) && cl > l) {
                ret = item.lastGeometry;
                l = cl;
            }
        }
    }
    return ret;
}

ItemSpace::ItemGroup::Request::Request(
    int sourceGroup,
    qreal sourceGroupPushRequested,
    qreal pushRequested
)
  : m_sourceGroup(sourceGroup),
    m_sourceGroupPushRequested(sourceGroupPushRequested),
    m_pushRequested(pushRequested),
    m_compensated(false)
{
}

void ItemSpace::ItemGroup::Request::activate (ItemSpace *itemSpace, ItemGroup *group)
{
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
    for (int itemId = 0; itemId < group->m_groupItems.size(); itemId++) {
        ItemSpaceItem &item = group->m_groupItems[itemId];
        QRectF origGeom = item.lastGeometry;
        QRectF fullGeom = origGeom.adjusted(-itemSpace->shiftingSpacing, -itemSpace->shiftingSpacing,
                                            itemSpace->shiftingSpacing, itemSpace->shiftingSpacing);

        // limit push by screen boundaries
        if (!(itemSpace->m_power & PushOverBorder)) {
            qreal limit;
            switch (itemSpace->m_direction) {
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
            group->m_pushAvailable = qMax(qreal(0.0), qMin(group->m_pushAvailable, limit));
            if (group->m_pushAvailable == 0) {
                break;
            }
        }

        // limit push to not push the item away from its preferred position
        if (!(itemSpace->m_power & PushAwayFromPreferred) && item.pushBack) {
            qreal limit;
            switch (itemSpace->m_direction) {
                case DirLeft:
                    limit = origGeom.left() - item.preferredGeometry.left();
                    break;
                case DirRight:
                    limit = -(origGeom.left() - item.preferredGeometry.left());
                    break;
                case DirUp:
                    limit = origGeom.top() - item.preferredGeometry.top();
                    break;
                case DirDown:
                    limit = -(origGeom.top() - item.preferredGeometry.top());
                    break;
            }
            limit = qMax(qreal(0.0), limit);
            group->m_pushAvailable = qMin(group->m_pushAvailable, limit);
            if (group->m_pushAvailable == 0) {
                break;
            }
        }

        // look for items in the way
        for (int testGroupId = 0; testGroupId < itemSpace->m_groups.size(); testGroupId++) {
            QList<int> asa;
            if (testGroupId == group->m_id || group->groupIsAbove(itemSpace, asa, testGroupId)) {
                continue;
            }
            ItemGroup &testGroup = itemSpace->m_groups[testGroupId];

            // calculate how much the offending group needs to be pushed
            qreal groupPush = 0;
            for (int testItemId = 0; testItemId < testGroup.m_groupItems.size(); testItemId++) {
                ItemSpaceItem &testItem = testGroup.m_groupItems[testItemId];

                QRectF newlyTakenSpace;
                qreal push;
                switch (itemSpace->m_direction) {
                case DirLeft:
                    newlyTakenSpace = QRectF(fullGeom.left() - group->m_pushAvailable, fullGeom.top(), group->m_pushAvailable, fullGeom.height());
                    push = testItem.lastGeometry.right() - newlyTakenSpace.left();
                    break;
                case DirRight:
                    newlyTakenSpace = QRectF(fullGeom.right(), fullGeom.top(), group->m_pushAvailable, fullGeom.height());
                    push = newlyTakenSpace.right() - testItem.lastGeometry.left();
                    break;
                case DirUp:
                    newlyTakenSpace = QRectF(fullGeom.left(), fullGeom.top() - group->m_pushAvailable, fullGeom.width(), group->m_pushAvailable);
                    push = testItem.lastGeometry.bottom() - newlyTakenSpace.top();
                    break;
                case DirDown:
                    newlyTakenSpace = QRectF(fullGeom.left(), fullGeom.bottom(), fullGeom.width(), group->m_pushAvailable);
                    push = newlyTakenSpace.bottom() - testItem.lastGeometry.top();
                    break;
                }

                // check if it is an obstacle
                if (testItem.lastGeometry.intersects(newlyTakenSpace)) {
                    groupPush = qMax(groupPush, push);
                }
            }

            if (groupPush == 0) {
                continue;
            }

            // post a move request to the obstacle
            if (!group->m_obstacles.contains(testGroupId)) {
                group->m_obstacles.append(testGroupId);
            }
            testGroup.addRequest(itemSpace, Request(group->m_id, group->m_pushAvailable, groupPush));

            // limit our push by how much the obstacle can actually move
            if (testGroup.m_pushAvailable < groupPush) {
                group->m_pushAvailable = qMax(qreal(0.0), group->m_pushAvailable - (groupPush - testGroup.m_pushAvailable));
                if (group->m_pushAvailable == 0) {
                    break;
                }
            }
        }
    }
}

void ItemSpace::ItemGroup::resetPush(int id)
{
    m_id = id;
    m_largestPushRequested = 0,
    m_pushAvailable = std::numeric_limits<qreal>::max();
    m_requests = QList<Request>();
    m_obstacles = QList<int>();
}

void ItemSpace::ItemGroup::addRequest (ItemSpace *itemSpace, const class Request &request)
{
    m_requests.append(request);
    m_requests.last().activate(itemSpace, this);
}

void ItemSpace::ItemGroup::applyResults(ItemSpace *itemSpace, int cameFrom)
{
    bool notComplete = false;
    for (int i = 0; i < m_requests.size(); i++) {
        Request &request = m_requests[i];
        if (request.m_sourceGroup == -1) {
            continue;
        }

        if (request.m_sourceGroup == cameFrom) {
            qreal pushLost = request.m_sourceGroupPushRequested - itemSpace->m_groups[cameFrom].m_pushAvailable;
            request.m_pushRequested -= pushLost;
            request.m_compensated = true;
        } else if (!request.m_compensated) {
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

    for (int groupId = 0; groupId < m_groupItems.size(); groupId++) {
        ItemSpaceItem &groupItem = m_groupItems[groupId];

        switch (itemSpace->m_direction) {
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

    foreach (int obstacleId, m_obstacles) {
        itemSpace->m_groups[obstacleId].applyResults(itemSpace, m_id);
    }
}

bool ItemSpace::ItemGroup::groupIsAbove(ItemSpace *itemSpace, QList<int> &visited, int groupId)
{
    foreach (const Request &request, m_requests) {
        if (request.m_sourceGroup == -1 || visited.contains(request.m_sourceGroup)) {
            continue;
        }
        if (request.m_sourceGroup == groupId) {
            return true;
        }
        visited.append(request.m_sourceGroup);
        if (itemSpace->m_groups[request.m_sourceGroup].groupIsAbove(itemSpace, visited, groupId)) {
            return true;
        }
    }
    return false;
}

// TODO: optimize
void ItemSpace::addItem(ItemSpaceItem newItem)
{
    QList<ItemSpaceItem> newGroupItems;
    QRectF newItemGeom = newItem.lastGeometry.adjusted(-shiftingSpacing, -shiftingSpacing,
                                                      shiftingSpacing, shiftingSpacing);

    // look for items overlapping with the new item
    for (int groupId = 0; groupId < m_groups.size();) {
        ItemGroup &group = m_groups[groupId];

        // if any item in the group overlaps it, save its items and remove the group
        bool removeGroup = false;
        for (int itemId = 0; itemId < group.m_groupItems.size(); itemId++) {
            ItemSpaceItem &item = group.m_groupItems[itemId];
            if (newItemGeom.intersects(item.lastGeometry)) {
                removeGroup = true;
                break;
            }
        }

        if (removeGroup) {
            newGroupItems << group.m_groupItems;
            m_groups.removeAt(groupId);
        } else {
            groupId++;
        }
    }

    // create the new group
    m_groups.append(ItemGroup());
    ItemGroup &newGroup = m_groups.last();
    newGroup.m_groupItems.append(newItem);
    newGroup.m_groupItems << newGroupItems;
}

// TODO: optimize
void ItemSpace::removeItem(int removeGroup, int removeItemInGroup)
{
    // remove items from group
    m_groups[removeGroup].m_groupItems.removeAt(removeItemInGroup);
    // save other group items
    QList<ItemSpaceItem> otherGroupItems = m_groups[removeGroup].m_groupItems;
    // remove group
    m_groups.removeAt(removeGroup);
    // re-add other group items
    foreach (const ItemSpaceItem &item, otherGroupItems) {
        addItem(item);
    }
}

// TODO: optimize
void ItemSpace::updateItem(int group, int itemInGroup)
{
    ItemSpaceItem copy = m_groups[group].m_groupItems[itemInGroup];
    removeItem(group, itemInGroup);
    addItem(copy);
}

bool ItemSpace::locateItemByPosition(int pos, int *groupIndex, int *itemInGroup) const
{
    int current = 0;
    for (int groupId = 0; groupId < m_groups.size(); groupId++) {
        ItemGroup group = m_groups[groupId];
        if (current + group.m_groupItems.size() > pos) {
            *groupIndex = groupId;
            *itemInGroup = pos - current;
            return true;
        }
        current += group.m_groupItems.size();
    }
    return false;
}

bool ItemSpace::locateItemByUser(QVariant user, int *groupIndex, int *itemInGroup) const
{
    for (int groupId = 0; groupId < m_groups.size(); groupId++) {
        ItemGroup group = m_groups[groupId];
        for (int itemId = 0; itemId < group.m_groupItems.size(); itemId++) {
            ItemSpaceItem &item = group.m_groupItems[itemId];
            if (item.user == user) {
                *groupIndex = groupId;
                *itemInGroup = itemId;
                return true;
            }
        }
    }
    return false;
}

void ItemSpace::preparePush(Direction direction, PushPower power)
{
    m_direction = direction;
    m_power = power;

    for (int i=0; i<m_groups.size(); i++) {
        m_groups[i].resetPush(i);
    }
}
