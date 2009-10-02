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

#include <Plasma/ScrollWidget>

class QGraphicsGridLayout;

namespace Plasma
{
    class IconWidget;
    class ItemBackground;
}

class ItemContainer;

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

    void setCurrentItem(Plasma::IconWidget *currentItem);
    Plasma::IconWidget *currentItem() const;

    void insertItem(Plasma::IconWidget *item, qreal weight);
    void clear();
    int count() const;

    void setOrientation(Qt::Orientation orientation);
    Qt::Orientation orientation() const;

    void setIconSize(int size);
    int iconSize() const;

protected:
    void resizeEvent(QGraphicsSceneResizeEvent *event);
    bool eventFilter(QObject *watched, QEvent *event);

protected Q_SLOTS:
    void selectItem(Plasma::IconWidget *icon);

Q_SIGNALS:
    void itemSelected(Plasma::IconWidget *);
    void itemActivated(Plasma::IconWidget *);
    void resetRequested();
    void scrollBarsNeededChanged(ItemView::ScrollBarFlags);

private:
    ItemContainer *m_itemContainer;
};

#endif
