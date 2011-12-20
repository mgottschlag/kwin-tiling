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
#include "ui/flipscrollview.h"

// Qt
#include <QMouseEvent>
#include <QPainter>
#include <QScrollBar>
#include <QStack>
#include <QTimeLine>

// KDE
#include <KDebug>
#include <KGlobalSettings>
#include <KIconLoader>
#include <KColorScheme>

#include "ui/itemdelegate.h"

using namespace Kickoff;

class FlipScrollView::Private
{
public:
    Private(FlipScrollView *view)
            : q(view)
            , flipAnimTimeLine(new QTimeLine())
            , animLeftToRight(true)
            , itemHeight(-1) {
    }

    ~Private()
    {
        delete flipAnimTimeLine;
    }

    QModelIndex currentRoot() const
    {
        if (currentRootIndex.isValid()) {
            return currentRootIndex;
        } else {
            return q->rootIndex();
        }
    }

    QModelIndex previousRoot() const
    {
        if (previousRootIndices.isEmpty()) {
            return QModelIndex();
        }
        return previousRootIndices.top();
    }

    void setCurrentRoot(const QModelIndex& index)
    {
        if (previousRootIndices.isEmpty() || previousRootIndices.top() != index) {
            // we're entering into a submenu
            //kDebug() << "pushing" << currentRootIndex.data(Qt::DisplayRole).value<QString>();
            animLeftToRight = true;
            hoveredIndex = QModelIndex();
            previousRootIndices.push(currentRootIndex);
            currentRootIndex = index;
            previousVerticalOffsets.append(q->verticalOffset());
            updateScrollBarRange();
            q->verticalScrollBar()->setValue(0);
        } else {
            // we're exiting to the parent menu
            //kDebug() << "popping" << previousRootIndices.top().data(Qt::DisplayRole).value<QString>();
            animLeftToRight = false;
            hoveredIndex = currentRootIndex;
            previousRootIndices.pop();
            //if (!previousRootIndices.isEmpty()) {
            //    kDebug() << "now the previos root is" << previousRootIndices.top().data(Qt::DisplayRole).value<QString>();
            //}
            currentRootIndex = index;
            updateScrollBarRange();
            q->verticalScrollBar()->setValue(previousVerticalOffsets.pop());
        }

        emit q->currentRootChanged(index);

        if (q->viewOptions().direction == Qt::RightToLeft) {
            animLeftToRight = !animLeftToRight;
        }

        flipAnimTimeLine->setCurrentTime(0);
        q->update();
    }

    int previousVerticalOffset()
    {
        return previousVerticalOffsets.isEmpty() ? 0 : previousVerticalOffsets.top();
    }

    int treeDepth(const QModelIndex& headerIndex) const
    {
        int depth = 0;
        QModelIndex index = headerIndex;
        while (index.isValid()) {
            index = index.parent();
            depth++;
        }
        return depth;
    }

    QPainterPath trianglePath(qreal width = 5, qreal height = 10) {
        QPainterPath path(QPointF(-width / 2, 0.0));
        path.lineTo(width, -height / 2);
        path.lineTo(width, height / 2);
        path.lineTo(-width / 2, 0.0);

        return path;
    }

    void updateScrollBarRange()
    {
        int childCount = q->model()->rowCount(currentRootIndex);
        int pageSize = q->height();
        int itemH = q->sizeHintForIndex(q->model()->index(0, 0)).height();
        q->verticalScrollBar()->setRange(0, (childCount * itemH) - pageSize);
        q->verticalScrollBar()->setPageStep(pageSize);
        q->verticalScrollBar()->setSingleStep(itemH);
    }

    FlipScrollView * const q;
    QPersistentModelIndex hoveredIndex;
    QPersistentModelIndex watchedIndexForDrag;

    QTimeLine *flipAnimTimeLine;
    bool animLeftToRight;

    int itemHeight;
    static const int FLIP_ANIM_DURATION = 200;

private:
    QPersistentModelIndex currentRootIndex;
    QStack<QPersistentModelIndex> previousRootIndices;
    QStack<int> previousVerticalOffsets;
};

