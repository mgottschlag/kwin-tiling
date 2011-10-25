/*
    Copyright 2007 Robert Knight <robertknight@gmail.com>

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

// Own
#include "ui/urlitemview.h"

// Qt
#include <QtCore/QHash>
#include <QtCore/QPersistentModelIndex>
#include <QtCore/QUrl>

#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtGui/QPaintEvent>
#include <QtGui/QScrollBar>
#include <QtGui/QToolTip>


// KDE
#include <KDebug>
#include <KGlobalSettings>
#include <KIconLoader>
#include <KColorScheme>

// Local
#include "core/models.h"
#include "core/kickoffmodel.h"
#include "ui/itemdelegate.h"

using namespace Kickoff;

class UrlItemView::Private
{
public:
    Private(UrlItemView *parent)
            : q(parent)
            , contentsHeight(0)
            , itemStateProvider(0) {
    }

    void doLayout() {
        // clear existing layout information
        itemRects.clear();
        visualOrder.clear();

        if (!q->model()) {
            return;
        }

        int verticalOffset = ItemDelegate::TOP_OFFSET;
        int horizontalOffset = 0;
        int row = 0;
        int visualColumn = 0;

        QModelIndex branch = currentRootIndex;

        while (true) {
            if (itemChildOffsets[branch] + row >= q->model()->rowCount(branch) ||
                    (branch != currentRootIndex && row > MAX_CHILD_ROWS)) {

                if (branch.isValid()) {
                    row = branch.row() + 1;
                    branch = branch.parent();
                    continue;
                } else {
                    break;
                }
            }

            QModelIndex child = q->model()->index(row + itemChildOffsets[branch], 0, branch);

            if (q->model()->hasChildren(child)) {
                QSize childSize = calculateHeaderSize(child);
                QRect rect(QPoint(ItemDelegate::HEADER_LEFT_MARGIN, verticalOffset), childSize);
                //kDebug() << "header rect for" << child.data(Qt::DisplayRole) << "is" << rect;
                itemRects.insert(child, rect);

                if (childSize.isValid()) {
                    // don't subtract 1
                    verticalOffset += childSize.height();
                }
                horizontalOffset = 0;
                branch = child;
                row = 0;
                visualColumn = 0;
            } else {
                QSize childSize = calculateItemSize(child);
                //kDebug() << "item rect for" << child.data(Qt::DisplayRole) << "is" << QRect(QPoint(horizontalOffset,verticalOffset), childSize);

                itemRects.insert(child, QRect(QPoint(horizontalOffset, verticalOffset),
                                              childSize));

                if (childSize.isValid()) {
                    visualOrder << child;
                }

                horizontalOffset += contentWidth() / MAX_COLUMNS;

                visualColumn++;
                row++;

                bool wasLastRow = row + itemChildOffsets[branch] >= q->model()->rowCount(branch);
                bool nextItemIsBranch = false;
                if (!wasLastRow) {
                    QModelIndex nextIndex = q->model()->index(row + itemChildOffsets[branch], 0, branch);
                    nextItemIsBranch = q->model()->hasChildren(nextIndex);
                }

                if (visualColumn >= MAX_COLUMNS || wasLastRow || nextItemIsBranch) {
                    horizontalOffset = 0;
                    visualColumn = 0;
                }

                if (childSize.isValid()) {
                    // don't subtract 1
                    verticalOffset += childSize.height();
                }
            }
        }
        contentsHeight = verticalOffset;

        updateScrollBarRange();
    }

    void drawHeader(QPainter *painter,
                    const QModelIndex& index,
                    const QStyleOptionViewItem& option) {
        const bool first = isFirstHeader(index);
        const int rightMargin = q->style()->pixelMetric(QStyle::PM_ScrollBarExtent) + 6;
        const int dy = (first ? 4 : ItemDelegate::HEADER_TOP_MARGIN);

        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, false);

        if (!first) {
            QLinearGradient gradient(option.rect.topLeft(), option.rect.topRight());
            gradient.setColorAt(0.0, Qt::transparent);
            gradient.setColorAt(0.1, option.palette.midlight().color());
            gradient.setColorAt(0.5, option.palette.mid().color());
            gradient.setColorAt(0.9, option.palette.midlight().color());
            gradient.setColorAt(1.0, Qt::transparent);
            painter->setPen(QPen(gradient, 1));

            painter->drawLine(option.rect.x() + 6, option.rect.y() + dy + 2,
                              option.rect.right() - rightMargin , option.rect.y() + dy + 2);
        }

        painter->setFont(KGlobalSettings::smallestReadableFont());
        painter->setPen(QPen(KColorScheme(QPalette::Active).foreground(KColorScheme::InactiveText), 0));
        QString text = index.data(Qt::DisplayRole).value<QString>();
        painter->drawText(option.rect.adjusted(0, dy, -rightMargin, 0),
                          Qt::AlignVCenter | Qt::AlignRight, text);
        painter->restore();
    }

    void updateScrollBarRange() {
        const int pageSize = q->height();
        q->verticalScrollBar()->setRange(0, contentsHeight - pageSize);
        q->verticalScrollBar()->setPageStep(pageSize);
        q->verticalScrollBar()->setSingleStep(q->sizeHintForRow(0));
    }

    int contentWidth() const {
        return q->width() - q->style()->pixelMetric(QStyle::PM_ScrollBarExtent) + 2;
    }

    QSize calculateItemSize(const QModelIndex& index) const {
        if (itemStateProvider && !itemStateProvider->isVisible(index)) {
            return QSize();
        } else {
            return  QSize(contentWidth() / MAX_COLUMNS, q->sizeHintForIndex(index).height());
        }
    }

    bool isFirstHeader(const QModelIndex &index) const {
        if (index.row() == 0) {
            return q->model()->hasChildren(index);
        }

        QModelIndex prevHeader = index.sibling(index.row() - 1, index.column());
        while (prevHeader.isValid()) {
            //kDebug() << "checking" << prevHeader.data(Qt::DisplayRole).value<QString>();
            if (q->model()->hasChildren(prevHeader)) {
                //kDebug() << "it has children";
                return false;
            }

            prevHeader = prevHeader.sibling(prevHeader.row() - 1, prevHeader.column());
        }

        return true;
    }

    bool insertAbove(const QRect &itemRect, const QPoint &pos) const {
        return pos.y() < itemRect.top() + (itemRect.height() / 2);
    }

    bool insertBelow(const QRect &itemRect, const QPoint &pos) const {
        return pos.y() >= itemRect.top() + (itemRect.height() / 2);
    }

    QSize calculateHeaderSize(const QModelIndex& index) const {
        const QFontMetrics fm(KGlobalSettings::smallestReadableFont());
        int minHeight = ItemDelegate::HEADER_HEIGHT;
        const bool isFirst = isFirstHeader(index);

        if (itemStateProvider && !itemStateProvider->isVisible(index)) {
            return QSize();
        } else if (isFirst) {
            minHeight = ItemDelegate::FIRST_HEADER_HEIGHT;
        }

        return QSize(q->width() - ItemDelegate::HEADER_LEFT_MARGIN,
                     qMax(fm.height() + (isFirst ? 4 : ItemDelegate::HEADER_TOP_MARGIN), minHeight)
                     + ItemDelegate::HEADER_BOTTOM_MARGIN) ;
    }

    QPoint mapFromViewport(const QPoint& point) const {
        return point + QPoint(0, q->verticalOffset());
    }

    QPoint mapToViewport(const QPoint& point) const {
        return point - QPoint(0, q->verticalOffset());
    }

    UrlItemView * const q;
    QPersistentModelIndex currentRootIndex;
    QPersistentModelIndex hoveredIndex;
    QPersistentModelIndex watchedIndexForDrag;

    QHash<QModelIndex, int> itemChildOffsets;
    QHash<QModelIndex, QRect> itemRects;
    QList<QModelIndex> visualOrder;

    QRect dropRect;
    int draggedRow;
    bool dragging;

    int contentsHeight;
    ItemStateProvider *itemStateProvider;

    static const int MAX_COLUMNS = 1;

    // TODO Eventually it will be possible to restrict each branch to only showing
    // a given number of children, with Next/Previous arrows to view more children
    //
    // eg.
    //
    // Recent Documents  [1-10 of 100]            Next
    // Recent Applications [10-20 of 30] Previous|Next
    //
    static const int MAX_CHILD_ROWS = 1000;
};

UrlItemView::UrlItemView(QWidget *parent)
        : QAbstractItemView(parent)
        , d(new Private(this))
{
    d->dragging = false;
    setIconSize(QSize(KIconLoader::SizeMedium, KIconLoader::SizeMedium));
    setMouseTracking(true);
    QPalette viewPalette(palette());
    viewPalette.setColor(QPalette::Window, palette().color(QPalette::Active, QPalette::Base));
    setPalette(viewPalette);
    setAutoFillBackground(true);
}

UrlItemView::~UrlItemView()
{
    delete d;
}

QModelIndex UrlItemView::indexAt(const QPoint& point) const
{
    // simple linear search through the item rects, this will
    // be inefficient when the viewport is large
    QHashIterator<QModelIndex, QRect> iter(d->itemRects);
    while (iter.hasNext()) {
        iter.next();
        if (iter.value().contains(d->mapFromViewport(point))) {
            return iter.key();
        }
    }
    return QModelIndex();
}

bool UrlItemView::initializeSelection()
{
    if (!selectionModel()
            || selectionModel()->hasSelection()
            || d->itemRects.size() == 0
    ) {
        return false;
    }

    // searching for the first item - the item whose rectangle is the
    // top one

    QHashIterator<QModelIndex, QRect> iter(d->itemRects);

    // There is at least one item
    iter.next();

    int y = iter.value().top();
    QModelIndex index = iter.key();

    while (iter.hasNext()) {
        iter.next();

        if (y > iter.value().top()) {
            y = iter.value().top();
            index = iter.key();
        }
    }

    setCurrentIndex(index);

    return selectionModel()->hasSelection();
}

void UrlItemView::setModel(QAbstractItemModel *model)
{
    QAbstractItemView::setModel(model);

    if (model) {
        connect(model, SIGNAL(rowsRemoved(QModelIndex,int,int)), this, SLOT(updateLayout()));
        connect(model, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(updateLayout()));
        connect(model, SIGNAL(modelReset()), this, SLOT(updateLayout()));
    }

    d->currentRootIndex = QModelIndex();
    d->itemChildOffsets.clear();
    updateLayout();
}

void UrlItemView::updateLayout()
{
    d->doLayout();

    if (viewport()->isVisible()) {
        viewport()->update();
    }
}

void UrlItemView::scrollTo(const QModelIndex& index, ScrollHint hint)
{
    QRect itemRect = d->itemRects[index];
    QRect viewedRect = QRect(d->mapFromViewport(QPoint(0, 0)),
                             size());
    int topDifference = viewedRect.top() - itemRect.top();
    int bottomDifference = viewedRect.bottom() - itemRect.bottom();
    QScrollBar *scrollBar = verticalScrollBar();

    if (!itemRect.isValid())
        return;

    switch (hint) {
    case EnsureVisible: {
        if (!viewedRect.contains(itemRect)) {

            if (topDifference < 0) {
                // scroll view down
                scrollBar->setValue(scrollBar->value() - bottomDifference);
            } else {
                // scroll view up
                scrollBar->setValue(scrollBar->value() - topDifference);
            }
        }
    }
    break;
    case PositionAtTop: {
        //d->viewportOffset = itemRect.top();
    }
    default:
        Q_ASSERT(false); // Not implemented
    }
}

QRect UrlItemView::visualRect(const QModelIndex& index) const
{
    QRect itemRect = d->itemRects[index];
    if (!itemRect.isValid()) {
        return itemRect;
    }

    itemRect.moveTopLeft(d->mapToViewport(itemRect.topLeft()));
    return itemRect;
}

int UrlItemView::horizontalOffset() const
{
    return 0;
}

bool UrlItemView::isIndexHidden(const QModelIndex&) const
{
    return false;
}

QModelIndex UrlItemView::moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers)
{
    QModelIndex index = currentIndex();

    int visualIndex = d->visualOrder.indexOf(index);

    switch (cursorAction) {
    case MoveUp:
        if (!currentIndex().isValid()) {
            const QModelIndex root = model()->index(0, 0);
            index = model()->index(model()->rowCount(root) - 1, 0, root);
        } else {
            visualIndex = qMax(0, visualIndex - 1);
        }
        break;
    case MoveDown:
        if (!currentIndex().isValid()) {
            const QModelIndex root = model()->index(0, 0);
            index = model()->index(0, 0, root);
        } else {
            visualIndex = qMin(d->visualOrder.count() - 1, visualIndex + 1);
        }
        break;
    default:
        // Do nothing
        break;
    }

    d->hoveredIndex = QModelIndex();

    return currentIndex().isValid() ? d->visualOrder.value(visualIndex, QModelIndex())
           : index;
}

void UrlItemView::setSelection(const QRect& rect, QItemSelectionModel::SelectionFlags flags)
{
    QItemSelection selection;
    selection.select(indexAt(rect.topLeft()), indexAt(rect.bottomRight()));
    selectionModel()->select(selection, flags);
}

int UrlItemView::verticalOffset() const
{
    return verticalScrollBar()->value();
}

QRegion UrlItemView::visualRegionForSelection(const QItemSelection& selection) const
{
    QRegion region;
    foreach(const QModelIndex& index, selection.indexes()) {
        region |= visualRect(index);
    }
    return region;
}

void UrlItemView::paintEvent(QPaintEvent *event)
{
    if (!model()) {
        return;
    }

    QPalette viewPalette(palette());
    viewPalette.setColor(QPalette::Window, palette().color(QPalette::Active, QPalette::Base));
    setPalette(viewPalette);
    setAutoFillBackground(true);

    QPainter painter(viewport());
    painter.setRenderHint(QPainter::Antialiasing);

    if (d->dragging && dragDropMode() == QAbstractItemView::DragDrop) {
        const int y = (d->dropRect.top() + d->dropRect.bottom()) / 2;

        painter.save();
        QLinearGradient gr(d->dropRect.left(), y, d->dropRect.right(), y);
        gr.setColorAt(0, palette().base().color());
        gr.setColorAt(.35, palette().windowText().color());
        gr.setColorAt(.65, palette().windowText().color());
        gr.setColorAt(1, palette().base().color());
        painter.setPen(QPen(gr, 1));
        painter.drawLine(d->dropRect.left(), y, d->dropRect.right(), y);
        painter.restore();
    }

    QHashIterator<QModelIndex, QRect> indexIter(d->itemRects);
    while (indexIter.hasNext()) {
        indexIter.next();
        const QRect itemRect = visualRect(indexIter.key());
        const QModelIndex index = indexIter.key();

        if (event->region().contains(itemRect)) {
            QStyleOptionViewItem option = viewOptions();
            option.rect = itemRect;

            if (selectionModel()->isSelected(index)) {
                option.state |= QStyle::State_Selected;
            }
            if (index == d->hoveredIndex) {
                option.state |= QStyle::State_MouseOver;
            }
            if (index == currentIndex()) {
                option.state |= QStyle::State_HasFocus;
            }

            if (model()->hasChildren(index)) {
                d->drawHeader(&painter, index, option);
            } else {
                if (option.rect.left() == 0) {
                    option.rect.setLeft(option.rect.left() + ItemDelegate::ITEM_LEFT_MARGIN);
                    option.rect.setRight(option.rect.right() - ItemDelegate::ITEM_RIGHT_MARGIN);
                }
                itemDelegate(index)->paint(&painter, option, index);
            }
        }
    }
}

void UrlItemView::resizeEvent(QResizeEvent *)
{
    updateLayout();
}


void UrlItemView::mouseMoveEvent(QMouseEvent *event)
{
    const QModelIndex itemUnderMouse = indexAt(event->pos());
    if (itemUnderMouse != d->hoveredIndex && state() == NoState) {
        update(itemUnderMouse);
        update(d->hoveredIndex);

        d->hoveredIndex = itemUnderMouse;
        setCurrentIndex(d->hoveredIndex);
    }

    Plasma::Delegate *hoveredItemDelegate =
                    static_cast<Plasma::Delegate*>(itemDelegate(d->hoveredIndex));
    if (hoveredItemDelegate->showToolTip()) {
        QModelIndex index = d->hoveredIndex;
        QString titleText = index.data(Qt::DisplayRole).toString();
        QString subTitleText = index.data(Plasma::Delegate::SubTitleRole).toString();
        setToolTip(titleText + '\n' + subTitleText);
    } else {
        setToolTip(QString());
    }

    QAbstractItemView::mouseMoveEvent(event);
}

void UrlItemView::mousePressEvent(QMouseEvent *event)
{
    d->watchedIndexForDrag = indexAt(event->pos());
    QAbstractItemView::mousePressEvent(event);
}

void UrlItemView::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event)

    d->watchedIndexForDrag = QModelIndex();
}

void UrlItemView::setItemStateProvider(ItemStateProvider *provider)
{
    d->itemStateProvider = provider;
}

void UrlItemView::dragEnterEvent(QDragEnterEvent *event)
{
    if (dragDropMode() != QAbstractItemView::DragDrop) {
        return;
    }

    d->dragging = true;
    setDirtyRegion(d->dropRect);

    event->accept();
}

void UrlItemView::dragLeaveEvent(QDragLeaveEvent *event)
{
    if (dragDropMode() != QAbstractItemView::DragDrop) {
        return;
    }

    d->dragging = false;
    setDirtyRegion(d->dropRect);

    event->accept();
}

void UrlItemView::dragMoveEvent(QDragMoveEvent *event)
{
    QAbstractItemView::dragMoveEvent(event);

    const QPoint pos = event->pos();
    const QModelIndex index = indexAt(pos);
    setDirtyRegion(d->dropRect);

    // check to see if it's the header
    if (d->isFirstHeader(index) && index.row() == 0) {
        event->ignore();
        return;
    }

    if (index.isValid()) {
        const QRect rect = visualRect(index);
        const int gap = d->contentsHeight;

        if (d->insertAbove(rect, pos)) {
            d->dropRect = QRect(rect.left(), rect.top() - gap / 2,
                                rect.width(), gap);
        } else if (d->insertBelow(rect, pos)) {
            d->dropRect = QRect(rect.left(), rect.bottom() + 1 - gap / 2,
                                rect.width(), gap);
        } else {
            d->dropRect = rect;
        }
    }

    setDirtyRegion(d->dropRect);
}


void UrlItemView::startDrag(Qt::DropActions supportedActions)
{
    Q_UNUSED(supportedActions)
    //kDebug() << "Starting UrlItemView drag with actions" << supportedActions;

    if (!d->watchedIndexForDrag.isValid()) {
        return;
    }

    QMimeData *mimeData = model()->mimeData(selectionModel()->selectedIndexes());

    if (!mimeData || mimeData->text().isNull()) {
        return;
    }

    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);

    QModelIndex idx = selectionModel()->selectedIndexes().first();
    QIcon icon = idx.data(Qt::DecorationRole).value<QIcon>();
    d->draggedRow = idx.row();
    drag->setPixmap(icon.pixmap(IconSize(KIconLoader::Desktop)));

    d->dropRect = QRect();
    drag->exec(Qt::CopyAction|Qt::MoveAction|Qt::LinkAction);
}

void UrlItemView::dropEvent(QDropEvent *event)
{
    if (!d->dragging) {
        return;
    }

    // This special code is necessary in order to be able to
    // inidcate to the model WHERE the item should be dropped,
    // Since the model cannot tell where the user dropped the
    // item. The model itself handles the actual moving of the
    // item.
    if (dragDropMode() == QAbstractItemView::DragDrop) {
        int row;
        QPoint pos = event->pos();
        QModelIndex parent = indexAt(pos);

        if (!parent.isValid()) {
            return;
        }
        const QRect rect = visualRect(parent);

        row = parent.row();

        if(d->insertBelow(rect, pos) && d->draggedRow > row) {
            row++;
        } else if(d->insertAbove(rect, pos) && d->draggedRow < row) {
            row--;
        }

        model()->dropMimeData(event->mimeData(), event->dropAction(),
							    row, 0, parent);

        d->dragging = false;

        event->accept();
    }
}
void UrlItemView::leaveEvent(QEvent *event)
{
    Q_UNUSED(event)

    //kDebug() << "UrlItemView leave event";

    d->hoveredIndex = QModelIndex();
    setCurrentIndex(QModelIndex());
}

ItemStateProvider *UrlItemView::itemStateProvider() const
{
    return d->itemStateProvider;
}
#include "urlitemview.moc"
