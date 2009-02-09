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

#include <Plasma/Animator>

#include "desktoplayout.h"

DesktopLayout::DesktopLayout()
      : QObject(0),
        temporaryPlacement(false),
        visibilityTolerance(0)
{
    connect(Plasma::Animator::self(), SIGNAL(movementFinished(QGraphicsItem*)),
            this, SLOT(movementFinished(QGraphicsItem*)));
}

void DesktopLayout::addItem(QGraphicsWidget *item, bool pushBack, bool position)
{
    int key = newItemKey();

    QRectF logicalGeom;
    QTransform revertTransform;
    getItemInstantRelativeGeometry(item, logicalGeom, revertTransform);

    //kDebug() << "addItem position" << position << "logical" << logicalGeom;

    if (position) {
        logicalGeom = positionNewItem(logicalGeom.size());
    }

    ItemSpace::ItemSpaceItem spaceItem;
    spaceItem.pushBack = pushBack;
    spaceItem.animateMovement = false;
    spaceItem.preferredPosition = logicalGeom.topLeft();
    spaceItem.lastGeometry = logicalGeom;
    spaceItem.user = QVariant(key);

    DesktopLayoutItem desktopItem;
    desktopItem.item = item;
    desktopItem.temporaryGeometry = QRectF(0, 0, -1, -1);
    desktopItem.revertTransform = revertTransform;

    itemSpace.addItem(spaceItem);
    items.insert(key, desktopItem);
}

QRectF DesktopLayout::positionNewItem(QSizeF itemSize)
{
    // get possible positions
    QList<QPointF> possiblePositions = itemSpace.positionVertically(itemSize, itemSpace.spaceAlignment, false, true);
    //kDebug() << "possiblePositions" << possiblePositions;

    // prefer free positions
    QRectF bestGeometry = QRectF();
    foreach (const QPointF &position, possiblePositions) {
        QRectF geom = QRectF(position, itemSize);
        if (itemSpace.positionedProperly(geom)) {
            bestGeometry = geom;
            break;
        }
    }

    if (!bestGeometry.isValid()) {
        // choose the position that with the best resulting visibility
        QPointF bestPosition = QPointF();
        qreal bestVisibility = 0;
        foreach (const QPointF &position, possiblePositions) {
            // see how much the item can be pushed into the working area:
            // copy our ItemSpace, add the item to the copy
            // and check the resulting position's visibility

            ItemSpace tempItemSpace(itemSpace);

            ItemSpace::ItemSpaceItem spaceItem;
            spaceItem.pushBack = false;
            spaceItem.animateMovement = false;
            spaceItem.preferredPosition = position;
            spaceItem.lastGeometry = QRectF(position, itemSize);
            spaceItem.user = QVariant(-1);

            tempItemSpace.addItem(spaceItem);
            int tempGroup, tempItem;
            tempItemSpace.locateItemByUser(QVariant(-1), &tempGroup, &tempItem);

            QRectF resultingGeom = tempItemSpace.m_groups[tempGroup].m_groupItems[tempItem].lastGeometry;
            qreal visibility = tempItemSpace.positionVisibility(resultingGeom);

            //kDebug() << "Trying " << position << " visibility " << visibility;

            if (visibility > bestVisibility) {
                bestPosition = position;
                bestVisibility = visibility;
                if (visibility >= 1) {
                    break;
                }
            }
        }

        if (bestVisibility < (1.0-visibilityTolerance)) {
            bestPosition = QPointF(itemSpace.screenSpacing, itemSpace.screenSpacing);
        }

        bestGeometry = QRectF(bestPosition, itemSize);
    }

    kDebug() << "Positioned item to" << bestGeometry;

    return bestGeometry;
}

bool DesktopLayout::getPushBack(int index)
{
    int group;
    int item;
    itemSpace.locateItemByPosition(index, &group, &item);

    return itemSpace.m_groups[group].m_groupItems[item].pushBack;
}

QPointF DesktopLayout::getPreferredPosition(int index)
{
    int group;
    int item;
    itemSpace.locateItemByPosition(index, &group, &item);

    return itemSpace.m_groups[group].m_groupItems[item].preferredPosition;
}

QRectF DesktopLayout::getLastGeometry(int index)
{
    int group;
    int item;
    itemSpace.locateItemByPosition(index, &group, &item);

    return itemSpace.m_groups[group].m_groupItems[item].lastGeometry;
}