FlipScrollView::FlipScrollView(QWidget *parent)
        : QAbstractItemView(parent)
        , d(new Private(this))
{
    connect(this, SIGNAL(clicked(QModelIndex)), this, SLOT(openItem(QModelIndex)));
    connect(d->flipAnimTimeLine, SIGNAL(valueChanged(qreal)), this, SLOT(updateFlipAnimation(qreal)));
    d->flipAnimTimeLine->setDuration(Private::FLIP_ANIM_DURATION);
    d->flipAnimTimeLine->setCurrentTime(Private::FLIP_ANIM_DURATION);
    setIconSize(QSize(KIconLoader::SizeMedium, KIconLoader::SizeMedium));
    setMouseTracking(true);
    setAutoScroll(true);
    QPalette viewPalette(palette());
    viewPalette.setColor(QPalette::Window, palette().color(QPalette::Active, QPalette::Base));
    setPalette(viewPalette);
    setAutoFillBackground(true);
}
FlipScrollView::~FlipScrollView()
{
    delete d;
}

void FlipScrollView::setCurrentRoot(const QModelIndex &index)
{
    d->setCurrentRoot(index);
}

void FlipScrollView::viewRoot()
{
    QModelIndex index;
    while (d->currentRoot().isValid()) {
        index = d->currentRoot();
        d->setCurrentRoot(d->currentRoot().parent());
        setCurrentIndex(index);
    }
    update(d->hoveredIndex);
    d->hoveredIndex = index;
}

QModelIndex FlipScrollView::indexAt(const QPoint& point) const
{
    const int items = model()->rowCount(d->currentRoot());
    const int rowIndex = (point.y() + verticalOffset()) / itemHeight();

    if (rowIndex < items) {
        return model()->index(rowIndex, 0, d->currentRoot());
    } else {
        return QModelIndex();
    }
}

int FlipScrollView::itemHeight() const
{
    //TODO: reset on font change
    if (d->itemHeight < 1) {
        QModelIndex index = model()->index(0, 0, d->currentRoot());
        d->itemHeight = sizeHintForIndex(index).height();
    }

    return d->itemHeight;
}

void FlipScrollView::scrollTo(const QModelIndex& index, ScrollHint hint)
{
    if (!index.isValid()) {
        return;
    }

    const QRect itemRect = visualRect(index);
    if (itemRect.isValid() && hint == EnsureVisible) {
        if (itemRect.top() < 0) {
            verticalScrollBar()->setValue(verticalScrollBar()->value() +
                                          itemRect.top());
        } else if (itemRect.bottom() > height()) {
            verticalScrollBar()->setValue(verticalScrollBar()->value() +
                                          (itemRect.bottom() - height()));
        }
        update(itemRect);
    }
}

bool FlipScrollView::isIndexHidden(const QModelIndex&) const
{
    return false;
}

QRect FlipScrollView::visualRect(const QModelIndex& index) const
{
    const int leftOffset = ItemDelegate::ITEM_LEFT_MARGIN;

    if (index.parent() != d->currentRoot() &&
            index.parent() != d->previousRoot() &&
            index.parent() != (QModelIndex)d->hoveredIndex) {
        return QRect();
    }

    const bool parentIsPreviousRoot = d->previousRoot().isValid() && index.parent() == d->previousRoot();
    if (parentIsPreviousRoot && d->flipAnimTimeLine->state() == QTimeLine::NotRunning) {
        return QRect();
    }

    const int scrollBarWidth = verticalScrollBar()->isVisible() ? verticalScrollBar()->width() : 0;
    QRect itemRect(leftOffset, index.row() * itemHeight() - verticalOffset() ,
                   width() - leftOffset - scrollBarWidth, itemHeight());

    const qreal timeValue = d->flipAnimTimeLine->currentValue();
    if (index.parent() == d->currentRoot()) {
        if (d->animLeftToRight) {
            itemRect.translate(itemRect.width() * (1 - timeValue), 0);
        } else {
            itemRect.translate(-itemRect.width() * (1 - timeValue), 0);
        }
    } else {
        if (d->animLeftToRight) {
            itemRect.translate((-timeValue*itemRect.width()), 0);
        } else {
            itemRect.translate((timeValue*itemRect.width()), 0);
        }
    }

    return itemRect;
}

int FlipScrollView::horizontalOffset() const
{
    return 0;
}

