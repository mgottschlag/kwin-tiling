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
#ifndef QUICKLAUNCH_ICONGRIDLAYOUT_H
#define QUICKLAUNCH_ICONGRIDLAYOUT_H

// Qt
#include <QtCore/QList>
#include <QtCore/QSizeF>
#include <QtGui/QGraphicsLayout>

class QEvent;
class QRectF;
class QGraphicsLayoutItem;

namespace Quicklaunch {

class IconGridLayout : public QGraphicsLayout {

public:
    /**
     * This enum type describes whether the layout prefers growing
     * horizontally by creating new columns or whether it prefers growing
     * vertically by creating new rows.
     */
    enum Mode {
        PreferColumns, /**< Prefer columns over rows. */
        PreferRows     /**< Prefer rows over columns. */
    };

    IconGridLayout(QGraphicsLayoutItem *parent = 0);
    ~IconGridLayout();

    Mode mode() const;
    void setMode(Mode mode);

    int cellSpacing() const;
    void setCellSpacing(int cellSpacing);
    int maxRowsOrColumns() const;

    /**
     * Depending on the mode, @c setMaxRowsOrColumns limits either the
     * number of rows or the number of columns that are displayed. In
     * @c PreferColumns mode, @a maxRowsOrColumns applies to the
     * number of columns while in @c PreferRows mode, it applies to
     * the number of rows.
     *
     * Setting @a maxRowsOrColumns to @c 0 disables the limitation.
     *
     * @param maxRowsOrColumns the maximum number of rows or columns
     *    (depending on the mode) that should be displayed.
     */
    void setMaxRowsOrColumns(int maxRowsOrColumns);

    bool maxRowsOrColumnsForced() const;

    void setMaxRowsOrColumnsForced(bool enable);

    void addItem(QGraphicsLayoutItem *item);
    void insertItem(int index, QGraphicsLayoutItem *item);

    int count() const;
    int columnCount() const;
    int rowCount() const;
    QGraphicsLayoutItem *itemAt(int index) const;
    QGraphicsLayoutItem *itemAt(int row, int column) const;
    void moveItem(int from, int to);
    void removeAt(int index);

    void setGeometry(const QRectF &rect);
    QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const;

    static const int DEFAULT_CELL_SPACING;

private:
    void updateGridParameters();

    QList<QGraphicsLayoutItem*> m_items;
    Mode m_mode;
    int m_cellSpacing;
    int m_maxRowsOrColumns;
    bool m_maxRowsOrColumnsForced;

    int m_rowCount;
    int m_columnCount;
    int m_columnWidth;
    int m_rowHeight;
    QSizeF m_preferredSizeHint;
};
}

#endif /* QUICKLAUNCH_ICONGRIDLAYOUT_H */
