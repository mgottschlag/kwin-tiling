/***************************************************************************
 *   Copyright (C) 2008 by Lukas Appelhans                                 *
 *   l.appelhans@gmx.de                                                    *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/
#include "QuicklaunchLayout.h"

#include <Plasma/IconWidget>

QuicklaunchLayout::QuicklaunchLayout(int rowCount, QGraphicsWidget *parentWidget, QGraphicsLayoutItem *parent)
  : QGraphicsGridLayout(parent), m_rowCount(rowCount)
{
}

QuicklaunchLayout::~QuicklaunchLayout()
{
}

void QuicklaunchLayout::setRowCount(int rowCount) 
{
    m_rowCount = rowCount;
}

void QuicklaunchLayout::addItem(Plasma::IconWidget *icon)
{
    //kDebug() << "Row count is" << rowCount() << "Wanted row count is" << m_rowCount;
    //int row = m_rowCount == rowCount() || rowCount() == -1 ? 0 : rowCount();
    //int column = m_rowCount == rowCount()  || columnCount() == 0 ? columnCount() : columnCount() - 1;
    //kDebug() << "Adding icon to row = " << row << ", column = " << column;
    int row = 0;
    int column = 0;
    while (itemAt(row, column))
    {
        kDebug() << "Row is" << row << "column is" << column;
        if (row < m_rowCount - 1) {
            row++;
        }
        else {
            kDebug() << "column++";
            row = 0;
            column++;
        }
    }
    QGraphicsGridLayout::addItem(icon, row, column);
}

QSizeF QuicklaunchLayout::sizeHint(Qt::SizeHint which, const QSizeF & constraint) const
{
    if (which == Qt::PreferredSize) {
        return QSizeF(columnCount() * geometry().height() / m_rowCount, QGraphicsGridLayout::sizeHint(which, constraint).height());
    }
    return QGraphicsGridLayout::sizeHint(which, constraint);
}
