/*
    Copyright 2007 by Alexis Ménard <darktears31@gmail.com>
    Copyright 2009 by Giulio Camuffo <giuliocamuffo@gmail.com>

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
#include <KMenu>

//Plasma
#include <Plasma/Delegate>
#include <Plasma/Theme>

//Own
#include "devicespaceinfodelegate.h"
#include "notifierdialog.h"

using namespace Notifier;

NotifierView::NotifierView(QWidget *parent)
    : QTreeView(parent),
      m_addShowAllAction(false)
{
    setIconSize(QSize(KIconLoader::SizeMedium, KIconLoader::SizeMedium));
    setRootIsDecorated(true);
    setHeaderHidden(true);
    setMouseTracking(true);
    viewport()->setAutoFillBackground(true);
    QPalette p(palette());
    p.setColor(QPalette::Base, Qt::transparent);
    viewport()->setAttribute(Qt::WA_NoSystemBackground, true);
    setPalette(p);
    setFrameShape(QFrame::NoFrame);
    m_ignoreLeaveEvent = false;

    m_hideItem = new QAction(this);
    m_showAll = new QAction(i18n("Show all the items"), this);
    m_showAll->setCheckable(true);
    m_hideItem->setCheckable(true);
    connect(m_hideItem, SIGNAL(triggered()), this, SLOT(setItemVisibility()));
    connect(m_showAll, SIGNAL(toggled(bool)), this, SIGNAL(allItemsVisibilityChanged(bool)));
}

NotifierView::~NotifierView()
{

}

QModelIndex NotifierView::indexAt(const QPoint& point) const
{
    // simple linear search through the item rects, this will
    // be inefficient when the viewport is large
    QHashIterator<QModelIndex,QRect> iter(itemRects);
    while (iter.hasNext()) {
        iter.next();
        if (iter.value().contains(point + QPoint(0, verticalOffset()))) {
            return iter.key();
        }
    }
    return QModelIndex();
}

void NotifierView::resizeEvent(QResizeEvent * event)
{
    //the columns after the first are squares KIconLoader::SizeMedium x KIconLoader::SizeMedium,
    //the first column takes all the remaining space
    calculateRects();

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
        update();
        m_hoveredIndex = itemUnderMouse;
        setCurrentIndex(m_hoveredIndex);
    } else if (!itemUnderMouse.isValid()) {
        m_hoveredIndex = QModelIndex();
        setCurrentIndex(m_hoveredIndex);
    }

    m_ignoreLeaveEvent = false;

    QAbstractItemView::mouseMoveEvent(event);
}

void NotifierView::mousePressEvent(QMouseEvent *event)
{
    const QModelIndex itemUnderMouse = indexAt(event->pos());
    //don't pass click for header
    if (event->button() != Qt::LeftButton || itemUnderMouse.data(NotifierDialog::IsCategoryRole).toBool() ) {
        return;
    }

    QAbstractItemView::mousePressEvent(event);
}

void NotifierView::mouseReleaseEvent(QMouseEvent *event)
{
    const QModelIndex itemUnderMouse = indexAt(event->pos());
    //don't pass click for header
    if (event->button() != Qt::LeftButton || itemUnderMouse.data(NotifierDialog::IsCategoryRole).toBool()) {
        return;
    }

    QAbstractItemView::mouseReleaseEvent(event);
}

void NotifierView::leaveEvent(QEvent *event)
{
    Q_UNUSED(event)
    if (m_ignoreLeaveEvent) {
        return;
    }

    if (m_hoveredIndex.isValid()) {
        const QModelIndex oldHoveredIndex = m_hoveredIndex;
        m_hoveredIndex = QModelIndex();
        setCurrentIndex(m_hoveredIndex);
        update(oldHoveredIndex);
    }
}

QModelIndex NotifierView::moveCursor(CursorAction cursorAction,Qt::KeyboardModifiers modifiers )
{
    m_hoveredIndex = QModelIndex();

    return QTreeView::moveCursor(cursorAction, modifiers );
}

void NotifierView::calculateRects()
{
    if (!model()) {
        return;
    }

    itemRects.clear();
    int verticalOffset = TOP_OFFSET;

    const int rows = model()->rowCount(rootIndex());
    const int cols = header()->count();
    //kDebug() << "painting" << rows << "rows" << cols << "columns";

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            const QModelIndex index = model()->index(i, j, rootIndex());
            if (model()->hasChildren(index)) {
                QRect itemRect(QPoint(HEADER_LEFT_MARGIN, verticalOffset),
                               QSize(width() - HEADER_LEFT_MARGIN, HEADER_HEIGHT));
                verticalOffset += itemRect.size().height();
                itemRects.insert(index, itemRect);

                QStandardItemModel * currentModel = dynamic_cast<QStandardItemModel *>(model());
                QStandardItem *currentItem = currentModel->itemFromIndex(index);
                // we display the children of this item (the devices)
                for (int k = 0; k < currentItem->rowCount(); ++k) {
                    for (int l = currentItem->columnCount() - 1; l > -1; --l) {
                        QStandardItem *childItem = currentItem->child(k, l);

                        if (!childItem) {
                            continue;
                        }

                        QModelIndex childIndex = childItem->index();
                        QRect itemChildRect;
                        if (l % 2 == 0) {
                            QSize size(width() - COLUMN_EJECT_SIZE, sizeHintForIndex(index).height());
                            itemChildRect = QRect(QPoint(HEADER_LEFT_MARGIN, verticalOffset), size);
                            itemRects.insert(childIndex, itemChildRect);
                            verticalOffset += itemChildRect.size().height();
                        } else {
                            QSize size(COLUMN_EJECT_SIZE - style()->pixelMetric(QStyle::PM_ScrollBarExtent) + 2,
                                    sizeHintForIndex(index).height());
                            itemChildRect = QRect(QPoint(width() - (COLUMN_EJECT_SIZE - COLUMN_EJECT_MARGIN ),
                                                verticalOffset), size);
                            itemRects.insert(childIndex, itemChildRect);
                        }

                        if (childItem->hasChildren()) { //actions
                            for (int n = 0; n < childItem->rowCount(); ++n) {
                                for (int m = 0; m < childItem->columnCount(); ++m) {
                                    QStandardItem *subChildItem = childItem->child(n, m);
                                    if (subChildItem) {
                                        QModelIndex subChildIndex = subChildItem->index();
                                        QRect subItemChildRect;

                                        QSize size;
                                        size = QSize(width() - COLUMN_EJECT_SIZE, sizeHintForIndex(index).height());
                                        subItemChildRect = QRect(QPoint(ACTION_LEFT_MARGIN, verticalOffset), size);
                                        itemRects.insert(subChildIndex, subItemChildRect);
                                        verticalOffset += size.height();
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void NotifierView::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    if (!model()) {
        return;
    }

    QPainter painter(viewport());
    painter.setRenderHint(QPainter::Antialiasing);

    QHashIterator<QModelIndex, QRect> it(itemRects);
    while (it.hasNext()) {
        it.next();
        QRect itemRect = it.value();
        QRect rect(itemRect.x(), itemRect.y() - verticalOffset(), itemRect.width(), itemRect.height()); 
        if (event->region().contains(rect)) {
            QModelIndex index = it.key();
            if (model()->data(index, NotifierDialog::IsCategoryRole).toBool()) {
                //kDebug()<<"header"<<rect;
                paintHeaderItem(painter, rect, index);
            } else {
                paintItem(painter, rect, index);
            }
        }
    }
}

QRect NotifierView::visualRect(const QModelIndex &index) const
{
    return itemRects[index];
}

QSize NotifierView::sizeHint()
{
    return QSize(150, 300);
}

void NotifierView::paintHeaderItem(QPainter &painter, const QRect &itemRect, const QModelIndex &index)
{
    QStyleOptionViewItem option = viewOptions();
    option.rect = itemRect;
    const int rightMargin = style()->pixelMetric(QStyle::PM_ScrollBarExtent) + 6;
    const int dy = HEADER_TOP_MARGIN;

    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, false);

    QLinearGradient gradient(option.rect.topLeft(), option.rect.topRight());
    gradient.setColorAt(0.0, Qt::transparent);
    gradient.setColorAt(0.5, Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor));
    gradient.setColorAt(1.0, Qt::transparent);
    painter.setPen(QPen(gradient, 1));

    painter.drawLine(option.rect.x() + 6, option.rect.y() + dy + 2,
                     option.rect.right() - rightMargin , option.rect.y() + dy + 2);

    gradient.setColorAt(0.5, Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor));
    painter.setPen(QPen(gradient, 1));
    painter.drawLine(option.rect.x() + 6, option.rect.y() + dy + 3,
                     option.rect.right() - rightMargin , option.rect.y() + dy + 3);

    painter.setFont(KGlobalSettings::smallestReadableFont());
    QColor textColor = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
    textColor.setAlphaF(0.6);
    painter.setPen(textColor);
    QString text = index.data(Qt::DisplayRole).value<QString>();
    painter.drawText(option.rect.adjusted(0, dy, -rightMargin, 0),
                    Qt::AlignVCenter|Qt::AlignRight, text);
    painter.restore();
}

void NotifierView::paintItem(QPainter &painter, const QRect &itemRect, const QModelIndex &index)
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

void NotifierView::contextMenuEvent(QContextMenuEvent *event)
{
    m_ignoreLeaveEvent = true;

    KMenu menu(this);

    bool drawMenu = false;

    const QModelIndex itemUnderMouse = indexAt(event->pos());
    if (itemUnderMouse.isValid() && state() == NoState && !itemUnderMouse.data(NotifierDialog::IsCategoryRole).toBool()) {
        m_hoveredIndex = itemUnderMouse;
        setCurrentIndex(m_hoveredIndex);

        QString name = itemUnderMouse.data(Qt::DisplayRole).value<QString>();
        QString udi = itemUnderMouse.data(NotifierDialog::SolidUdiRole).value<QString>();
        m_hideItem->setChecked(!itemUnderMouse.data(NotifierDialog::VisibilityRole).toBool());
        if (udi == QString()) {
            QModelIndex device = itemUnderMouse.parent();
            name = device.data(Qt::DisplayRole).value<QString>();
            udi = device.data(NotifierDialog::SolidUdiRole).value<QString>();
            m_hideItem->setChecked(!device.data(NotifierDialog::VisibilityRole).toBool());
        } else if (name == QString()) {
            QModelIndex device = itemUnderMouse.sibling(itemUnderMouse.row(), 0);
            name = device.data(Qt::DisplayRole).value<QString>();
            m_hideItem->setChecked(!device.data(NotifierDialog::VisibilityRole).toBool());
        }
        m_hideItem->setText(i18n("Hide ") + name);
        m_hideItem->setData(udi);
        menu.addAction(m_hideItem);
        drawMenu = true;
    } else if (!itemUnderMouse.isValid() || itemUnderMouse.data(NotifierDialog::IsCategoryRole).toBool()) {
        m_hoveredIndex = QModelIndex();
        setCurrentIndex(m_hoveredIndex);
    }

    if (m_addShowAllAction) {
        menu.addAction(m_showAll);
        drawMenu = true;
    }

    if (drawMenu) {
        update();

        menu.exec(event->globalPos());
    }
}

void NotifierView::setItemVisibility()
{
    QString udi = m_hideItem->data().toString();
    bool checked = m_hideItem->isChecked();

    emit itemVisibilityChanged(udi, !checked);
}

void NotifierView::addShowAllAction(bool value)
{
    m_addShowAllAction = value;
}

#include "notifierview.moc"
