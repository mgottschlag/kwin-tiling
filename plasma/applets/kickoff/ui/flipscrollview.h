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

#ifndef FLIPSCROLLVIEW_H
#define FLIPSCROLLVIEW_H

// Qt
#include <QAbstractItemView>

namespace Kickoff
{

/**
 * An "iPod-style" item view for single-column tree and list data models which 
 * displays items in pages (one per tree branch).
 *
 * Clicking on an item which has children (eg. a folder in a directory model)
 * scrolls the whole contents of the view to show the items children.  A large back
 * arrow is displayed on the left of the display if the current item has a valid parent,
 * when clicked on this scrolls the whole contents of the view to show the parent item's
 * children.
 * 
 * The view assumes that the item delegate will fill the background with the current palette's
 * highlight color when the user hovers over items with the mouse.  Item delegates should check
 * for the QStyle::State_MouseOver or QStyle::State_Selected flags in the state 
 * field of the QStyleOptionViewItem passed to the QAbstractItemDelegate::paint() method.
 */
class FlipScrollView : public QAbstractItemView
{
Q_OBJECT 

public:
    /** Construct a new FlipScrollView with the specified parent. */
    FlipScrollView(QWidget *parent = 0);
    virtual ~FlipScrollView();

    /** Go to the root item. */
    void viewRoot();

    // reimplemented from QAbstractItemView 
    virtual QModelIndex indexAt(const QPoint& point) const;
    virtual void scrollTo(const QModelIndex& index, ScrollHint hint = EnsureVisible);
    virtual QRect visualRect(const QModelIndex& index) const;

    virtual void setModel(QAbstractItemModel *model);

protected:
    // reimplemented from QWidget
    virtual void paintEvent(QPaintEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void resizeEvent(QResizeEvent *event);
    virtual void keyPressEvent(QKeyEvent *event);

    // reimplemented from QAbstractItemView 
    virtual bool isIndexHidden(const QModelIndex& index) const;
    virtual int horizontalOffset() const;
    virtual int verticalOffset() const;
    virtual QRegion visualRegionForSelection(const QItemSelection& selection) const;
    virtual QModelIndex moveCursor(CursorAction cursorAction,Qt::KeyboardModifiers modifiers);
    virtual void setSelection(const QRect& rect , QItemSelectionModel::SelectionFlags flags);
    virtual void startDrag(Qt::DropActions supportedActions);

private Q_SLOTS:
    void openItem(const QModelIndex& index);
    void updateFlipAnimation(qreal value);

private:
    void paintItems(QPainter &painter, QPaintEvent *event, QModelIndex &index);

    class Private;
    Private * const d;
};

}

#endif // FLIPSCROLLVIEW_H
