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

#ifndef GRIDITEMVIEW_H
#define GRIDITEMVIEW_H

#include <Plasma/Frame>
#include <Plasma/Plasma>

class QGraphicsGridLayout;

namespace Plasma
{
    class IconWidget;
}

class GridItemView : public Plasma::Frame
{
    Q_OBJECT

public:
    GridItemView(QGraphicsWidget *parent);
    ~GridItemView();

    void setCurrentItem(Plasma::IconWidget *currentItem);
    Plasma::IconWidget *currentItem() const;

protected:
    void focusInEvent(QFocusEvent *event);
    void focusOutEvent(QFocusEvent *event);
    void keyPressEvent(QKeyEvent *event);

Q_SIGNALS:
    void itemSelected(Plasma::IconWidget *);
    void itemActivated(Plasma::IconWidget *);
    void resetRequested();

private:
    QGraphicsGridLayout *m_layout;
    Plasma::IconWidget *m_currentIcon;
    int m_currentIconIndexX;
    int m_currentIconIndexY;
};

#endif
