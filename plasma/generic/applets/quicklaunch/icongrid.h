/***************************************************************************
 *   Copyright (C) 2010 by Ingomar Wesp <ingomar@wesp.name>                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 ***************************************************************************/
#ifndef QUICKLAUNCH_ICONGRID_H
#define QUICKLAUNCH_ICONGRID_H

// Qt
#include <QtCore/QList>
#include <QtCore/QPointF>
#include <QtGui/QGraphicsWidget>

// KDE
#include <KUrl>

// Plasma
#include <Plasma/Plasma>

// Own
#include "itemdata.h"

class QAction;
class QEvent;
class QGraphicsItem;
class QGraphicsSceneResizeEvent;
class QGraphicsSceneDragDropEvent;
class QGraphicsSceneMouseEvent;

namespace Plasma {
    class IconWidget;
}

namespace Quicklaunch {

class DropMarker;
class IconGridLayout;
class QuicklaunchIcon;

class IconGrid : public QGraphicsWidget
{
    Q_OBJECT

public:
    IconGrid(
        Plasma::FormFactor formFactor,
        QGraphicsItem *parent = 0);

    bool iconNamesVisible();
    void setIconNamesVisible(bool enable);

    int cellSpacing() const;
    void setCellSpacing(int cellSpacing);
    int maxRowsOrColumns() const;
    void setMaxRowsOrColumns(int maxRowsOrColumns);
    bool maxRowsOrColumnsForced() const;
    void setMaxRowsOrColumnsForced(bool enable);

    void setFormFactor(Plasma::FormFactor formFactor);

    int iconCount() const;
    int displayedItemCount() const;

    void insert(int index, const ItemData &itemData);
    void insert(int index, const QList<ItemData> &itemDataList);
    void removeAt(int index);
    ItemData iconAt(int index) const;
    int iconIndexAtPosition(const QPointF& pos) const;

    bool eventFilter(QObject *watched, QEvent *event);

Q_SIGNALS:
    /**
     * Indicates that the number of displayed items (including the
     * placeholder and drop marker) changed.
     */
    void displayedItemCountChanged();

    /**
     * Indicates a change to the displayed quicklaunch icons.
     */
    void iconsChanged();

    /**
     * Indicates that one of the launcher items was clicked.
     */
    void iconClicked();

protected:
    void dragEnterEvent(QGraphicsSceneDragDropEvent *event);
    void dragMoveEvent(QGraphicsSceneDragDropEvent *event);
    void dragLeaveEvent(QGraphicsSceneDragDropEvent *event);
    void dropEvent(QGraphicsSceneDragDropEvent *event);

private Q_SLOTS:
    void onPlaceHolderActivated();

private:
    void initPlaceHolder();
    void deletePlaceHolder();
    int determineDropMarkerIndex(const QPointF &localPos) const;

    QList<QuicklaunchIcon*> m_icons;
    bool m_iconNamesVisible;

    IconGridLayout *m_layout;

    QPointF m_mousePressedPos;
    DropMarker *m_dropMarker;
    int m_dropMarkerIndex;
    Plasma::IconWidget *m_placeHolder;
};
}

#endif /* QUICKLAUNCH_ICONGRID_H */
