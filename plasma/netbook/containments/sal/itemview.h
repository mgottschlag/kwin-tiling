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

#ifndef ITEMVIEW_H
#define ITEMVIEW_H

#include <QGraphicsWidget>
#include <QModelIndex>

#include <Plasma/ScrollWidget>

#include "itemcontainer.h"


namespace Plasma
{
    class IconWidget;
}

class QTimer;

class ItemView : public Plasma::ScrollWidget
{
    Q_OBJECT

public:
    enum ScrollBarNeeded {
        NoScrollBar = 0,
        HorizontalScrollBar = 1,
        VerticalScrollBar = 2,
        AllScrollBars = HorizontalScrollBar|VerticalScrollBar
    };
    Q_DECLARE_FLAGS(ScrollBarFlags, ScrollBarNeeded)

    ItemView(QGraphicsWidget *parent);
    ~ItemView();

    void setCurrentItem(ResultWidget *currentItem);
    ResultWidget *currentItem() const;

    void setOrientation(Qt::Orientation orientation);
    Qt::Orientation orientation() const;

    void showSpacer(const QPointF &pos);

    void setIconSize(int size);
    int iconSize() const;

    QList<ResultWidget *>items() const;

    void setDragAndDropMode(ItemContainer::DragAndDropMode mode);
    ItemContainer::DragAndDropMode dragAndDropMode() const;

    qreal positionToWeight(const QPointF &point);
    void setModel(QAbstractItemModel *model);
    QAbstractItemModel *model() const;
    void setRootIndex(QModelIndex index);
    QModelIndex rootIndex() const;
    int rowForPosition(const QPointF &point);

public Q_SLOTS:
    void setScrollPositionFromDragPosition(const QPointF &point);

protected:
    void resizeEvent(QGraphicsSceneResizeEvent *event);
    bool eventFilter(QObject *watched, QEvent *event);
    void focusInEvent(QFocusEvent *event);

protected Q_SLOTS:
    void selectItem(ResultWidget *icon);

Q_SIGNALS:
    void itemSelected(ResultWidget *);
    void itemActivated(const QModelIndex &index);
    void resetRequested();
    void scrollBarsNeededChanged(ItemView::ScrollBarFlags);
    void itemAskedReorder(const QModelIndex &index, const QPointF &point);
    void dragStartRequested(const QModelIndex &index);
    void addActionTriggered(const QModelIndex &index);

private:
    ItemContainer *m_itemContainer;
    QTimer *m_noActivateTimer;
};

#endif