void DesktopLayout::setPlacementSpacing(qreal spacing)
{
    itemSpace.placementSpacing = spacing;
}

void DesktopLayout::setScreenSpacing(qreal spacing)
{
    itemSpace.screenSpacing = spacing;
}

void DesktopLayout::setShiftingSpacing(qreal spacing)
{
    itemSpace.shiftingSpacing = spacing;
    // NOTE: not wise to call that during operation yet
}

void DesktopLayout::setVisibilityTolerance(qreal part)
{
    visibilityTolerance = part;
}

void DesktopLayout::setWorkingArea(QRectF area)
{
    // itemSpace positions are relative to working area start,
    // adjust them to correspond to the same on-screen positions
    itemSpace.offsetPositions(workingStart - area.topLeft());
    itemSpace.setWorkingArea(area.size());
    workingStart = area.topLeft();
}

void DesktopLayout::setAlignment(Qt::Alignment alignment)
{
    itemSpace.spaceAlignment = alignment;
}

void DesktopLayout::setTemporaryPlacement(bool enabled)
{
    temporaryPlacement = enabled;
}

int DesktopLayout::count () const
{
    return items.size();
}

QGraphicsWidget *DesktopLayout::itemAt (int i) const
{
    int group = -2, item = -2;
    itemSpace.locateItemByPosition(i, &group, &item);
    int itemKey = itemSpace.m_groups[group].m_groupItems[item].user.toInt();

    return items[itemKey].item;
}

void DesktopLayout::removeAt (int i)
{
    int group, item;
    itemSpace.locateItemByPosition(i, &group, &item);
    int itemKey = itemSpace.m_groups[group].m_groupItems[item].user.toInt();

    // remove from ItemSpace
    itemSpace.removeItem(group, item);
    // remove from local list
    items.remove(itemKey);
}

void DesktopLayout::performTemporaryPlacement(int group, int itemInGroup)
{
    ItemSpace::ItemSpaceItem &spaceItem = itemSpace.m_groups[group].m_groupItems[itemInGroup];
    DesktopLayoutItem &item = items[spaceItem.user.toInt()];

    QRectF origGeom = spaceItem.lastGeometry;
    spaceItem.lastGeometry = QRectF();

    QPointF newPos = QPointF(0, 0);
    QList<QPointF> possiblePositions = itemSpace.positionVertically(origGeom.size(), itemSpace.spaceAlignment, true, false);
    if (possiblePositions.count() > 0) {
        newPos = possiblePositions[0];
    }

    spaceItem.lastGeometry = origGeom;
    item.temporaryGeometry = QRectF(newPos, origGeom.size());

    item.item->setGeometry(geometryRelativeToAbsolute(spaceItem.user.toInt(), item.temporaryGeometry));
}

void DesktopLayout::revertTemporaryPlacement(int group, int itemInGroup)
{
    ItemSpace::ItemSpaceItem &spaceItem = itemSpace.m_groups[group].m_groupItems[itemInGroup];
    DesktopLayoutItem &item = items[spaceItem.user.toInt()];

    item.temporaryGeometry = QRectF();

    item.item->setGeometry(geometryRelativeToAbsolute(spaceItem.user.toInt(), spaceItem.lastGeometry));
}

QRectF DesktopLayout::transformRect(const QRectF &rect, const QTransform &transform)
{
    QTransform t;
    t.translate(rect.left(), rect.top());
    t = transform * t;
    t.translate(-rect.left(), -rect.top());
    return t.mapRect(rect);
}

void DesktopLayout::getItemInstantRelativeGeometry(QGraphicsWidget *item, QRectF &outGeometry, QTransform &outRevertTransform)
{
    QRectF origGeom = item->geometry();

    QTransform transform;
    if (item->transform().m11() && item->transform().m22()) {
        transform = item->transform();
    }

    QRectF transformedAbsoluteGeom = transformRect(origGeom, transform);
    QRectF logicalGeom = transformedAbsoluteGeom.translated(-workingStart);

    QPointF positionDiff = origGeom.topLeft() - transformedAbsoluteGeom.topLeft();
    qreal scaleDiffX = origGeom.width() / transformedAbsoluteGeom.width();
    qreal scaleDiffY = origGeom.height() / transformedAbsoluteGeom.height();

    QTransform r;
    r.translate(positionDiff.x(), positionDiff.y());
    r.scale(scaleDiffX, scaleDiffY);

    outGeometry = logicalGeom;
    outRevertTransform = r;
}

