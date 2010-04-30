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
#include "icongridlayout.h"

// Qt
#include <QtCore/QList>
#include <QtCore/QSizeF>
#include <QtCore/QRectF>
#include <QtGui/QGraphicsLayout>
#include <QtGui/QGraphicsLayoutItem>

// KDE
#include <KDebug>
#include <KIconLoader>

// stdlib
#include <math.h>

namespace Quicklaunch {

const int IconGridLayout::DEFAULT_CELL_SPACING = 2;

const int IconGridLayout::MIN_CELL_SIZE_HINT = KIconLoader::SizeSmall;
const int IconGridLayout::DEFAULT_CELL_SIZE_HINT = KIconLoader::SizeSmall;
const int IconGridLayout::MAX_CELL_SIZE_HINT = KIconLoader::SizeEnormous;

IconGridLayout::IconGridLayout(QGraphicsLayoutItem *parent)
    : QGraphicsLayout(parent),
      m_items(),
      m_mode(PreferRows),
      m_cellSpacing(DEFAULT_CELL_SPACING),
      m_cellSizeHint(DEFAULT_CELL_SIZE_HINT),
      m_maxRowsOrColumns(0),
      m_maxRowsOrColumnsForced(false),
      m_rowCount(0),
      m_columnCount(0),
      m_cellWidth(0),
      m_cellHeight(0)
{
    setContentsMargins(0, 0, 0, 0);
}

IconGridLayout::~IconGridLayout()
{
    Q_FOREACH(QGraphicsLayoutItem *item, m_items) {
        if (item->ownedByLayout()) {
            delete item;
        }
    }
    m_items.clear();
}

IconGridLayout::Mode IconGridLayout::mode() const
{
    return m_mode;
}

void IconGridLayout::setMode(Mode mode)
{
    if (mode == m_mode) {
        return;
    }

    m_mode = mode;
    invalidate();
}

int IconGridLayout::cellSizeHint() const
{
    return m_cellSizeHint;
}

void IconGridLayout::setCellSizeHint(int cellSizeHint)
{
    cellSizeHint =
        qBound(MIN_CELL_SIZE_HINT, cellSizeHint, MAX_CELL_SIZE_HINT);

    if (cellSizeHint == m_cellSizeHint) {
        return;
    }

    m_cellSizeHint = cellSizeHint;
    invalidate();
}

int IconGridLayout::cellSpacing() const
{
    return m_cellSpacing;
}

void IconGridLayout::setCellSpacing(int cellSpacing)
{
    cellSpacing = qMax(0, cellSpacing);

    if (cellSpacing == m_cellSpacing) {
        return;
    }

    m_cellSpacing = cellSpacing;
    invalidate();
}

int IconGridLayout::maxRowsOrColumns() const
{
    return m_maxRowsOrColumns;
}

void IconGridLayout::setMaxRowsOrColumns(int maxRowsOrColumns)
{
    if (m_maxRowsOrColumns == maxRowsOrColumns) {
        return;
    }

    m_maxRowsOrColumns = maxRowsOrColumns;
    invalidate();
}

bool IconGridLayout::maxRowsOrColumnsForced() const
{
    return m_maxRowsOrColumnsForced;
}

void IconGridLayout::setMaxRowsOrColumnsForced(bool enable)
{
    if (m_maxRowsOrColumnsForced == enable) {
        return;
    }

    m_maxRowsOrColumnsForced = enable;
    invalidate();
}

void IconGridLayout::addItem(QGraphicsLayoutItem *item)
{
    m_items.append(item);
    addChildLayoutItem(item);
    item->setParentLayoutItem(this);
    invalidate();
}

void IconGridLayout::insertItem(int index, QGraphicsLayoutItem *item)
{
    m_items.insert(index, item);
    addChildLayoutItem(item);
    item->setParentLayoutItem(this);
    invalidate();
}

void IconGridLayout::moveItem(int from, int to)
{
    m_items.move(from, to);
    invalidate();
}

int IconGridLayout::rowCount() const
{
    return m_rowCount;
}

int IconGridLayout::columnCount() const
{
    return m_columnCount;
}

int IconGridLayout::count() const
{
    return m_items.size();
}

QGraphicsLayoutItem *IconGridLayout::itemAt(int index) const
{
    return m_items[index];
}

QGraphicsLayoutItem *IconGridLayout::itemAt(int row, int column) const
{
    // kDebug() << "Requested:" << row << column << ",rows" << m_rowCount << ",cols" << m_columnCount;
    // kDebug() << "Item count " << count();
    return m_items[row * m_columnCount + column];
}

void IconGridLayout::removeAt(int index)
{
    QGraphicsLayoutItem *item = m_items.takeAt(index);
    item->setParentLayoutItem(0);

    if (item->ownedByLayout()) {
        delete item;
    }

    invalidate();
}

void IconGridLayout::invalidate()
{
    updateGridParameters();
    QGraphicsLayout::invalidate();
}

void IconGridLayout::setGeometry(const QRectF &rect)
{
    QGraphicsLayout::setGeometry(rect);
    updateGridParameters();

    qreal offsetLeft =
        qMax(0.0, (geometry().width() - preferredWidth()) / 2);

    qreal offsetTop =
        qMax(0.0, (geometry().height() - preferredHeight()) / 2);

    int itemCount = m_items.size();

    for (int i = 0; i < itemCount; i++) {

        int row = i / m_columnCount;
        int column = i % m_columnCount;

        m_items[i]->setGeometry(
            QRectF(
            offsetLeft + column * (m_cellWidth + m_cellSpacing),
            offsetTop + row * (m_cellHeight + m_cellSpacing),
            m_cellWidth, m_cellHeight));
    }
}

QSizeF IconGridLayout::sizeHint(
    Qt::SizeHint which, const QSizeF &constraint) const
{
    return QSizeF();
}

void IconGridLayout::updateGridParameters()
{
    const int itemCount = m_items.size();

    if (itemCount == 0) {
        m_rowCount = 0;
        m_columnCount = 0;
        m_cellWidth = 0;
        m_cellHeight = 0;
        return;
    }

    const int width = int(contentsRect().width());
    const int height = int(contentsRect().height());

    // Determine the minimum cell size according to widget constraints.
    int minCellWidth = 0;
    int minCellHeight = 0;

    Q_FOREACH(QGraphicsLayoutItem *item, m_items) {
        minCellWidth = qMax(minCellWidth, (int)item->minimumWidth());
        minCellHeight = qMax(minCellHeight, (int)item->minimumHeight());
    }

    if (m_mode == PreferRows) {

        if (m_maxRowsOrColumns > 0 && m_maxRowsOrColumnsForced) {
            m_rowCount = qMin(itemCount, m_maxRowsOrColumns);
        }
        else {
            int desiredRowHeight = qMax(minCellHeight, m_cellSizeHint);
            m_rowCount = height / (desiredRowHeight + m_cellSpacing);
            m_rowCount = qBound(1, m_rowCount, itemCount);

            if (m_maxRowsOrColumns > 0) {
                m_rowCount = qMin(m_rowCount, m_maxRowsOrColumns);
            }
        }
        m_columnCount = ceil(double(itemCount) / m_rowCount);
    } else {

        if (m_maxRowsOrColumns > 0 && m_maxRowsOrColumnsForced) {
            m_columnCount = qMin(itemCount, m_maxRowsOrColumns);
        }
        else {
            int desiredColumnWidth = qMax(minCellWidth, m_cellSizeHint);
            m_columnCount = width / (desiredColumnWidth + m_cellSpacing);
            m_columnCount = qBound(1, m_columnCount, itemCount);

            if (m_maxRowsOrColumns > 0) {
                m_columnCount = qMin(m_columnCount, m_maxRowsOrColumns);
            }
        }
        m_rowCount = ceil(double(itemCount) / m_columnCount);
    }

    Q_ASSERT(m_rowCount > 0 && m_columnCount > 0);

    const int availableCellWidth =
        (width - (m_columnCount - 1) * m_cellSpacing) / m_columnCount;

    const int availableCellHeight =
        (height - (m_rowCount - 1) * m_cellSpacing) / m_rowCount;

    const int availableCellSize = qMin(availableCellWidth, availableCellHeight);

    m_cellWidth = qMax(minCellWidth, availableCellSize);
    m_cellHeight = qMax(minCellHeight, availableCellSize);

    // Giving the availableCellHeight as preferred cell width
    // (and the other way round) is a bit of a hack that  makes the panel allocate just
    // the right size and allows us to request more width when
    // the height changes.
    int preferredWidth =
        m_columnCount * (qMax(minCellWidth, availableCellHeight) + m_cellSpacing) - m_cellSpacing;

    int preferredHeight =
        m_rowCount * (qMax(minCellHeight, availableCellWidth) + m_cellSpacing) - m_cellSpacing;

    setPreferredSize(QSizeF(preferredWidth, preferredHeight));
}

void IconGridLayout::widgetEvent(QEvent *event)
{
    // kDebug() << event;
    // invalidate();
    QGraphicsLayout::widgetEvent(event);
}
}