int FlipScrollView::verticalOffset() const
{
    return verticalScrollBar()->value();
}

QRegion FlipScrollView::visualRegionForSelection(const QItemSelection& selection) const
{
    QRegion region;
    foreach(const QModelIndex& index , selection.indexes()) {
        region |= visualRect(index);
    }
    return region;
}
QModelIndex FlipScrollView::moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers)
{
    QModelIndex index = currentIndex();
    // kDebug() << "Moving cursor with current index" << index.data(Qt::DisplayRole);
    switch (cursorAction) {
    case MoveUp:
        if (!currentIndex().isValid()) {
            index = model()->index(model()->rowCount(d->currentRoot()) - 1, 0, d->currentRoot());
        } else if (currentIndex().row() > 0) {
            index = currentIndex().sibling(currentIndex().row() - 1,
                                           currentIndex().column());
        }
        break;
    case MoveDown:
        if (!currentIndex().isValid()) {
            index = model()->index(0, 0, d->currentRoot());
        } else if (currentIndex().row() <
                   model()->rowCount(currentIndex().parent()) - 1) {
            index = currentIndex().sibling(currentIndex().row() + 1,
                                           currentIndex().column());
        }
        break;
    case MoveLeft:
        if (d->currentRoot().isValid()) {
            index = d->currentRoot();
            d->setCurrentRoot(d->currentRoot().parent());
            setCurrentIndex(index);
        }
        break;
    case MoveRight:
        if (model()->hasChildren(currentIndex())) {
            openItem(currentIndex());
            // return the new current index set by openItem()
            index = currentIndex();
        }
        break;
    default:
        // Do nothing
        break;
    }

    // clear the hovered index
    update(d->hoveredIndex);
    d->hoveredIndex = index;

    //kDebug() << "New index after move" << index.data(Qt::DisplayRole);

    return index;
}

void FlipScrollView::setSelection(const QRect& rect , QItemSelectionModel::SelectionFlags flags)
{
    QItemSelection selection;
    selection.select(indexAt(rect.topLeft()), indexAt(rect.bottomRight()));
    selectionModel()->select(selection, flags);
}

void FlipScrollView::openItem(const QModelIndex& index)
{
    if (model()->canFetchMore(index)) {
        model()->fetchMore(index);
    }

    const bool hasChildren = model()->hasChildren(index);

    if (hasChildren) {
        d->setCurrentRoot(index);
        setCurrentIndex(model()->index(0, 0, index));
    } else {
        //TODO Emit a signal to open/execute the item
    }
}

void FlipScrollView::resizeEvent(QResizeEvent*)
{
    d->updateScrollBarRange();
}

void FlipScrollView::mousePressEvent(QMouseEvent *event)
{
    d->watchedIndexForDrag = indexAt(event->pos());
    QAbstractItemView::mousePressEvent(event);
}

void FlipScrollView::mouseReleaseEvent(QMouseEvent *event)
{
    d->watchedIndexForDrag = QModelIndex();
    QAbstractItemView::mouseReleaseEvent(event);
}

void FlipScrollView::mouseMoveEvent(QMouseEvent *event)
{
    const QModelIndex itemUnderMouse = indexAt(event->pos());
    if (itemUnderMouse != d->hoveredIndex) {
        update(itemUnderMouse);
        update(d->hoveredIndex);

        d->hoveredIndex = itemUnderMouse;
        setCurrentIndex(d->hoveredIndex);
    }

    QAbstractItemView::mouseMoveEvent(event);
}

void FlipScrollView::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Enter ||
            event->key() == Qt::Key_Return) {
        moveCursor(MoveRight, event->modifiers());
        event->accept();
        return;
    }

    if (event->key() == Qt::Key_Escape &&
            d->currentRoot().isValid()) {
        moveCursor(MoveLeft, event->modifiers());
        event->accept();
        return;
    }

    QAbstractItemView::keyPressEvent(event);
}

void FlipScrollView::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
    d->hoveredIndex = QModelIndex();
    setCurrentIndex(QModelIndex());
}