QRectF DesktopLayout::geometryRelativeToAbsolute(int itemKey, const QRectF &relative)
{
    QRectF translated = relative.translated(workingStart);
    QRectF detransformed = transformRect(translated, items[itemKey].revertTransform);
    return detransformed;
}

void DesktopLayout::adjustPhysicalPositions()
{
    for (int groupId = 0; groupId < itemSpace.m_groups.size(); groupId++) {
        ItemSpace::ItemGroup &group = itemSpace.m_groups[groupId];

        for (int itemId = 0; itemId < group.m_groupItems.size(); itemId++) {
            ItemSpace::ItemSpaceItem &spaceItem = group.m_groupItems[itemId];
            DesktopLayoutItem &desktopItem = items[spaceItem.user.toInt()];

            // Temporarily place the item if it could not be pushed inside the working area.
            // Put it back if it fits again.
            if (itemSpace.positionVisibility(spaceItem.lastGeometry) < visibilityTolerance) {
                performTemporaryPlacement(groupId, itemId);
            } else if (desktopItem.temporaryGeometry.isValid()) {
                revertTemporaryPlacement(groupId, itemId);
            }

            // Reset the absolute position if needed

            QRectF effectiveGeom = (desktopItem.temporaryGeometry.isValid() ? desktopItem.temporaryGeometry : spaceItem.lastGeometry);
            QRectF absoluteGeom = geometryRelativeToAbsolute(spaceItem.user.toInt(), effectiveGeom);

            if (desktopItem.item->geometry() != absoluteGeom) {
                if (spaceItem.animateMovement)  {
                    Plasma::Animator *anim = Plasma::Animator::self();
                    bool animating = m_animatingItems.contains(desktopItem.item);
                    if (animating) {
                        anim->stopItemMovement(m_animatingItems.value(desktopItem.item));
                    }
                    int id = Plasma::Animator::self()->moveItem(desktopItem.item, Plasma::Animator::FastSlideInMovement,
                                                                absoluteGeom.topLeft().toPoint());
                    if (id > 0) {
                        m_animatingItems.insert(desktopItem.item, id);
                    } else if (animating) {
                        m_animatingItems.remove(desktopItem.item);
                    }

                    spaceItem.animateMovement = false;
                } else {
                    desktopItem.item->setGeometry(absoluteGeom);
                }
            }
        }
    }
}

void DesktopLayout::itemTransformed(QGraphicsWidget *layoutItem, ItemTransformType type)
{
    // get local item key
    int itemKey = -1;
    QMapIterator<int, DesktopLayoutItem> i(items);
    while (i.hasNext()) {
        i.next();
        if (i.value().item == layoutItem) {
            itemKey = i.key();
            break;
        }
    }
    if (itemKey == -1) {
        return;
    }

    // locate item
    int group, item;
    itemSpace.locateItemByUser(itemKey, &group, &item);
    ItemSpace::ItemSpaceItem &spaceItem = itemSpace.m_groups[group].m_groupItems[item];

    QRectF logicalGeom;
    QTransform revertTransform;
    getItemInstantRelativeGeometry(layoutItem, logicalGeom, revertTransform);

    if (type == ItemTransformSelf) {
        // don't care about the new position, just update the size
        itemSpace.resizeItem(group, item, logicalGeom.size());
    }
    else if (spaceItem.lastGeometry != logicalGeom) {
        // use the new geometry as the preferred
        itemSpace.moveItem(group, item, logicalGeom);
    }
    items[itemKey].revertTransform = revertTransform;
}

void DesktopLayout::movementFinished(QGraphicsItem* item)
{
    if (m_animatingItems.contains(item)) {
        m_animatingItems.remove(item);
    }
}

int DesktopLayout::newItemKey()
{
    int from = -1;
    QList<int> usedKeys = items.keys();
    foreach (int key, usedKeys) {
        if (key - from > 1) {
            break;
        }
        from = key;
    }
    return from+1;
}

#include <desktoplayout.moc>
