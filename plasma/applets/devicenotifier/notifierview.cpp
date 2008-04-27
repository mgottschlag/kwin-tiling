/*
    Copyright 2007 by Alexis Ménard <darktears31@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "notifierview.h"

// Qt

#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtGui/QPaintEvent>
#include <QtGui/QScrollBar>
#include <QtGui/QHeaderView>

//KDE
#include <KDebug>
#include <KIconLoader>

//Plasma
#include <plasma/delegate.h>

using namespace Notifier;

NotifierView::NotifierView(QWidget *parent)
    : QTreeView(parent)
{
    setIconSize(QSize(KIconLoader::SizeMedium, KIconLoader::SizeMedium));
    setRootIsDecorated(false);
    setHeaderHidden(true);
    setMouseTracking(true);
}

NotifierView::~NotifierView()
{

}

void NotifierView::resizeEvent(QResizeEvent * event)
{
    //the columns after the first are squares KIconLoader::SizeMedium x KIconLoader::SizeMedium,
    //the first column takes all the remaining space
    if (header()->count() > 0) {
        const int newWidth = event->size().width() -
                             (header()->count()-1)*(sizeHintForRow(0));
        header()->resizeSection(0, newWidth);
    }
}

void NotifierView::mouseMoveEvent(QMouseEvent *event)
{
    const QModelIndex itemUnderMouse = indexAt(event->pos());
    if (itemUnderMouse != m_hoveredIndex && itemUnderMouse.isValid() &&
        state() == NoState) {
        update(itemUnderMouse);
        update(m_hoveredIndex);

        m_hoveredIndex = itemUnderMouse;
        setCurrentIndex(m_hoveredIndex);
    } else if (!itemUnderMouse.isValid()) {
        m_hoveredIndex = QModelIndex();
        setCurrentIndex(m_hoveredIndex);
    }

    QAbstractItemView::mouseMoveEvent(event);
}

void NotifierView::leaveEvent(QEvent *event)
{
    const QModelIndex oldHoveredIndex = m_hoveredIndex;
    m_hoveredIndex = QModelIndex();
    setCurrentIndex(m_hoveredIndex);
    update(oldHoveredIndex);
}

QModelIndex NotifierView::moveCursor(CursorAction cursorAction,Qt::KeyboardModifiers modifiers )
{
    m_hoveredIndex = QModelIndex();

    return QTreeView::moveCursor(cursorAction, modifiers );
}

void NotifierView::paintEvent(QPaintEvent *event)
{
    if (!model()) {
        return;
    }

    QPainter painter(viewport());
    painter.setRenderHint(QPainter::Antialiasing);

    const int rows = model()->rowCount(rootIndex());
    const int cols = header()->count();
    //kDebug() << "painting" << rows << "rows" << cols << "columns";

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            const QModelIndex index = model()->index(i, j, rootIndex());
            const QRect itemRect = visualRect(index);

            if (event->region().contains(itemRect)) {
                QStyleOptionViewItem option = viewOptions();
                option.rect = itemRect;

                if (selectionModel()->isSelected(index)) {
                    option.state |= QStyle::State_Selected;
                }
                if (index == m_hoveredIndex) {
                    option.state |= QStyle::State_MouseOver;
                }
                if (index == currentIndex()) {
                    option.state |= QStyle::State_HasFocus;
                }

                itemDelegate(index)->paint(&painter,option,index);
            }
        }
    }
}

#include "notifierview.moc"
