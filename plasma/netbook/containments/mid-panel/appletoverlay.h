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

#ifndef APPLETOVERLAY_H
#define APPLETOVERLAY_H


#include <QGraphicsWidget>

namespace Plasma
{
    class Applet;
};

class Panel;
class AppletMoveSpacer;
class QGraphicsLinearLayout;
class QTimer;

//FIXME: this really screams for mid containments to inherit midContainment...
class AppletOverlay : public QGraphicsWidget
{
    Q_OBJECT

    friend class AppletMoveSpacer;

public:
    explicit AppletOverlay(QGraphicsWidget *parent = 0, Panel *panel = 0);
    ~AppletOverlay();

    void showSpacer(const QPointF &pos);

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget=0);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void dragEnterEvent(QGraphicsSceneDragDropEvent *event);
    void dragMoveEvent(QGraphicsSceneDragDropEvent *event);
    void dropEvent(QGraphicsSceneDragDropEvent *event);

private:
    Plasma::Applet *m_applet;
    Panel *m_panel;
    AppletMoveSpacer *m_spacer;
    int m_spacerIndex;
    bool m_clickDrag;
    QPointF m_origin;
};

#endif
