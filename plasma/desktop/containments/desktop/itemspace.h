/*
  Copyright (c) 2008 Ambroz Bizjak <ambro@b4ever.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

#ifndef _ITEMSPACE_H
#define _ITEMSPACE_H

#include <QRectF>
#include <QList>
#include <QVariant>

/**
 * ItemSpace class
 * Implements "push and pull" dynamics of rectangular items in 2D space.
 *
 * All isolated motion folows these rules:
 *  - overlapping items stay at the same positions relative to each other
 *  - non-overlapping items stay such and do not jump one over another
 *
 * There are two types of motion:
 *  - forced motion: an item moves all items on its way, even if any of them
 *    would intersect the border of the working area
 *  - non-forced motion: an item moves items on its way only as much as the border
 *    of the working area
 *
 * Items are pushed to fit inside the working area:
 * if an item is in the way of one of the borders of alignment, the move is forced;
 * if an item is in the way of one of the opposite borders, the move is non-forced.
 *
 * An item can have a "preferred position". Such item is moved non-forced in the
 * direction towards the preferred position.
 **/
class ItemSpace
{
  public:
    ItemSpace();

    void setWorkingArea(const QSizeF& area);

    /**
     * Returns the visibility of an item at a given position.
     * This is the part of the item inside the working area.
     **/
    qreal positionVisibility(const QRectF& geom);

    class ItemSpaceItem
    {
      public:
        QPointF preferredPosition;
        QRectF lastGeometry;
        bool pushBack : 1;
        bool animateMovement : 1;
        QVariant user;
    };

    enum DirectionFlag {
        DirLeft = 1,
        DirRight = 2,
        DirUp = 4,
        DirDown = 8
    };
    Q_DECLARE_FLAGS(Direction, DirectionFlag)

    enum PushPowerFlag {
        NoPower = 0,
        PushAwayFromPreferred = 1,
        PushOverBorder = 2
    };
    Q_DECLARE_FLAGS(PushPower, PushPowerFlag)

    /**
     * Push an item group. Requires no initialization.
     *
     * @param groupId the index of the group
     * @param direction in which direction pushing will be done
     * @param amount how much to push
     * @param power how 'powerful' the push is; what types of obstacles
     *              can be pushed or ignored
     *
     * @return how much the item group was really pushed
     **/
    qreal performPush(int groupId, Direction direction, qreal amount, PushPower power);

    /**
     * Add a new item.
     * Groups will be updated to reflect the change.
     *
     * @param newItem the item to add; must be initialized
     **/
    void addItem(ItemSpaceItem newItem);

    /**
     * Removes an item by its location.
     *
     * @param groupIndex the index of the item's group
     * @param itemInGroup the index of the item in its group
     **/
    void removeItem(int groupIndex, int itemInGroup);

    /**
     * Move the item to a new position.
     *
     * @param groupIndex the index of the item's group
     * @param itemInGroup the index of the item in its group
     * @param newGeom the new geometry of the item
     **/
    void moveItem(int groupIndex, int itemInGroup, const QRectF& newGeom);

    /**
     * Resize an item. The item's alignment corner will be the center of resizing.
     *
     * @param groupId the index of the group
     * @param direction in which direction pushing will be done
     * @param newSize the item's new size
     **/
    void resizeItem(int groupId, int itemInGroup, const QSizeF& newSize);

    /**
     * Offset the positions of all items.
     **/
    void offsetPositions(const QPointF &offset);

    /**
     * Find an item by its number as if we iterated over
     * all groups and over all items in each group.
     **/
    bool locateItemByPosition(int pos, int *groupIndex, int *itemInGroup) const;

    /**
     * Find an item by its 'user' parameter.
     **/
    bool locateItemByUser(QVariant user, int *groupIndex, int *itemInGroup) const;

    /**
     * Finds an empty place for an item.
     * Tries to stack the item vertically, starting in the corner
     * of alignment, and advances horizontally once no more positions
     * are valid.
     *
     * @param itemSize the size of the item; placementSpacing is already
     *                 considered
     * @param align the corner of the screen where position testing will
     *              begin (and in what directions it will advance)
     *              must be an OR of Qt::AlignLeft or Qt::AlignRight
     *              and Qt::AlignTop or Qt::AlignBottom
     * @param limitedSpace if true, positions outside the working area
     *                     will not be considered; otherwise, positions
     *                     will only be limited by the borders at the
     *                     alignment corner.
     * @param findAll if false, searching will stop after the first valid
     *                position
     *
     * @return all positions found
     **/
    QList<QPointF> positionVertically(
        const QSizeF &itemSize,
        Qt::Alignment align,
        bool limitedSpace,
        bool findAll
    ) const;

