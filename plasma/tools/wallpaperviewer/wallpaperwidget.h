/*
 *   Copyright 2009 Aaron Seigo <aseigo@kde.org>
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

#ifndef WALLPAPERWIDGET_H
#define WALLPAPERWIDGET_H

#include <QWidget>

namespace Plasma
{
    class Wallpaper;
} // namespace Plasma

class WallpaperWidget : public QWidget
{
    Q_OBJECT

public:
    WallpaperWidget(const QString &paper, const QString &mode, QWidget *parent = 0);
    ~WallpaperWidget();

protected:
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);

protected Q_SLOTS:
    void updatePaper(const QRectF &exposedRect);

private:
    Plasma::Wallpaper *m_wallpaper;
};

#endif

