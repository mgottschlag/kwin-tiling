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
#include <KDebug>
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
      m_maxSectionCount(0),
      m_maxSectionCountForced(false),
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

int IconGridLayout::maxSectionCount() const
{
    return m_maxSectionCount;
}

void IconGridLayout::setMaxSectionCount(int maxSectionCount)
{
    if (m_maxSectionCount == maxSectionCount) {
        return;
    }

    m_maxSectionCount = maxSectionCount;
    updateGridParameters();
    invalidate();
}

bool IconGridLayout::maxSectionCountForced() const
{
    return m_maxSectionCountForced;
}

void IconGridLayout::setMaxSectionCountForced(bool enable)
{
    if (m_maxSectionCountForced == enable) {
        return;
    }

    m_maxSectionCountForced = enable;
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

    /* qreal offsetLeft =
        qMax<qreal>(
            contentsRect().left(),
            (contentsRect().width() - preferredWidth()) / 2);

    qreal offsetTop =
        qMax<qreal>(
            contentsRect().top(),
            (contentsRect().height() - preferredHeight()) / 2);

    kDebug() << "Get rect:" << rect;
    kDebug() << "Offsets: " << offsetLeft << "," << offsetTop; */

    int itemCount = m_items.size();

    for (int i = 0; i < itemCount; i++) {

        int row = i / m_columnCount;
        int column = i % m_columnCount;

        m_items[i]->setGeometry(
            QRectF(
                /* offsetLeft + */ column * (m_columnWidth + m_cellSpacing),
                /* offsetTop + */ row * (m_rowHeight + m_cellSpacing),
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
    // children's size hints.
    int minColumnWidth = 0;
    int minRowHeight = 0;

    int maxUnconstrainedPreferredColumnWidth = 0;
    int maxUnconstrainedPreferredRowHeight = 0;

    Q_FOREACH(QGraphicsLayoutItem *item, m_items) {
        minColumnWidth = qMax(minColumnWidth, (int)item->minimumWidth());
        minRowHeight = qMax(minRowHeight, (int)item->minimumHeight());

        maxUnconstrainedPreferredColumnWidth =
            qMax(maxUnconstrainedPreferredColumnWidth, (int)item->preferredWidth());

        maxUnconstrainedPreferredRowHeight =
            qMax(maxUnconstrainedPreferredRowHeight, (int)item->preferredHeight());
    }

    if (m_mode == PreferRows) {

        if (m_maxSectionCount > 0 && m_maxSectionCountForced) {
            m_rowCount = qMin(itemCount, m_maxSectionCount);
        }
        else {
            m_rowCount = height / (minRowHeight + m_cellSpacing);
            m_rowCount = qBound(1, m_rowCount, itemCount);

            if (m_maxSectionCount > 0) {
                m_rowCount = qMin(m_rowCount, m_maxSectionCount);
            }
        }
        m_columnCount = ceil(double(itemCount) / m_rowCount);
    } else { // m_mode == PreferColumns

        if (m_maxSectionCount > 0 && m_maxSectionCountForced) {
            m_columnCount = qMin(itemCount, m_maxSectionCount);
        }
        else {
            m_columnCount = width / (minColumnWidth + m_cellSpacing);
            m_columnCount = qBound(1, m_columnCount, itemCount);

            if (m_maxSectionCount > 0) {
                m_columnCount = qMin(m_columnCount, m_maxSectionCount);
            }
        }
        m_rowCount = ceil(double(itemCount) / m_columnCount);
    }

    Q_ASSERT(m_rowCount > 0 && m_columnCount > 0);

    m_columnWidth =
        (width - (m_columnCount - 1) * m_cellSpacing) / m_columnCount;

    m_rowHeight =
        (height - (m_rowCount - 1) * m_cellSpacing) / m_rowCount;

    kDebug() << "Reported height: " << height;
    kDebug() << "Row height: " << m_rowHeight;


    int preferredWidth;
    int preferredHeight;

    if (m_mode == PreferRows) {
        preferredWidth = 0;

        for (int i = 0; i < m_columnCount; i++) {
            preferredWidth += m_items.at(i)->effectiveSizeHint(Qt::PreferredSize, QSizeF(-1, m_rowHeight)).width();
            kDebug() << "Item said, it would like to have a width of " << m_items.at(i)->effectiveSizeHint(Qt::PreferredSize, QSizeF(-1, m_rowHeight)).width() << " for the given height of " << m_rowHeight;
        }

        preferredWidth += (m_columnCount - 1) * m_cellSpacing;
        preferredHeight =
            m_rowCount * (maxUnconstrainedPreferredRowHeight + m_cellSpacing) - m_cellSpacing;
    } else { // m_mode == PreferColumns

        preferredHeight = 0;

        Q_FOREACH(QGraphicsLayoutItem *item, m_items) {
            preferredHeight += item->effectiveSizeHint(Qt::PreferredSize, QSizeF(m_columnWidth, -1)).height();
        }

        preferredHeight += (m_rowCount - 1) * m_cellSpacing;
        preferredWidth =
            m_columnCount * (maxUnconstrainedPreferredColumnWidth + m_cellSpacing) - m_cellSpacing;
    }

    if (preferredWidth != int(m_preferredSizeHint.width()) ||
        preferredHeight != int(m_preferredSizeHint.height())) {

        m_preferredSizeHint = QSizeF(preferredWidth, preferredHeight);
        kDebug() << "Reporting preferred size: " << m_preferredSizeHint;
        updateGeometry();
    }
}
}
