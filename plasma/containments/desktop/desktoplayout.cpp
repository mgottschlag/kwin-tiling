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
    autoWorkingArea(true),
    temporaryPlacement(false),
    itemRelativeTolerance(0)
{
}

void DesktopLayout::addItem (QGraphicsLayoutItem *item, bool pushBack, const QRectF &preferredGeom, const QRectF &lastGeom)
{
    DesktopLayoutItem newItem;
    newItem.item = item;
    newItem.temporaryGeometry = QRectF(0, 0, -1, -1);

    items.append(newItem);
    itemSpace.addItem(pushBack, preferredGeom, lastGeom);

    invalidate();
}

void DesktopLayout::addItem (QGraphicsLayoutItem *item, bool pushBack, const QSizeF &size)
{
    QSizeF itemSize = ( size.isValid() ? size : item->effectiveSizeHint(Qt::PreferredSize) );

    // get possible positions
    QList<QPointF> possiblePositions = itemSpace.positionVertically(itemSize, itemSpace.spaceAlignment, false, true);
    //kDebug() << "possiblePositions" << possiblePositions;

    // prefer free positions
    QRectF bestGeometry = QRectF();
    foreach (QPointF position, possiblePositions) {
        kDebug() << "Checkfreeing" << position;

        QRectF geom = QRectF(position, itemSize);
        if (itemSpace.positionedProperly(geom)) {
            kDebug() << "FREE";
            bestGeometry = geom;
            break;
        }
    }

    if (!bestGeometry.isValid()) {
        // choose the position that with the best resulting visibility
        QPointF bestPosition = QPointF();
        qreal bestVisibility = 0;
        foreach (QPointF position, possiblePositions) {
            // see how much the item can be pushed into the working area:
            // copy our ItemSpace, add the item to the copy, activate it
            // and check the resulting position's visibility
            ItemSpace tempItemSpace(itemSpace);
            tempItemSpace.addItem(pushBack, QRectF(position, itemSize));
            tempItemSpace.activate();
            qreal visibility = tempItemSpace.positionVisibility(tempItemSpace.count()-1);
            //kDebug() << "Trying " << position << " visibility " << visibility;

            if (visibility > bestVisibility) {
                bestPosition = position;
                bestVisibility = visibility;
                if (visibility >= 1) {
                    break;
                }
            }
        }

        if (bestVisibility < (1.0-itemRelativeTolerance)) {
            bestPosition = QPointF(itemSpace.screenSpacing, itemSpace.screenSpacing);
        }

        bestGeometry = QRectF(bestPosition, itemSize);
    }

    addItem(item, pushBack, bestGeometry);
    kDebug() << "Positioned item to" << bestGeometry;
}

bool DesktopLayout::getPushBack (int index)
{
    return itemSpace.items[index].pushBack;
}

QRectF DesktopLayout::getPreferredGeometry (int index)
{
    return itemSpace.items[index].preferredGeometry;
}

QRectF DesktopLayout::getLastGeometry (int index)
{
    return itemSpace.items[index].lastGeometry;
}

void DesktopLayout::setPlacementSpacing (qreal spacing)
{
    itemSpace.placementSpacing = spacing;
}

void DesktopLayout::setScreenSpacing (qreal spacing)
{
    itemSpace.screenSpacing = spacing;
    invalidate();
}

void DesktopLayout::setShiftingSpacing (qreal spacing)
{
    itemSpace.shiftingSpacing = spacing;
    // NOTE: not wise to call that during operation yet
}

void DesktopLayout::setItemRelativeTolerance(qreal part)
{
    itemRelativeTolerance = part;
    invalidate();
}

void DesktopLayout::setWorkingArea (QRectF area)
{
    itemSpace.setWorkingArea(area.size());
    workingStart = area.topLeft();
    invalidate();
}

void DesktopLayout::setAlignment (Qt::Alignment alignment)
{
    itemSpace.spaceAlignment = alignment;
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
    itemSpace.removeAt(i);
    items.removeAt(i);
    invalidate();
}

void DesktopLayout::performTemporaryPlacement (int itemIndex)
{
    DesktopLayoutItem &item = items[itemIndex];
    ItemSpace::ItemSpaceItem &spaceItem = itemSpace.items[itemIndex];

    QRectF origGeom = spaceItem.lastGeometry;
    spaceItem.lastGeometry = QRectF();

    QPointF newPos = QPointF(0, 0);
    QList<QPointF> possiblePositions = itemSpace.positionVertically(origGeom.size(), itemSpace.spaceAlignment, true, false);
    if (possiblePositions.count() > 0) {
        newPos = possiblePositions[0];
    }
    
    kDebug() << "Temp placing" << itemIndex << "to" << newPos;
    spaceItem.lastGeometry = origGeom;
    item.temporaryGeometry = QRectF(newPos, origGeom.size());
    item.item->setGeometry(item.temporaryGeometry.translated(workingStart));
}

void DesktopLayout::revertTemporaryPlacement (int itemIndex)
{
    DesktopLayoutItem &item = items[itemIndex];
    ItemSpace::ItemSpaceItem &spaceItem = itemSpace.items[itemIndex];

    kDebug() << "Reverting temp placing" << itemIndex;
    item.temporaryGeometry = QRectF();
    item.item->setGeometry(spaceItem.lastGeometry.translated(workingStart));
}

// update anything that needs updating
void DesktopLayout::setGeometry(const QRectF &rect)
{
    QGraphicsLayout::setGeometry(rect);

    if (autoWorkingArea || !itemSpace.workingGeom.isValid()) {
        setWorkingArea(rect);
    }

    // activate the ItemSpace to perform motion as needed
    itemSpace.activate();

    /*
      Temporarily place items that could not be pushed inside the working area.
      Put them back if they fit again.
    */
    for (int i=0; i<items.size(); i++) {
        DesktopLayoutItem &item = items[i];
        if (itemSpace.positionVisibility(i) < itemRelativeTolerance) {
            if (temporaryPlacement) {
                performTemporaryPlacement(i);
            }
        } else if (item.temporaryGeometry.isValid()) {
            revertTemporaryPlacement(i);
        }
    }

    // reset the absolute positions of applets
    for (int i=0; i<items.size(); i++) {
        DesktopLayoutItem &item = items[i];
        ItemSpace::ItemSpaceItem &spaceItem = itemSpace.items[i];

        QRectF absoluteGeom = (item.temporaryGeometry.isValid() ? item.temporaryGeometry : spaceItem.lastGeometry).translated(workingStart);
        if (item.item->geometry() != absoluteGeom) {
            item.item->setGeometry(absoluteGeom);
        }
    }
}

// This should be called when the geometry of an item has been changed.
// If the change was made by the user, the new position is used as the preferred position.
// TODO: refresh the layout without that item so things jump back and then add the item to the new position
void DesktopLayout::itemGeometryChanged (QGraphicsLayoutItem *layoutItem)
{
    if (isActivated()) return;

    for (int i=0; i<items.size(); i++) {
        DesktopLayoutItem &item = items[i];
        ItemSpace::ItemSpaceItem &spaceItem = itemSpace.items[i];

        if (item.item == layoutItem) {
            QRectF currentRelative = item.item->geometry().translated(-workingStart);
            if (spaceItem.lastGeometry != currentRelative) {
                spaceItem.lastGeometry = currentRelative;
                spaceItem.preferredGeometry = currentRelative;
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
