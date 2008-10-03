/*
  Copyright (c) 2008 Ambroz Bizjak <ambro@b4ever.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

/*
  ItemSpace class
  Implements "push and pull" dynamics of rectangular items in 2D space.

  All isolated motion folows these rules:
   - overlapping items stay at the same positions relative to each other
   - non-overlapping items stay such and do not jump one over another

  There are two types of motion:
   - forced motion: an item moves all items on its way, even if any of them
     would intersect the border of the working area
   - non-forced motion: an item moves items on its way only as much as the border
     of the working area

  Items are pushed to fit inside the working area:
  if an item is in the way of one of the borders of alignment, the move is forced;
  if an item is in the way of one of the opposite borders, the move is non-forced.

  An item can have a "preferred position". Such item is moved non-forced in the
  direction towards the preferred position.
*/

#ifndef _ITEMSPACE_H
#define _ITEMSPACE_H

#include <QRectF>
#include <QList>

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

    enum Direction {
        DirLeft = 1,
        DirRight = 2,
        DirUp = 4,
        DirDown = 8
    };

    void offsetPositions(const QPointF &offset);
    qreal performPush(int itemIndex, Direction direction, qreal amount, qreal minAmount, bool ignoreBorder);
    void findPullGroup(int thisItem, QList<int> *currentItems);
    qreal pushItem(int itemIndex, Direction direction, qreal amount, const QList<int> *previousItems, bool doPush, bool ignoreBorder);

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
};

#endif
