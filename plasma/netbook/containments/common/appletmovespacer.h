/*
 *   Copyright 2009 Marco Martin <notmart@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
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

#ifndef APPLETMOVESPACER_H
#define APPLETMOVESPACER_H

#include <QGraphicsWidget>

namespace Plasma {
    class FrameSvg;
}

class AppletMoveSpacer : public QGraphicsWidget
{
    Q_OBJECT

public:
    AppletMoveSpacer(QGraphicsWidget *parent);
    ~AppletMoveSpacer();

protected:
    void resizeEvent(QGraphicsSceneResizeEvent *event);
    void dropEvent(QGraphicsSceneDragDropEvent *event);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

Q_SIGNALS:
    void dropRequested(QGraphicsSceneDragDropEvent *event);

private:
    Plasma::FrameSvg *m_background;
};

#endif
