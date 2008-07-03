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

namespace Plasma
{
    class Applet;
}

class PanelAppletOverlay : public QWidget
{
    Q_OBJECT

public:
    PanelAppletOverlay(Plasma::Applet *applet, QWidget *parent);

protected:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

protected slots:
    void syncGeometry();

private:
    void swapWithPrevious();
    void swapWithNext();

    Plasma::Applet *m_applet;
    Qt::Orientation m_orientation;
    QGraphicsLinearLayout *m_layout;
    QRectF m_prevGeom;
    QRectF m_nextGeom;
    int m_index;
};

#endif

