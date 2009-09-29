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
#include <QTimer>

#include <KIconLoader>

#include <Plasma/IconWidget>
#include <Plasma/ItemBackground>

GridItemView::GridItemView(QGraphicsWidget *parent)
    : QGraphicsWidget(parent),
      m_currentIcon(0),
      m_orientation(Qt::Vertical),
      m_currentIconIndexX(-1),
      m_currentIconIndexY(-1),
      m_iconSize(KIconLoader::SizeHuge),
      m_maxColumnWidth(0),
      m_maxRowHeight(1)
{
    m_layout = new QGraphicsGridLayout(this);

    setFocusPolicy(Qt::StrongFocus);
    setAcceptHoverEvents(true);
    m_hoverIndicator = new Plasma::ItemBackground(this);
    m_hoverIndicator->setZValue(-100);
    m_hoverIndicator->hide();

    m_relayoutTimer = new QTimer(this);
    m_relayoutTimer->setSingleShot(true);
    connect(m_relayoutTimer, SIGNAL(timeout()), this, SLOT(relayout()));
}

GridItemView::~GridItemView()
{}

void GridItemView::setCurrentItem(Plasma::IconWidget *currentIcon)
{
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

    m_hoverIndicator->setTargetItem(currentIcon);
}

Plasma::IconWidget *GridItemView::currentItem() const
{
    return m_currentIcon;
}

void GridItemView::insertItem(Plasma::IconWidget *icon, qreal weight)
{
    qreal left, top, right, bottom;
    m_hoverIndicator->getContentsMargins(&left, &top, &right, &bottom);
    icon->setContentsMargins(left, top, right, bottom);

    icon->setMinimumSize(icon->sizeFromIconSize(m_iconSize));
    icon->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    if (icon->size().width() > m_maxColumnWidth) {
        m_maxColumnWidth = icon->size().width();
    }
    if (icon->size().height() > m_maxRowHeight) {
        m_maxRowHeight = icon->size().height();
    }
    icon->hide();

    if (weight != -1 || m_items.count() == 0) {
        m_items.insert(weight, icon);
    } else {
        m_items.insert(m_items.uniqueKeys().last()+1, icon);
    }

    connect(icon, SIGNAL(destroyed(QObject *)), this, SLOT(itemRemoved(QObject *)));

    m_relayoutTimer->start(400);
}

void GridItemView::clear()
{
    m_hoverIndicator->setTargetItem(0);
    for (int i = 0; i < m_layout->count(); ++i) {
        m_layout->removeAt(0);
    }
    foreach (Plasma::IconWidget *icon, m_items) {
        icon->deleteLater();
    }
    m_items.clear();
}

int GridItemView::count() const
{
    return m_layout->count();
}

void GridItemView::setOrientation(Qt::Orientation orientation)
{
    m_orientation = orientation;
}

Qt::Orientation GridItemView::orientation() const
{
    return m_orientation;
}

void GridItemView::setIconSize(int size)
{
    m_iconSize = size;
}

int GridItemView::iconSize() const
{
    return m_iconSize;
}

void GridItemView::relayout()
{
    if (m_layout->count() == 0) {
        hide();
    }

    //Relayout the grid
    QList<Plasma::IconWidget *>orderedItems = m_items.values();
    int validIndex = 0;

    QSizeF availableSize;
    QGraphicsWidget *pw = parentWidget();
    //FIXME: if this widget will be scrollwidget::widget itself this part could become a bit prettier
    if (pw) {
        pw->resize(0,0);
        availableSize = pw->size();
    } else {
        resize(0,0);
        availableSize = size();
    }

    if (size().width() <= availableSize.width()) {
        foreach (Plasma::IconWidget *icon, orderedItems) {
            if (m_layout->itemAt(validIndex) == icon) {
                ++validIndex;
            } else {
                break;
            }
        }
    }

    for (int i = validIndex; i < m_layout->count(); ++i) {
        m_layout->removeAt(validIndex);
    }

    if (m_orientation == Qt::Vertical) {

        int nColumns;
        // if we already decided how many columns are going to be don't decide again
        if (validIndex > 0 && m_layout->columnCount() > 0 &&  m_layout->rowCount() > 0) {
            nColumns = m_layout->columnCount();
        } else {
            nColumns = qMax(1, int(availableSize.width() / m_maxColumnWidth));
        }
        int i = 0;


        foreach (Plasma::IconWidget *icon, orderedItems) {
            if (i < validIndex) {
                ++i;
                continue;
            }

            m_layout->addItem(icon, i / nColumns, i % nColumns);
            m_layout->setAlignment(icon, Qt::AlignHCenter);
            icon->show();
            ++i;
        }
    } else {

        int nRows;
        // if we already decided how many columns are going to be don't decide again
        if (validIndex > 0 && m_layout->columnCount() > 0 &&  m_layout->rowCount() > 0) {
            nRows = m_layout->rowCount();
        } else {
            nRows = qMax(1, int(availableSize.height() / m_maxRowHeight));
        }
        int i = 0;


        foreach (Plasma::IconWidget *icon, orderedItems) {
            if (i < validIndex) {
                ++i;
                continue;
            }

            m_layout->addItem(icon, i % nRows, i / nRows);
            m_layout->setAlignment(icon, Qt::AlignHCenter);
            icon->show();
            ++i;
        }
    }

    if (!isVisible()) {
        m_layout->activate();
        show();
    }
}

