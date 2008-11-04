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

DesktopLayout::DesktopLayout(QGraphicsLayoutItem *parent)
      : QObject(0),
        QGraphicsLayout(parent),
        autoWorkingArea(true),
        temporaryPlacement(false),
        visibilityTolerance(0),
        m_activated(false)
{
    connect(Plasma::Animator::self(), SIGNAL(movementFinished(QGraphicsItem*)),
            this, SLOT(movementFinished(QGraphicsItem*)));
}

void DesktopLayout::addItem(QGraphicsLayoutItem *item, bool pushBack, const QRectF &preferredGeom, const QRectF &lastGeom)
{
    int key = newItemKey();

    ItemSpace::ItemSpaceItem spaceItem;
    spaceItem.pushBack = pushBack;
    spaceItem.animateMovement = false;
    spaceItem.preferredGeometry = preferredGeom;
    spaceItem.lastGeometry = (lastGeom.isValid() ? lastGeom : preferredGeom);
    spaceItem.user = QVariant(key);

    DesktopLayoutItem desktopItem;
    desktopItem.item = item;
    desktopItem.temporaryGeometry = QRectF(0, 0, -1, -1);

    itemSpace.addItem(spaceItem);
    items.insert(key, desktopItem);

    invalidate();
}

void DesktopLayout::addItem(QGraphicsLayoutItem *item, bool pushBack, const QSizeF &size)
{
    QSizeF itemSize = ( size.isValid() ? size : item->effectiveSizeHint(Qt::PreferredSize) );

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
            // copy our ItemSpace, add the item to the copy, activate it
            // and check the resulting position's visibility

            ItemSpace tempItemSpace(itemSpace);

            ItemSpace::ItemSpaceItem spaceItem;
            spaceItem.pushBack = pushBack;
            spaceItem.animateMovement = false;
            spaceItem.preferredGeometry = QRectF(position, itemSize);
            spaceItem.lastGeometry = QRectF(position, itemSize);
            spaceItem.user = QVariant(-1);

            tempItemSpace.addItem(spaceItem);
            tempItemSpace.activate();
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

    addItem(item, pushBack, bestGeometry);
    kDebug() << "Positioned item to" << bestGeometry;
}

bool DesktopLayout::getPushBack(int index)
{
    int group;
    int item;
    itemSpace.locateItemByPosition(index, &group, &item);

    return itemSpace.m_groups[group].m_groupItems[item].pushBack;
}

QRectF DesktopLayout::getPreferredGeometry(int index)
{
    int group;
    int item;
    itemSpace.locateItemByPosition(index, &group, &item);

    return itemSpace.m_groups[group].m_groupItems[item].preferredGeometry;
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
    invalidate();
}

void DesktopLayout::setShiftingSpacing(qreal spacing)
{
    itemSpace.shiftingSpacing = spacing;
    // NOTE: not wise to call that during operation yet
}

void DesktopLayout::setVisibilityTolerance(qreal part)
{
    visibilityTolerance = part;
    invalidate();
}

void DesktopLayout::setWorkingArea(QRectF area)
{
    itemSpace.setWorkingArea(area.size());
    workingStart = area.topLeft();
    invalidate();
}

void DesktopLayout::setAlignment(Qt::Alignment alignment)
{
    itemSpace.spaceAlignment = alignment;
    invalidate();
}

void DesktopLayout::setTemporaryPlacement(bool enabled)
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

    invalidate();
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
    item.item->setGeometry(item.temporaryGeometry.translated(workingStart));
}

void DesktopLayout::revertTemporaryPlacement(int group, int itemInGroup)
{
    ItemSpace::ItemSpaceItem &spaceItem = itemSpace.m_groups[group].m_groupItems[itemInGroup];
    DesktopLayoutItem &item = items[spaceItem.user.toInt()];

    item.temporaryGeometry = QRectF();
    item.item->setGeometry(spaceItem.lastGeometry.translated(workingStart));
}

// update anything that needs updating
void DesktopLayout::setGeometry(const QRectF &rect)
{
    m_activated = true;
    QGraphicsLayout::setGeometry(rect);

    if (autoWorkingArea || !itemSpace.workingGeom.isValid()) {
        setWorkingArea(rect);
    }

    // activate the ItemSpace to perform motion as needed
    itemSpace.activate();

    for (int groupId = 0; groupId < itemSpace.m_groups.size(); groupId++) {
        ItemSpace::ItemGroup &group = itemSpace.m_groups[groupId];

        for (int itemId = 0; itemId < group.m_groupItems.size(); itemId++) {
            ItemSpace::ItemSpaceItem &spaceItem = group.m_groupItems[itemId];
            DesktopLayoutItem &desktopItem = items[spaceItem.user.toInt()];

            //  Temporarily place the item if it could not be pushed inside the working area.
            //  Put it back if it fits again.
            if (itemSpace.positionVisibility(spaceItem.lastGeometry) < visibilityTolerance) {
                performTemporaryPlacement(groupId, itemId);
            } else if (desktopItem.temporaryGeometry.isValid()) {
                revertTemporaryPlacement(groupId, itemId);
            }

            // Reset the absolute position if needed
            QRectF visibleGeom = (desktopItem.temporaryGeometry.isValid() ? desktopItem.temporaryGeometry : spaceItem.lastGeometry);
            QRectF absoluteGeom = visibleGeom.translated(workingStart);
            if (desktopItem.item->geometry() != absoluteGeom) {
                QGraphicsWidget *w = dynamic_cast<QGraphicsWidget*>(desktopItem.item);
                if (w && spaceItem.animateMovement)  {
                    Plasma::Animator *anim = Plasma::Animator::self();
                    bool animating = m_animatingItems.contains(w);
                    if (animating) {
                        anim->stopItemMovement(m_animatingItems.value(w));
                    }
                    int id = Plasma::Animator::self()->moveItem(w, Plasma::Animator::FastSlideInMovement,
                                                                absoluteGeom.topLeft().toPoint());
                    if (id > 0) {
                        m_animatingItems.insert(w, id);
                    } else if (animating) {
                        m_animatingItems.remove(w);
                    }

                    spaceItem.animateMovement = false;
                } else {
                    desktopItem.item->setGeometry(absoluteGeom);
                }
            }
        }
    }
    m_activated = false;
}

// This should be called when the geometry of an item has been changed.
// If the change was made by the user, the new position is used as the preferred position.
void DesktopLayout::itemGeometryChanged(QGraphicsLayoutItem *layoutItem)
{
    if (m_activated || m_animatingItems.contains(dynamic_cast<QGraphicsWidget*>(layoutItem))) {
        return;
    }

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

    QRectF currentRelative = layoutItem->geometry().translated(-workingStart);
    if (spaceItem.lastGeometry != currentRelative) {
        spaceItem.lastGeometry = currentRelative;
        spaceItem.preferredGeometry = currentRelative;

        itemSpace.updateItem(group, item);
        invalidate();
    }
}

QSizeF DesktopLayout::sizeHint (Qt::SizeHint which, const QSizeF &constraint) const
{
    Q_UNUSED(which)
    Q_UNUSED(constraint)
    return QSizeF();
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
