/***************************************************************************
 *   Copyright 2009 by Alessandro Diaferia <alediaferia@gmail.com>         *
 *   Copyright 2009 by Marco Martin <notmart@gmail.com>                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#ifndef ITEMBACKGROUND_H
#define ITEMBACKGROUND_H

#include <QGraphicsWidget>

namespace Plasma {
    class FrameSvg;
}


class ItemBackground : public QGraphicsWidget
{
    Q_OBJECT
public:
    ItemBackground(QGraphicsWidget *parent = 0);
    ~ItemBackground();

    void animatedShowAtRect(const QRectF &newGeometry);
    void animatedSetVisible(bool visible);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

private Q_SLOTS:
    void animationUpdate(qreal progress);
private:
    Plasma::FrameSvg *m_frameSvg;
    QRectF m_oldGeometry;
    QRectF m_newGeometry;
    int m_animId;
    qreal m_opacity;
    bool m_fading;
    bool m_fadeIn;
};

#endif