void GridItemView::itemRemoved(QObject *object)
{
    Plasma::IconWidget *icon = static_cast<Plasma::IconWidget *>(object);

    QMapIterator<qreal, Plasma::IconWidget *> i(m_items);
    while (i.hasNext()) {
        i.next();
        if (i.value() == icon) {
            m_items.remove(i.key());
            break;
        }
    }

    m_relayoutTimer->start(400);
}

void GridItemView::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Left: {
        m_currentIcon = 0;
        while (!m_currentIcon) {
            m_currentIconIndexX = (m_layout->columnCount() + m_currentIconIndexX - 1) % m_layout->columnCount();
            m_currentIcon = static_cast<Plasma::IconWidget *>(m_layout->itemAt(m_currentIconIndexY, m_currentIconIndexX));
        }
        m_hoverIndicator->setTargetItem(m_currentIcon);
        emit itemSelected(m_currentIcon);
        break;
    }
    case Qt::Key_Right: {
        m_currentIcon = 0;
        while (!m_currentIcon) {
            m_currentIconIndexX = (m_currentIconIndexX + 1) % m_layout->columnCount();
            m_currentIcon = static_cast<Plasma::IconWidget *>(m_layout->itemAt(m_currentIconIndexY, m_currentIconIndexX));
        }
        m_hoverIndicator->setTargetItem(m_currentIcon);
        emit itemSelected(m_currentIcon);
        break;
    }
    case Qt::Key_Up: {
        m_currentIcon = 0;
        while (!m_currentIcon) {
            m_currentIconIndexY = (m_layout->rowCount() + m_currentIconIndexY - 1) % m_layout->rowCount();
            m_currentIcon = static_cast<Plasma::IconWidget *>(m_layout->itemAt(m_currentIconIndexY, m_currentIconIndexX));
        }
        m_hoverIndicator->setTargetItem(m_currentIcon);
        emit itemSelected(m_currentIcon);
        break;
    }
    case Qt::Key_Down: {
        m_currentIcon = 0;
        while (!m_currentIcon) {
            m_currentIconIndexY = (m_currentIconIndexY + 1) % m_layout->rowCount();
            m_currentIcon = static_cast<Plasma::IconWidget *>(m_layout->itemAt(m_currentIconIndexY, m_currentIconIndexX));
        }
        m_hoverIndicator->setTargetItem(m_currentIcon);
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

    if (m_layout && m_layout->count() > 0 && m_currentIconIndexX == -1) {
        m_currentIconIndexX = 0;
        m_currentIconIndexY = 0;
        Plasma::IconWidget *icon = static_cast<Plasma::IconWidget*>(m_layout->itemAt(0, 0));
        emit itemSelected(icon);
    }
}

void GridItemView::focusOutEvent(QFocusEvent *event)
{
    Q_UNUSED(event)

    m_hoverIndicator->hide();
}

void GridItemView::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event)

    if (!hasFocus()) {
        m_hoverIndicator->hide();
    }
}

void GridItemView::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    Q_UNUSED(event)

    m_relayoutTimer->start();
}

#include <griditemview.moc>