void FlipScrollView::paintItems(QPainter &painter, QPaintEvent *event, QModelIndex &root)
{
    const int rows = model()->rowCount(root);
    //kDebug() << "painting" << rows << "items";

    for (int i = 0; i < rows; ++i) {
        QModelIndex index = model()->index(i, 0, root);

        QStyleOptionViewItem option = viewOptions();
        option.rect = visualRect(index);

        // only draw items intersecting the region of the widget
        // being updated
        if (!event->rect().intersects(option.rect)) {
            continue;
        }

        if (selectionModel()->isSelected(index)) {
            option.state |= QStyle::State_Selected;
        }

        if (index == d->hoveredIndex) {
            option.state |= QStyle::State_MouseOver;
        }

        if (index == currentIndex()) {
            option.state |= QStyle::State_HasFocus;
        }

        itemDelegate(index)->paint(&painter, option, index);

        if (model()->hasChildren(index)) {
            painter.save();
            painter.setPen(Qt::NoPen);
            // there is an assumption made here that the delegate will fill the background
            // with the selected color or some similar color which contrasts well with the
            // highlighted text color
            if (option.state & QStyle::State_MouseOver) {
                painter.setBrush(palette().highlight());
            } else {
                painter.setBrush(palette().text());
            }

            QRect triRect = option.rect;
            QPainterPath tPath = d->trianglePath();
            if (option.direction == Qt::LeftToRight) {
                triRect.setLeft(triRect.right() - ItemDelegate::ITEM_RIGHT_MARGIN);
                painter.translate(triRect.center().x() - 6, triRect.y() + (option.rect.height() / 2));

            } else {
                triRect.setRight(triRect.left() + ItemDelegate::ITEM_RIGHT_MARGIN);
                painter.translate(triRect.center().x() + 6, triRect.y() + (option.rect.height() / 2));
            }


            if (option.direction == Qt::LeftToRight) {
                painter.rotate(180);
            }

            painter.drawPath(tPath);
            painter.resetTransform();
            painter.restore();
        }
    }
}

void FlipScrollView::paintEvent(QPaintEvent * event)
{
    QPalette viewPalette(palette());
    viewPalette.setColor(QPalette::Window, palette().color(QPalette::Active, QPalette::Base));
    setPalette(viewPalette);
    setAutoFillBackground(true);

    QPainter painter(viewport());
    painter.setRenderHint(QPainter::Antialiasing);

    // draw items
    QModelIndex currentRoot = d->currentRoot();
    QModelIndex previousRoot = d->animLeftToRight ? d->previousRoot() : (QModelIndex)d->hoveredIndex;
    //kDebug() << "current root is" << currentRoot.data(Qt::DisplayRole).value<QString>();

    paintItems(painter, event, currentRoot);

    const qreal timerValue = d->flipAnimTimeLine->currentValue();

    if (timerValue < 1.0) {
        //kDebug() << "previous root is" << previousRoot.data(Qt::DisplayRole).value<QString>();
        paintItems(painter, event, previousRoot);

        if (d->flipAnimTimeLine->state() != QTimeLine::Running) {
            d->flipAnimTimeLine->start();
        }
    }

    QRectF eventRect = event->rect();

    // draw navigation
    QStyle::State state = 0;
    if (currentRoot.isValid()) {
        state |= QStyle::State_Enabled;
    }

    if (currentRoot.isValid() || previousRoot.isValid()) {
        qreal opacity = 1.0;
        if (!previousRoot.isValid()) {
            opacity = timerValue;
        } else if (!currentRoot.isValid()) {
            opacity = 1 - timerValue;
        }
    }
}

void FlipScrollView::startDrag(Qt::DropActions supportedActions)
{
    kDebug() << "Starting UrlItemView drag with actions" << supportedActions;

    if (!d->watchedIndexForDrag.isValid()) {
        return;
    }

    QDrag *drag = new QDrag(this);
    QMimeData *mimeData = model()->mimeData(selectionModel()->selectedIndexes());

    if (mimeData->text().isNull()) {
        return;
    }

    drag->setMimeData(mimeData);

    QModelIndex idx = selectionModel()->selectedIndexes().first();
    QIcon icon = idx.data(Qt::DecorationRole).value<QIcon>();
    drag->setPixmap(icon.pixmap(IconSize(KIconLoader::Desktop)));

    drag->exec();
}

void FlipScrollView::updateFlipAnimation(qreal)
{
    setDirtyRegion(rect());
}

#include "flipscrollview.moc"
