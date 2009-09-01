/*
 *   Copyright 2009 by Marco Martin <notmart@gmail.com>
 *
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

#include "griditemview.h"

#include <QGraphicsGridLayout>

#include <Plasma/IconWidget>

GridItemView::GridItemView(QGraphicsWidget *parent)
    : Plasma::Frame(parent),
      m_layout(0),
      m_currentIcon(0),
      m_currentIconIndexX(-1),
      m_currentIconIndexY(-1)
{
}

GridItemView::~GridItemView()
{}

void GridItemView::keyPressEvent(QKeyEvent *event)
{
    if (!m_layout) {
        m_layout = dynamic_cast<QGraphicsGridLayout *>(layout());
    }
    if (!m_layout) {
        return;
    }
    switch (event->key()) {
    case Qt::Key_Left: {
        m_currentIconIndexX = (m_layout->columnCount() + m_currentIconIndexX - 1) % m_layout->columnCount();
        m_currentIcon = static_cast<Plasma::IconWidget *>(m_layout->itemAt(m_currentIconIndexY, m_currentIconIndexX));
        emit itemSelected(m_currentIcon);
        break;
    }
    case Qt::Key_Right: {
        m_currentIconIndexX = (m_currentIconIndexX + 1) % m_layout->columnCount();
        m_currentIcon = static_cast<Plasma::IconWidget *>(m_layout->itemAt(m_currentIconIndexY, m_currentIconIndexX));
        emit itemSelected(m_currentIcon);
        break;
    }
    case Qt::Key_Up: {
        m_currentIconIndexY = (m_layout->columnCount() + m_currentIconIndexY - 1) % m_layout->columnCount();
        m_currentIcon = static_cast<Plasma::IconWidget *>(m_layout->itemAt(m_currentIconIndexY, m_currentIconIndexX));
        emit itemSelected(m_currentIcon);
        break;
    }
    case Qt::Key_Down: {
        m_currentIconIndexY = (m_currentIconIndexY + 1) % m_layout->columnCount();
        m_currentIcon = static_cast<Plasma::IconWidget *>(m_layout->itemAt(m_currentIconIndexY, m_currentIconIndexX));
        emit itemSelected(m_currentIcon);
        break;
    }
    case Qt::Key_Enter:
    case Qt::Key_Return:
        emit itemActivated(m_currentIcon);
    default:
        break;
    }
}


void GridItemView::focusInEvent(QFocusEvent *event)
{
    if (!m_layout) {
        m_layout = dynamic_cast<QGraphicsGridLayout *>(layout());
    }
    if (m_layout) {
        m_currentIconIndexX = 0;
        m_currentIconIndexY = 0;
        Plasma::IconWidget *icon = static_cast<Plasma::IconWidget*>(m_layout->itemAt(0, 0));
        emit itemSelected(icon);
    }
}

void GridItemView::focusOutEvent(QFocusEvent *event)
{

}