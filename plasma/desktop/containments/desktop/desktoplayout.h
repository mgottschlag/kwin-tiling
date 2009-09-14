/*
  Copyright (c) 2008 Ambroz Bizjak <ambro@b4ever.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

#ifndef _DESKTOPLAYOUT_H
#define _DESKTOPLAYOUT_H

#include <QHash>
#include <QMap>
#include <QList>
#include <QObject>
#include <QTransform>

#include "itemspace.h"

class DesktopLayout : public QObject
{
    Q_OBJECT

public:
    DesktopLayout ();

    /**
     * Adds a new item.
     *
     * @param item the item to add
     * @param pushBack if the item should attempt to always be in its preferred position;
     *                 if false, it will only move when pushed by other items or edges
     *                 of the working area
     * @param position if the item should be repositioned
     **/
    void addItem(QGraphicsWidget *item, bool pushBack = true, bool position = true);

    bool getPushBack(int index);
    QPointF getPreferredPosition(int index);
    QRectF getLastGeometry(int index);

    /**
     * Sets spacing required between an item and
     * the edge of the working area when a new item is positioned.
     **/
    void setPlacementSpacing(qreal spacing);

    /**
     * Sets spacing between an item and the edge of
     * the working area when items are being pushed around
     * and when an item is moved by the user.
     **/
    void setScreenSpacing(qreal spacing);

    /**
     * Sets spacing between items when items are being pushed around.
     **/
    void setShiftingSpacing(qreal spacing);

    /**
     * Sets the tolerance for temporary placement in terms of surface
     * of the item concerned.
     **/
    void setVisibilityTolerance(qreal part);

    /**
     * Enables or disables temporary placement.
     **/
    void setTemporaryPlacement(bool enabled);

    /**
     * Sets the alignment.
     * This defines the sides of the working area where items are
     * pushed inside in case the working area shrinks.
     * Default is Qt::AlignTop|Qt::AlignLeft which pushes items on
     * the right and the bottom sides.
     **/
    void setAlignment(Qt::Alignment alignment);

    /**
     * Call this to change the working area.
     **/
    void setWorkingArea(QRectF area);

    enum ItemTransformType {
        ItemTransformUser = 1,
        ItemTransformSelf = 2
    };

    /**
     * Call this when an item has been moved/resized/transformed either by the user or itself.
     *
     * @param item the item affected
     * @param type whether the change was by the user (ItemTransformUser) or the applet itself (ItemTransformSelf)
     **/
    void itemTransformed(QGraphicsWidget *item, ItemTransformType type);

    /**
     * Adjusts the items' on-screen positions to match calculations.
     **/
    void adjustPhysicalPositions();

    /**
     * Returns the count of items in the layout.
     **/
    int count() const;

    /**
     * Returns the applet at the specified position.
     **/
    QGraphicsWidget *itemAt(int index) const;

    /**
     * Removes the item at the specified position.
     * The ordering of remaining items after removing one is undefined.
     **/
    void removeAt(int index);

private slots:
    void movementFinished(QGraphicsItem *);

private:

    QRectF positionNewItem(QSizeF itemSize);

    QRectF transformRect(const QRectF &rect, const QTransform &transform);
    void getItemInstantRelativeGeometry(QGraphicsWidget *item, QRectF &outGeometry, QTransform &outRevertTransform);
    QRectF geometryRelativeToAbsolute(int itemKey, const QRectF &relative);

    class DesktopLayoutItem
    {
      public:
        QGraphicsWidget *item;
        QRectF temporaryGeometry;
        QTransform revertTransform;
    };

    int newItemKey();

    // layout status

    /**
     * The ItemSpace where items are stored and calculations are done.
     * We use the 'user' item option as an identifier that survives
     * regrouping.
     **/
    ItemSpace itemSpace;

    /**
     * Item-specific data that cannot be stored in the ItemSpace.
     * Maps integer IDs in the 'user' item field to their local data.
     **/
    QMap<int, DesktopLayoutItem> items;

    QHash<QGraphicsItem*, int> m_animatingItems;
    QPointF workingStart;

    // layout configuration

    bool temporaryPlacement;

    qreal visibilityTolerance;

    // item manipulation functions
    void performTemporaryPlacement(int group, int itemInGroup);
    void revertTemporaryPlacement(int group, int itemInGroup);
};

#endif
