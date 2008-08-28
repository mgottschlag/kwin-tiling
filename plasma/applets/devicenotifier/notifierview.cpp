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
#include <QtGui/QStandardItemModel>

//KDE
#include <KDebug>
#include <KIconLoader>
#include <KColorScheme>
#include <KGlobalSettings>

//Plasma
#include <plasma/delegate.h>

using namespace Notifier;

NotifierView::NotifierView(QWidget *parent)
    : QTreeView(parent)
{
    setIconSize(QSize(KIconLoader::SizeMedium, KIconLoader::SizeMedium));
    setRootIsDecorated(true);
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
    Q_UNUSED(event)
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
    Q_UNUSED(event);
    if (!model()) {
        return;
    }

    QPainter painter(viewport());
    painter.setRenderHint(QPainter::Antialiasing);

    const int rows = model()->rowCount(rootIndex());
    const int cols = header()->count();
    //kDebug() << "painting" << rows << "rows" << cols << "columns";

    int verticalOffset = TOP_OFFSET;

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            const QModelIndex index = model()->index(i, j, rootIndex());
            if (model()->hasChildren(index)) {
              QRect itemRect(QPoint(HEADER_LEFT_MARGIN, verticalOffset), QSize(0,0));
              const QFontMetrics fm(KGlobalSettings::smallestReadableFont());
              int minHeight = HEADER_HEIGHT;
              itemRect.setSize(QSize(width() - HEADER_LEFT_MARGIN,
                          qMax(fm.height() + (HEADER_TOP_MARGIN), minHeight)
                          + HEADER_BOTTOM_MARGIN));

              verticalOffset += itemRect.size().height();


              //paint the parent
              if (event->region().contains(itemRect)) {
                  //kDebug()<<"header"<<itemRect;
                  QStyleOptionViewItem option = viewOptions();
                  option.rect = itemRect;
                  const int rightMargin = style()->pixelMetric(QStyle::PM_ScrollBarExtent) + 6;
                  const int dy = HEADER_TOP_MARGIN;

                  painter.save();
                  painter.setRenderHint(QPainter::Antialiasing, false);

                  QLinearGradient gradient(option.rect.topLeft(), option.rect.topRight());
                  gradient.setColorAt(0.0, Qt::transparent);
                  gradient.setColorAt(0.1, option.palette.midlight().color());
                  gradient.setColorAt(0.5, option.palette.mid().color());
                  gradient.setColorAt(0.9, option.palette.midlight().color());
                  gradient.setColorAt(1.0, Qt::transparent);
                  painter.setPen(QPen(gradient, 1));

                  painter.drawLine(option.rect.x() + 6, option.rect.y() + dy + 2,
                                    option.rect.right() - rightMargin , option.rect.y() + dy + 2);
                  painter.setFont(KGlobalSettings::smallestReadableFont());
                  painter.setPen(QPen(KColorScheme(QPalette::Active).foreground(KColorScheme::InactiveText), 0));
                  QString text = index.data(Qt::DisplayRole).value<QString>();
                  painter.drawText(option.rect.adjusted(0, dy, -rightMargin, 0),
                                    Qt::AlignVCenter|Qt::AlignRight, text);
                  painter.restore();
              }
            }

            QStandardItemModel * currentModel = dynamic_cast<QStandardItemModel *>(model());
            QStandardItem *currentItem = currentModel->itemFromIndex(index);
            //we display the childs of this item
            for (int j=0; j < currentItem->rowCount(); ++j) {
                for (int k=0; k < currentItem->columnCount(); ++k) {
                    QStandardItem *childItem = currentItem->child(j, k);
                    //const QRect itemChildRect = visualRect(childItem->index());
                    QRect itemChildRect(QPoint(HEADER_LEFT_MARGIN, verticalOffset), QSize(0,0));
                    QModelIndex childIndex = childItem->index();
                    itemChildRect.setSize(QSize(width() - style()->pixelMetric(QStyle::PM_ScrollBarExtent) + 2,sizeHintForIndex(index).height()));
                    verticalOffset += itemChildRect.size().height();
                    if (event->region().contains(itemChildRect)) {
                        paintItem(painter,itemChildRect,childIndex);
                    }
                }
            }
        }
    }
}

void NotifierView::paintItem(QPainter &painter,const QRect &itemRect,const QModelIndex &index)
{
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

#include "notifierview.moc"
