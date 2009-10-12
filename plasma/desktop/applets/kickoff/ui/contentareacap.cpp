/*
    Copyright 2008 Andrew Lake <jamboarder@yahoo.com>

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

#include "contentareacap.h"
#include <QPainter>

ContentAreaCap::ContentAreaCap(QWidget *parent, bool flip)
        :QWidget(parent)
{
    setMaximumHeight(3);
    setMinimumHeight(3);
    sizePolicy().setVerticalPolicy(QSizePolicy::Fixed);
    flipCap = flip;

    parent->setCursor(Qt::ArrowCursor);
}

void ContentAreaCap::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    QPainterPath path;
    QRect r = rect();
    if (!flipCap) {
        path.moveTo(r.topLeft() + QPoint(0,3));
        path.quadTo(r.topLeft(), r.topLeft() + QPoint(3,0));
        path.lineTo(r.topRight() + QPoint(-2,0));
        path.quadTo(r.topRight() + QPoint(1,0), r.topRight() + QPoint(1,3));
    } else {
        path.moveTo(r.topLeft());
        path.quadTo(r.topLeft() + QPoint(0,3), r.topLeft() + QPoint(3,3));
        path.lineTo(r.topRight() + QPoint(-2,3));
        path.quadTo(r.topRight() + QPoint(1,3), r.topRight() + QPoint(1,0));
    }   
    painter.setPen(QPen(palette().base(), 1));
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillPath(path, palette().base());
    painter.end();
}
 
