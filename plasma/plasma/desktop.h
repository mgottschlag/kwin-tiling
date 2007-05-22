/*
 *   Copyright (C) 2007 Matt Broadstone <mbroadst@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
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

#ifndef DESKTOP_H
#define DESKTOP_H

#include <QGraphicsView>

class QGraphicsScene;
namespace Plasma
{
    class Svg;
}

/**
 * @short The view that displays the all the desktop
 */
class Desktop : public QGraphicsView
{
    Q_OBJECT
public:
    Desktop(QWidget *parent = 0);
    ~Desktop();

protected:
    void resizeEvent(QResizeEvent* event);
    void drawBackground(QPainter * painter, const QRectF & rect);
    QAction *engineExplorer;
    QAction *exitPlasma;

protected Q_SLOTS:
    void displayContextMenu(const QPoint& point);
    void launchExplorer(bool /*param*/);

private:
    QGraphicsScene *m_graphicsScene;
    Plasma::Svg* m_background;
};

#endif


