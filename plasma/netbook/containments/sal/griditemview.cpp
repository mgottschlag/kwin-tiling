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
    : QGraphicsWidget(parent),
      m_layout(0),
      m_currentIcon(0),
      m_currentIconIndexX(-1),
      m_currentIconIndexY(-1)
{
}

GridItemView::~GridItemView()
{}

void GridItemView::setCurrentItem(Plasma::IconWidget *currentIcon)
{
    if (!m_layout) {
        m_layout = dynamic_cast<QGraphicsGridLayout *>(layout());
    }
    if (!m_layout) {
        return;
    }

    for (int x = 0; x < m_layout->columnCount(); ++x) {
        for (int y = 0; y < m_layout->rowCount(); ++y) {
            if (m_layout->itemAt(y, x) == currentIcon) {
                m_currentIcon = currentIcon;
                m_currentIconIndexX = x;
                m_currentIconIndexY = y;
                emit itemSelected(m_currentIcon);
                break;
            }
        }
    }
}

Plasma::IconWidget *GridItemView::currentItem() const
{
    return m_currentIcon;
}

void GridItemView::keyPressEvent(QKeyEvent *event)
{
    if (!m_layout) {
        m_layout = dynamic_cast<QGraphicsGridLayout *>(layout());
    }
    if (!m_layout || m_layout->columnCount() == 0) {
        return;
    }
    switch (event->key()) {
    case Qt::Key_Left: {
        m_currentIcon = 0;
        while (!m_currentIcon) {
            m_currentIconIndexX = (m_layout->columnCount() + m_currentIconIndexX - 1) % m_layout->columnCount();
            m_currentIcon = static_cast<Plasma::IconWidget *>(m_layout->itemAt(m_currentIconIndexY, m_currentIconIndexX));
        }
        emit itemSelected(m_currentIcon);
        break;
    }
    case Qt::Key_Right: {
        m_currentIcon = 0;
        while (!m_currentIcon) {
            m_currentIconIndexX = (m_currentIconIndexX + 1) % m_layout->columnCount();
            m_currentIcon = static_cast<Plasma::IconWidget *>(m_layout->itemAt(m_currentIconIndexY, m_currentIconIndexX));
        }
        emit itemSelected(m_currentIcon);
        break;
    }
    case Qt::Key_Up: {
        m_currentIcon = 0;
        while (!m_currentIcon) {
            m_currentIconIndexY = (m_layout->rowCount() + m_currentIconIndexY - 1) % m_layout->rowCount();
            m_currentIcon = static_cast<Plasma::IconWidget *>(m_layout->itemAt(m_currentIconIndexY, m_currentIconIndexX));
        }
        emit itemSelected(m_currentIcon);
        break;
    }
    case Qt::Key_Down: {
        m_currentIcon = 0;
        while (!m_currentIcon) {
            m_currentIconIndexY = (m_currentIconIndexY + 1) % m_layout->rowCount();
            m_currentIcon = static_cast<Plasma::IconWidget *>(m_layout->itemAt(m_currentIconIndexY, m_currentIconIndexX));
        }
        emit itemSelected(m_currentIcon);
        break;
    }
    case Qt::Key_Enter:
    case Qt::Key_Return:
        emit itemActivated(m_currentIcon);
        break;
    case Qt::Key_Backspace:
    case Qt::Key_Home:
        emit resetRequested();
        break;
    default:
        break;
    }
}


void GridItemView::focusInEvent(QFocusEvent *event)
{
    Q_UNUSED(event)

    if (!m_layout) {
        m_layout = dynamic_cast<QGraphicsGridLayout *>(layout());
    }
    if (m_layout && m_currentIconIndexX == -1) {
        m_currentIconIndexX = 0;
        m_currentIconIndexY = 0;
        Plasma::IconWidget *icon = static_cast<Plasma::IconWidget*>(m_layout->itemAt(0, 0));
        emit itemSelected(icon);
    }
}

void GridItemView::focusOutEvent(QFocusEvent *event)
{

}