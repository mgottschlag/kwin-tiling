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

#include <Plasma/Plasma>

#include <QWeakPointer>

class QGraphicsGridLayout;

namespace Plasma
{
    class IconWidget;
    class ItemBackground;
}

class ItemContainer : public QGraphicsWidget
{
    Q_OBJECT

public:
    ItemContainer(QGraphicsWidget *parent);
    ~ItemContainer();

    void setCurrentItem(Plasma::IconWidget *currentItem);
    Plasma::IconWidget *currentItem() const;

    void insertItem(Plasma::IconWidget *item, qreal weight);
    void clear();
    int count() const;

    void setOrientation(Qt::Orientation orientation);
    Qt::Orientation orientation() const;

    void setIconSize(int size);
    int iconSize() const;

    QList<Plasma::IconWidget *>items() const;

    Plasma::IconWidget *createItem();

    void askRelayout();

protected:
    void focusInEvent(QFocusEvent *event);
    void focusOutEvent(QFocusEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    void resizeEvent(QGraphicsSceneResizeEvent *event);

private Q_SLOTS:
    void relayout();
    void syncCurrentItem();
    void itemRemoved(QObject *object);

Q_SIGNALS:
    void itemSelected(Plasma::IconWidget *);
    void itemActivated(Plasma::IconWidget *);
    void resetRequested();

private:
    QGraphicsGridLayout *m_layout;
    QWeakPointer<Plasma::IconWidget> m_currentIcon;
    Plasma::ItemBackground *m_hoverIndicator;
    QTimer *m_relayoutTimer;
    QTimer *m_setCurrentTimer;
    QMultiMap<qreal, Plasma::IconWidget*> m_items;
    QList<Plasma::IconWidget*> m_usedItems;
    Qt::Orientation m_orientation;
    int m_currentIconIndexX;
    int m_currentIconIndexY;
    int m_iconSize;
    int m_maxColumnWidth;
    int m_maxRowHeight;
};

#endif
