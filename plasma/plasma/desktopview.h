/*
 *   Copyright (C) 2007 Aaron Seigo <aseigo@kde.org>
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

#ifndef DESKTOPVIEW_H
#define DESKTOPVIEW_H

#include <QGraphicsView>

#include "plasma/plasma.h"

class QPixmap;

namespace Plasma
{
    class Svg;
    class Corona;
}

class DesktopView : public QGraphicsView
{
    Q_OBJECT

public:
    DesktopView(QWidget *parent);
    ~DesktopView();

    Plasma::Corona* corona();

public slots:
    void zoomIn();
    void zoomOut();
    void launchExplorer();
    void runCommand();

protected:
    void drawBackground(QPainter *painter, const QRectF &);
    void resizeEvent(QResizeEvent *event);
    void wheelEvent(QWheelEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);

private:
    Plasma::Svg *m_background;
    QPixmap* m_bitmapBackground;
    QString m_wallpaperPath;
    QAction *m_engineExplorerAction;
    QAction *m_runCommandAction;
    QAction *m_zoomInAction;
    QAction *m_zoomOutAction;
    Plasma::ZoomLevel m_zoomLevel;
};

#endif // multiple inclusion guard