    bool positionedProperly(const QRectF& itemGeom);

    /**
     * Represents a group of overlapping items.
     **/
    class ItemGroup
    {
      public:

        class Request
        {
          public:
            /**
             * Create a push request. No calculations will be performed.
             *
             * @param sourceGroup the group that posted the request, or
             *                    -1 if it was posted manually.
             * @param sourceGroupPushRequested how much the posting group wanted
             *                                 to move itself when the request was
             *                                 posted (if sourceGroup is -1)
             * @param pushRequested how much the group concerned is asked to move
             **/
            Request(
                int sourceGroup,
                qreal sourceGroupPushRequested,
                qreal pushRequested
            );

            /**
             * Perform obstacle searching and post push request to obstacle groups.
             * This is the main method involved in recursive push calculation.
             *
             * If an item is found to be in the way of any of the group's items,
             * its ItemGroup will be created if it doesn't have one already,
             * and a new push request will bo posted to it.
             *
             * If the offending group can not move as much as we need it to,
             * we limit the amount our group wants to move, and future obstacles
             * will be asked to move less than they would have been had there
             * been no obstacle.
             *
             * @param group the ItemGroup this push request belongs to
             **/
            void activate(ItemSpace *itemSpace, ItemGroup *group);

            // saved from constructor
            int m_sourceGroup;
            qreal m_sourceGroupPushRequested;
            qreal m_pushRequested;

            // true if the request has already been reached by applyResults
            // and compensated for the reduction of the requester's move
            bool m_compensated;
        };

        void resetPush(int id);

        /**
         * Post a move request.
         * This adds the request to the group and calls activate on it.
         **/
        void addRequest(ItemSpace *itemSpace, const class Request &request);

        /**
         * Apply the results of initial push calculation, moving the items.
         *
         * For each push request belonging to the calling/requesting group,
         * the requesting group is checked for how much it still wants to
         * move itself, and the value is compared to how much it wanted to
         * when the request was posted. The amount of the request is reduced
         * by the difference.
         *
         * If all requests have been compensated, it updates the amount it
         * would like to move (the maximum of all move requests) and
         * physically moves its items. In that case it also calls applyResults
         * on the item groups it has requested to move, which will see the new
         * push amount.
         * (Otherwise, another requesting group will reach it later on.)
         **/
        void applyResults(ItemSpace *itemSpace, int cameFrom);

        // items belonging to this group
        QList<ItemSpaceItem> m_groupItems;

        // the list index of this group in the calculation process
        int m_id;
        // the maximum of all push requests
        qreal m_largestPushRequested;
        // the available space calculated so-far
        qreal m_pushAvailable;

      private:
        // return true if the group is above this one (its requests lead here)
        bool groupIsAbove(ItemSpace *itemSpace, QList<int> &visited, int groupId);

        // move requests posted to this group
        QList<Request> m_requests;
        // groups we asked to move
        QList<int> m_obstacles;
    };

    /**
     * All item groups.
     **/
    QList<ItemGroup> m_groups;

    Qt::Alignment spaceAlignment;
    QSizeF workingGeom;

    qreal placementSpacing;
    qreal screenSpacing;
    qreal shiftingSpacing;

  private:

    void linkItem(ItemSpaceItem newItem);
    void unlinkItem(int removeGroup, int removeItemInGroup);

    /**
     * Prepare for pushing.
     * After that, move requests can be posted to item groups
     * with ItemGroup::addRequest and the move can be performed
     * with ItemGroup::applyResults.
     *
     * @param direction in which direction pushing will be done
     * @param power how 'powerful' the push is; what types of obstacles
     *              can be pushed or ignored
     **/
    void preparePush(Direction direction, PushPower power);

    /**
     * Look for items overlapping with working area borders and move them inside as much as possible.
     **/
    void checkBorders();

    /**
     * Look for items not in their preferred positions and move them back as much as possible.
     **/
    void checkPreferredPositions();

    QRectF itemInRegionStartingFirstVert(const QRectF &region) const;
    QRectF itemInRegionEndingLastVert(const QRectF &region) const;
    QRectF itemInRegionEndingFirstHoriz(const QRectF &region) const;
    QRectF itemInRegionStartingLastHoriz(const QRectF &region) const;

    Direction m_direction;
    PushPower m_power;
};

#endif
