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
#ifndef QUICKLAUNCH_LAUNCHERLIST_H
#define QUICKLAUNCH_LAUNCHERLIST_H

// Qt
#include <Qt>
#include <QtCore/QList>
#include <QtCore/QPointF>
#include <QtGui/QGraphicsWidget>

// KDE
#include <KUrl>

// Plasma
#include <Plasma/Plasma>

// Own
#include "launcherdata.h"

class QAction;
class QEvent;
class QGraphicsItem;
class QGraphicsLayout;
class QGraphicsLinearLayout;
class QGraphicsSceneResizeEvent;
class QGraphicsSceneDragDropEvent;
class QGraphicsSceneMouseEvent;

namespace Plasma {
    class IconWidget;
}

namespace Quicklaunch {

class DropMarker;
class IconGridLayout;
class Launcher;

/**
 * The LauncherList is a QGraphicsWidget that displays and manages Launchers.
 * Depending on its type, it uses different layouts for presenting
 * the launchers it governs. Since all launchers are managed by the
 * LauncherList and its layout, they should not be accessed directly, but
 * rather added or removed by passing LauncherData objects.
 *
 * LauncherList also takes care of drag & drop handling.
 */
class LauncherList : public QGraphicsWidget
{
    Q_OBJECT

public:

    enum LauncherListType {
        IconGrid, /** A grid of launchers managed by an IconGridLayout. */
        IconList  /** A vertical list of launchers. */
    };

    /**
     * Creates a new LauncherList of the given type with the given parent item.
     *
     * @param type the type of the LauncherList
     * @param parent the parent QGraphicsItem
     */
    LauncherList(LauncherListType type, QGraphicsItem *parent = 0);

    bool launcherNamesVisible() const;
    void setLauncherNamesVisible(bool enable);

    void setPreferredIconSize(int size);


    /**
     * Indicates whether this LauncherList is locked and thus does not allow
     * adding, removing or reordering launchers by drag & drop.
     */
    bool locked() const;

    /**
     * Locks or unlocks this LauncherList.
     *
     * @param enable whether this LauncherList should be locked, thereby
     * disabling adding, removing or reordering launchers by drag & drop.
     */
    void setLocked(bool enable);

    /**
     * Returns the IconGridLayout used to layout LauncherLists that have
     * the type IconGrid. Returns 0 if this LauncherList is of a different type.
     * This is a convenience method that allows callers to avoid casting
     * the QGraphicsLayout returned by layout().
     *
     * @return the IconGridLayout used by this LauncherList
     */
    IconGridLayout *gridLayout() const;

    /**
     * Returns the QGraphicsLinearLayout used to layout LauncherLists that have
     * the type IconList. Returns 0 if this LauncherList is of a different type.
     * This is a convenience method that allows callers to avoid casting
     * the QGraphicsLayout returned by layout().
     *
     * @return the QGraphicsLinearLayout used by this LauncherList
     */
    QGraphicsLinearLayout *listLayout() const;

    int launcherCount() const;

    void clear();
    void insert(int index, const LauncherData &launcherData);
    void insert(int index, const QList<LauncherData> &launcherDataList);
    void removeAt(int index);
    LauncherData launcherAt(int index) const;
    int launcherIndexAtPosition(const QPointF& pos) const;

    bool eventFilter(QObject *watched, QEvent *event);

    /**
     * Returns the type of this LauncherList.
     *
     * @return the type of this LauncherList
     */
    LauncherListType launcherListType() const;

Q_SIGNALS:
    /**
     * Indicates a change to one or more of the displayed launchers.
     */
    void launchersChanged();

    /**
     * Indicates that one of the launcher items was clicked.
     */
    void launcherClicked();

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

    LauncherListType m_type;
    QList<Launcher*> m_launchers;
    bool m_launcherNamesVisible;
    QSizeF m_preferredIconSize;
    bool m_locked;

    QGraphicsLayout *m_layout;

    QPointF m_mousePressedPos;
    DropMarker *m_dropMarker;
    int m_dropMarkerIndex;
    Plasma::IconWidget *m_placeHolder;
};
}

#endif /* QUICKLAUNCH_LAUNCHERLIST_H */
