/*
 *   Copyright 2008 Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2,
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

#ifndef APPLETMOVEOVERLAY_H
#define APPLETMOVEOVERLAY_H

#include <QWidget>

class QGraphicsLinearLayout;
class QGraphicsWidget;

namespace Plasma
{
    class Applet;
    class Dialog;
}

class PanelAppletHandle;

class PanelAppletOverlay : public QWidget
{
    Q_OBJECT

public:
    enum DragType {
       Move = 1,
       LeftResize = 2,
       RightResize = 3
    };

    PanelAppletOverlay(Plasma::Applet *applet, QWidget *parent);
    ~PanelAppletOverlay();

    void syncOrientation();
    void syncIndex();
    Plasma::Applet *applet() const;

signals:
    void removedWithApplet(PanelAppletOverlay*);
    void moved(PanelAppletOverlay*);

protected:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);
    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);

protected slots:
    void appletDestroyed();
    void delaySyncGeometry();
    void syncGeometry();

    void handleMousePressed(Plasma::Applet *applet, QMouseEvent *event);
    void handleMouseMoved(Plasma::Applet *applet, QMouseEvent *event);
    void handleMouseReleased(Plasma::Applet *applet, QMouseEvent *event);

private:
    void swapWithPrevious();
    void swapWithNext();

    Plasma::Applet *m_applet;
    QGraphicsWidget *m_spacer;
    Qt::Orientation m_orientation;
    QGraphicsLinearLayout *m_layout;
    QRectF m_prevGeom;
    QRectF m_nextGeom;
    QPoint m_origin;
    QPoint m_lastGlobalPos;
    DragType m_dragAction;
    static PanelAppletHandle *s_appletHandle;
    static int s_appletHandleCount;
    int m_offset;
    int m_index;
    bool m_clickDrag;
};

#endif

