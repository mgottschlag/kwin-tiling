/*
  Copyright (c) 2008 Ambroz Bizjak <ambro@b4ever.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

#ifndef _DESKTOPLAYOUT_H
#define _DESKTOPLAYOUT_H

#include <QGraphicsLayout>
#include <QHash>
#include <QMap>
#include <QList>
#include <QObject>

#include "itemspace.h"

class QGraphicsItem;

class DesktopLayout : public QObject, public QGraphicsLayout
{
    Q_OBJECT

public:
    DesktopLayout (QGraphicsLayoutItem *parent = 0);

    /**
     * Adds a new item.
     * The item will be automatically positioned.
     *
     * @param item the item to add
     * @param pushBack if the item should attempt to always be in its preferred position;
     *                 if false, it will only move when pushed by other items or edges
     *                 of the working area
     * @param size the size initial size of the item; if invalid or not supplied,
     *             the PreferredSize hint will be used
     **/
    void addItem(QGraphicsLayoutItem *item, bool pushBack = true, const QSizeF &size = QSizeF());

    /**
     * Adds a previously managed item.
     **/
    void addItem(QGraphicsLayoutItem *item, bool pushBack, const QRectF &preferredGeom, const QRectF &lastGeom = QRectF());

    bool getPushBack(int index);
    QRectF getPreferredGeometry(int index);
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
     * Sets whether the working area should always be
     * considered the geometry of the managed widget.
     * Default is on.
     **/
    void setAutoWorkingArea(bool value);

    /**
     * Sets the area of the widget where items can be displayed.
     * If you turned off auto working area, you have to use
     * this function to adjust it manually every time it changes.
     **/
    void setWorkingArea(QRectF area);

    /**
     * Sets the alignment.
     * This defines the sides of the working area where items are
     * pushed inside in case the working area shrinks.
     * Default is Qt::AlignTop|Qt::AlignLeft which pushes items on
     * the right and the bottom sides.
     **/
    void setAlignment(Qt::Alignment alignment);

    /**
     * Enables or disables temporary placement.
     **/
    void setTemporaryPlacement(bool enabled);

    /**
     * Checks if the specified item's geometry has been changed externally
     * and sets its preferred position to the current position.
     * Call this when an item has been manually moved or resized.
     **/
    void itemGeometryChanged(QGraphicsLayoutItem *layoutItem);

    // inherited from QGraphicsLayout
    int count() const;
    QGraphicsLayoutItem *itemAt(int index) const;
    void removeAt(int index);

    // inherited from QGraphicsLayoutItem
    void setGeometry(const QRectF &rect);
    QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const;

private slots:
    void movementFinished(QGraphicsItem*);

private:

    class DesktopLayoutItem
    {
      public:
        QGraphicsLayoutItem *item;
        QRectF temporaryGeometry;
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

    bool autoWorkingArea;
    bool temporaryPlacement;

    qreal visibilityTolerance;

    // item manipulation functions
    void performTemporaryPlacement(int group, int itemInGroup);
    void revertTemporaryPlacement(int group, int itemInGroup);

    bool m_activated;
};

#endif
