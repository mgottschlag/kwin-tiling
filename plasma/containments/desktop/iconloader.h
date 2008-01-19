/*
*   Copyright 2007 by Christopher Blauvelt <cblauvelt@gmail.com>
*   Copyright (C) 2007 Matt Broadstone <mbroadst@kde.org>
*   Copyright (C) 2007 Matias Costa <m.costacano@gmail.com>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Library General Public License version 2,
*   or (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details
*
*   You should have received a copy of the GNU Library General Public
*   License along with this program; if not, write to the
*   Free Software Foundation, Inc.,
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef PLASMA_ICONLOADER_H
#define PLASMA_ICONLOADER_H

#include <QObject>
#include <QMap>
#include <QString>
#include <QPointF>
#include <QApplication>
#include <QDesktopWidget>

#include <KDirLister>
#include <KFileItem>
#include <plasma/applet.h>

class DefaultDesktop;
class KToggleAction;

class IconLoader : public QObject
{
    Q_OBJECT

public:
    IconLoader(DefaultDesktop *parent);
    ~IconLoader();

    /**
    * Get whether device icons will be displayed or not.
    * @return whether device icons will be displayed
    */
    bool showDeviceIcons() const;
    /**
    * Sets whether to display device icons or not.
    * @param show true sets device icons to be displayed.
    */
    void setShowDeviceIcons(bool show);

    /**
     * Reloads the configuration for the icon manager
     */
    void reloadConfig();

    bool isGridAligned() const;
    void setGridAligned(bool align);
    QSizeF gridSize() const;

    /**
     * Places an item on the desktop in the first available space.
     */
    void alignToGrid(Plasma::Applet *item);
    void alignToGrid(Plasma::Applet *item, const QPoint &pos, bool moveIntersectingItems=false);

    /**
     * Get the grid square index where a point in local coordinates is located.
     * In normal circustances desktop coordinates is screen position.
     * A grid place is like a square in a checkboard.
     *
     *
     * @param pos A point in desktop coordinates
     * @return The grid 2D index pos is located.
     */
    inline QPoint mapToGrid(const QPointF &pos) const;
    
    /**
     * A grid place is like a square in a checkboard, and this method returns
     * its center point. So if you want the top left corner gridSize()/2 must
     * be substracted.
     *
     * @param pos A grid place index
     * @return The center point of the grid square indexed by pos.
     */
    inline QPointF mapFromGrid(const QPoint &pos) const;

    /**
     * Sets the grid size to the nearest size which results in
     * a integer number of grid places. If the desktop width is 1024
     * and the desired grid width is 100 the resulting gridSize will be
     * 102.4 because it's the nearest width with a integer number of
     * divisions (10)
     *
     * @arg gridSize The desired grid size to be set.
     */
    void setGridSize(const QSizeF& gridSize);

    /**
     * The width and height of the size returned by this method represents
     * the grid places a Applet can be placed. A grid place is like a
     * square in a checkboard, this size represents the white and black squares
     * for each border.
     *
     * @return The grid places for each dimension
     */
    QSize gridDimensions() const;

    /**
     * The actions associated with the menu.
     * @return The menu items relating to icons.
     */
    virtual QList<QAction*> contextActions();

    bool showIcons() const;
    void setShowIcons(bool iconsVisible);

    //inline const QList<Plasma::Applet*>& desktopItems();


private:
    void addIcon(const KUrl &url);
    void addIcon(Plasma::Applet *applet);
    void deleteIcon(const KUrl &url);
    void deleteIcon(Plasma::Applet *applet);
    void configureMedia();
    void createMenu();

    void alignIcons();
    void setToGrid(Plasma::Applet* icon, const QPoint p);
    QRectF nextFreeRect(const QRectF itemRect);
    QRectF nextFreeRect(const QRectF itemRect, QList<Plasma::Applet*> placedItems);
    bool intersectsWithItems(const QRectF item, const QList<Plasma::Applet*> &items) const;
    QRectF advanceAlongGrid(QRectF rect);
    //bool shiftIcon(Plasma::Applet *icon, Plasma::Applet*);
    bool isFreePlace(const QPointF &p);
    inline QRectF availableGeometry() const;

    KDirLister m_desktopDir;
    Plasma::DataEngine *m_solidEngine;
    QHash<QString, Plasma::Applet*> m_iconMap;
    QHash<QString, Plasma::Applet*> m_solidDevices;
    DefaultDesktop *m_desktop;

    QList<QAction*> actions;

    Qt::Orientation m_orientation;
    bool m_showIcons;
    bool m_gridAlign;
    bool m_enableMedia;
    QSizeF m_gridSize;

    static const qreal topBorder=20.0;

private Q_SLOTS:
    void init();
    void newItems(const KFileItemList& items);
    void deleteItem(const KFileItem item);
    void appletDeleted(Plasma::Applet *applet);
    void sourceAdded(const QString &source);
    void sourceDeleted(const QString &source);

    void slotAlignHorizontal();
    void slotAlignVertical();
};

QPoint IconLoader::mapToGrid(const QPointF &pos) const
{
    return QPoint(qRound(pos.x()/m_gridSize.width()),
                  qRound(pos.y()/m_gridSize.height()));
}

QPointF IconLoader::mapFromGrid(const QPoint &pos) const
{
    return QPointF(pos.x()*m_gridSize.width(),
                  pos.y()*m_gridSize.height());
}

QRectF IconLoader::availableGeometry() const
{
    QRectF desktop(QApplication::desktop()->availableGeometry(0));
    //take this out once availableGeometry gives us the expected result
    desktop.setBottom(desktop.bottom()-48.0);
    return desktop;
}

#endif
