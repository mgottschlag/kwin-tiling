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
    ItemSpace ();

    void addItem(bool pushBack, const QRectF &preferredGeom, const QRectF &lastGeom = QRectF());
    void removeAt(int itemIndex);
    int count();

    void setWorkingArea(QSizeF area);
    void activate();

    qreal positionVisibility(int itemIndex);

    enum DirectionFlag {
        DirLeft = 1,
        DirRight = 2,
        DirUp = 4,
        DirDown = 8
    };
    Q_DECLARE_FLAGS(Direction, DirectionFlag);

    enum PushPowerFlag {
        NoPower = 0,
        PushAwayFromPreferred = 1,
        PushOverBorder = 2
    };
    Q_DECLARE_FLAGS(PushPower, PushPowerFlag);

    void offsetPositions(const QPointF &offset);
    qreal performPush(int itemIndex, Direction direction, qreal amount, PushPower power);

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
     **/
    QList<QPointF> positionVertically(
        const QSizeF &itemSize,
        Qt::Alignment align,
        bool limitedSpace,
        bool findAll
    ) const;

    bool positionedProperly(QRectF itemGeom);

    QRectF itemInRegionStartingFirstHoriz(const QRectF &region) const;
    QRectF itemInRegionEndingLastHoriz(const QRectF &region) const;
    QRectF itemInRegionEndingFirstVert(const QRectF &region) const;
    QRectF itemInRegionStartingLastVert(const QRectF &region) const;
    QRectF itemInRegionStartingFirstVert(const QRectF &region) const;
    QRectF itemInRegionEndingLastVert(const QRectF &region) const;
    QRectF itemInRegionEndingFirstHoriz(const QRectF &region) const;
    QRectF itemInRegionStartingLastHoriz(const QRectF &region) const;

    class ItemSpaceItem
    {
      public:
        QRectF preferredGeometry;
        QRectF lastGeometry;
        bool pushBack : 1;
        bool animateMovement : 1;
    };

    QList<ItemSpaceItem> items;

    Qt::Alignment spaceAlignment;
    QSizeF workingGeom;

    qreal placementSpacing;
    qreal screenSpacing;
    qreal shiftingSpacing;

    class RootItemGroup;

    /**
     * Represents a group of overlapping items in the process of
     * push calculation.
     **/
    class ItemGroup
    {
      public:
        ItemGroup(RootItemGroup *rootGroup);
        ~ItemGroup();

        // find all items intersecting with the item specified and
        // populate m_groupItems
        void populateGroup(int thisItem);

        class Request
        {
          public:
            /**
             * Create a push request. No calculations will be performed.
             *
             * @param sourceGroup the group that posted the request, or
             *                    NULL if it was posted manually.
             * @param sourceGroupPushRequested how much the posting group wanted
             *                                 to move itself when the request was
             *                                 posted (if sourceGroup is not NULL)
             * @param pushRequested how much the group concerned is asked to move
             **/
            Request(
                ItemGroup *sourceGroup,
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
            void activate(ItemGroup *group);

            // saved from constructor
            ItemGroup *m_sourceGroup;
            qreal m_sourceGroupPushRequested;
            qreal m_pushRequested;

            // true if the request has already been reached by applyResults
            // and compensated for the reduction of the requester's move
            bool m_compensated;
        };

        /**
         * Post a move request.
         * This adds the request to the group and calls activate on it.
         **/
        void addRequest (const class Request &request);

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
        void applyResults(ItemGroup *cameFrom);

        // the maximum of all push requests
        qreal m_largestPushRequested;
        // the available space calculated so-far
        qreal m_pushAvailable;

      private:
        struct RemainingItem;
        struct RemainingItem
        {
            int item;
            RemainingItem *prev;
            RemainingItem *next;
            int locked;
            bool deleteLater;
        };
        void RemainingUnlink(struct RemainingItem **remainingFirst, struct RemainingItem *item);
        void populateGroupRecurser(int thisItem, struct RemainingItem **remainingFirst);

        // TODO: improve performance of these recursive lookup functions

        // return true if the item is either in this group
        // of any of the groups above
        bool itemIsCurrentOrAbove(int item);
        bool itemIsCurrentOrAboveRecurser(QList<ItemGroup *> *visited, ItemGroup *current, int item);

        // check if a group node containing the item already exists
        ItemGroup * findGroup(int item);
        ItemGroup * findGroupRecurser(QList<ItemGroup *> *visited, ItemGroup *current, int item);

        // the root group
        RootItemGroup *m_root;

        // move requests posted to this group
        QList<Request> m_requests;
        // items belonging to this group
        QList<int> m_groupItems;
        // groups we asked to move
        QList<ItemGroup *> m_obstacles;
    };

    // root group, contains some config vars
    class RootItemGroup : public ItemGroup
    {
      public:
        ItemSpace *m_itemSpace;
        PushPower m_power;
        Direction m_direction;

        RootItemGroup(
            ItemSpace *itemSpace,
            PushPower power,
            Direction direction
        );
    };
};

#endif
