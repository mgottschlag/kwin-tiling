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

#ifndef ITEMCONTAINER_H
#define ITEMCONTAINER_H

#include <QGraphicsWidget>
#include <QModelIndex>

#include <Plasma/Plasma>

#include <QWeakPointer>
#include <QModelIndex>

class QGraphicsGridLayout;
class QPropertyAnimation;
class QAbstractItemModel;

namespace Plasma
{
    class IconWidget;
    class ItemBackground;
}

class ItemView;
class IconActionCollection;

class ItemContainer : public QGraphicsWidget
{
    Q_OBJECT

public:
    enum DragAndDropMode{
        NoDragAndDrop = 0,
        CopyDragAndDrop = 1,
        MoveDragAndDrop = 2
    };

    ItemContainer(ItemView *parent);
    ~ItemContainer();

    void setCurrentItem(Plasma::IconWidget *currentItem);
    Plasma::IconWidget *currentItem() const;

    void setOrientation(Qt::Orientation orientation);
    Qt::Orientation orientation() const;

    void setIconSize(int size);
    int iconSize() const;

    void setDragAndDropMode(DragAndDropMode mode);
    DragAndDropMode dragAndDropMode() const;

    QList<Plasma::IconWidget *>items() const;

    Plasma::IconWidget *createItem();

    void askRelayout();

    void setModel(QAbstractItemModel *model);
    QAbstractItemModel *model() const;
    void setRootIndex(QModelIndex index);
    QModelIndex rootIndex() const;
    QModelIndex indexForPosition(const QPointF &point);
    int rowForPosition(const QPointF &point);

protected:
    void focusInEvent(QFocusEvent *event);
    void focusOutEvent(QFocusEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    void resizeEvent(QGraphicsSceneResizeEvent *event);
    bool eventFilter(QObject *watched, QEvent *event);
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    void disposeItem(Plasma::IconWidget *icon);

private Q_SLOTS:
    void relayout();
    void syncCurrentItem();
    void itemRequestedDrag(Plasma::IconWidget *);
    void reset();
    void generateItems(const QModelIndex &parent, int start, int end);
    void removeItems(const QModelIndex &parent, int start, int end);
    void resultClicked();

Q_SIGNALS:
    void itemSelected(Plasma::IconWidget *);
    void itemActivated(const QModelIndex &);
    void resetRequested();
    void itemAskedReorder(const QModelIndex &index, const QPointF &point);
    void dragStartRequested(const QModelIndex &index);
    void dragMoveMouseMoved(const QPointF &);

private:
    QGraphicsGridLayout *m_layout;
    QWeakPointer<Plasma::IconWidget> m_currentIcon;
    Plasma::IconWidget *m_ghostIcon;
    Plasma::ItemBackground *m_hoverIndicator;
    QTimer *m_relayoutTimer;
    QTimer *m_setCurrentTimer;
    QHash<QPersistentModelIndex, Plasma::IconWidget*> m_items;
    QHash<Plasma::IconWidget*, QPersistentModelIndex> m_itemToIndex;
    QList<Plasma::IconWidget*> m_usedItems;
    Qt::Orientation m_orientation;
    QPropertyAnimation *m_positionAnimation;
    int m_currentIconIndexX;
    int m_currentIconIndexY;
    int m_iconSize;
    int m_maxColumnWidth;
    int m_maxRowHeight;
    bool m_firstRelayout;
    DragAndDropMode m_dragAndDropMode;
    bool m_dragging;
    QAbstractItemModel *m_model;
    QModelIndex m_rootIndex;
    ItemView *m_itemView;
    IconActionCollection *m_iconActionCollection;
};

#endif
