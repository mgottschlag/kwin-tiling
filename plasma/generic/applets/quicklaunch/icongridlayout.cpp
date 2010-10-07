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
#include <QtGui/QSizePolicy>

// KDE
#include <KIconLoader>

// stdlib
#include <math.h>

namespace Quicklaunch {

const int IconGridLayout::DEFAULT_CELL_SPACING = 4;

IconGridLayout::IconGridLayout(QGraphicsLayoutItem *parent)
    : QGraphicsLayout(parent),
      m_items(),
      m_mode(PreferRows),
      m_cellSpacing(DEFAULT_CELL_SPACING),
      m_maxRowsOrColumns(0),
      m_maxRowsOrColumnsForced(false),
      m_rowCount(0),
      m_columnCount(0),
      m_columnWidth(0),
      m_rowHeight(0)
{
    setContentsMargins(0, 0, 0, 0);

    QSizePolicy sizePolicy(
        QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    sizePolicy.setHeightForWidth(true);
    sizePolicy.setHorizontalStretch(1);
    sizePolicy.setVerticalStretch(1);
    setSizePolicy(sizePolicy);
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
    updateGridParameters();
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
    updateGridParameters();
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
    updateGridParameters();
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
    updateGridParameters();
    invalidate();
}

void IconGridLayout::addItem(QGraphicsLayoutItem *item)
{
    m_items.append(item);
    addChildLayoutItem(item);
    item->setParentLayoutItem(this);
    updateGridParameters();
    invalidate();
}

void IconGridLayout::insertItem(int index, QGraphicsLayoutItem *item)
{
    m_items.insert(index, item);
    addChildLayoutItem(item);
    item->setParentLayoutItem(this);
    updateGridParameters();
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
    return m_items[row * m_columnCount + column];
}

void IconGridLayout::removeAt(int index)
{
    QGraphicsLayoutItem *item = m_items.takeAt(index);
    item->setParentLayoutItem(0);

    if (item->ownedByLayout()) {
        delete item;
    }

    updateGridParameters();
    invalidate();
}

void IconGridLayout::setGeometry(const QRectF &rect)
{
    QGraphicsLayout::setGeometry(rect);

    updateGridParameters();

    qreal offsetLeft =
        qMax<qreal>(
            contentsRect().left(),
            (contentsRect().width() - preferredWidth()) / 2);

    qreal offsetTop =
        qMax<qreal>(
            contentsRect().top(),
            (contentsRect().height() - preferredHeight()) / 2);

    int itemCount = m_items.size();

    for (int i = 0; i < itemCount; i++) {

        int row = i / m_columnCount;
        int column = i % m_columnCount;

        m_items[i]->setGeometry(
            QRectF(
                offsetLeft + column * (m_columnWidth + m_cellSpacing),
                offsetTop + row * (m_rowHeight + m_cellSpacing),
                m_columnWidth, m_rowHeight));
    }
}

QSizeF IconGridLayout::sizeHint(
    Qt::SizeHint which, const QSizeF &constraint) const
{
    Q_UNUSED(constraint);

    switch(which) {
        case Qt::PreferredSize: return m_preferredSizeHint;

        default:
            return QSizeF();
    }
}

void IconGridLayout::updateGridParameters()
{
    const int itemCount = m_items.size();

    if (itemCount == 0) {
        m_rowCount = 0;
        m_columnCount = 0;
        m_columnWidth = 0;
        m_rowHeight = 0;
        return;
    }

    const int width = int(contentsRect().width());
    const int height = int(contentsRect().height());

    // Determine the minimum and preferred cell size according to the
    // children's constraints.
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
            m_rowCount = height / (minCellHeight + m_cellSpacing);
            m_rowCount = qBound(1, m_rowCount, itemCount);

            if (m_maxRowsOrColumns > 0) {
                m_rowCount = qMin(m_rowCount, m_maxRowsOrColumns);
            }
        }
        m_columnCount = ceil(double(itemCount) / m_rowCount);
    } else { // m_mode == PreferColumns

        if (m_maxRowsOrColumns > 0 && m_maxRowsOrColumnsForced) {
            m_columnCount = qMin(itemCount, m_maxRowsOrColumns);
        }
        else {
            m_columnCount = width / (minCellWidth + m_cellSpacing);
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

    m_columnWidth = qMax(minCellWidth, availableCellSize);
    m_rowHeight = qMax(minCellHeight, availableCellSize);

    // Giving the availableCellHeight as preferred cell width
    // (and the other way round) is a bit of a hack that  makes the panel allocate just
    // the right size and allows us to request more width when
    // the height changes.
    int preferredWidth =
        m_columnCount * (qMax(minCellWidth, availableCellHeight) + m_cellSpacing) - m_cellSpacing;

    int preferredHeight =
        m_rowCount * (qMax(minCellHeight, availableCellWidth) + m_cellSpacing) - m_cellSpacing;

    if (preferredWidth != int(m_preferredSizeHint.width()) ||
        preferredHeight != int(m_preferredSizeHint.height())) {

        m_preferredSizeHint = QSizeF(preferredWidth, preferredHeight);
        updateGeometry();
    }
}
}
