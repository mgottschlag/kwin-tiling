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

#ifndef CORONAVIEW_H
#define CORONAVIEW_H

#include <QGraphicsView>

class QPixmap;

namespace Plasma
{
    class Svg;
    class Corona;
}

class CoronaView : public QGraphicsView
{
    Q_OBJECT

public:
    CoronaView(QWidget *parent);
    ~CoronaView();

    Plasma::Corona* corona();
    
public slots:
    void zoomIn();
    void zoomOut();

protected:
    void drawBackground(QPainter *painter, const QRectF &);
    void resizeEvent(QResizeEvent *event);
    void wheelEvent(QWheelEvent *event);

private:
    Plasma::Svg *m_background;
    QPixmap* m_bitmapBackground;
    QString m_wallpaperPath;
};

#endif // multiple inclusion guard
