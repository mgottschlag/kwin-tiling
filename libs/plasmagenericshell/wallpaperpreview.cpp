/*
 * Copyright 2008  Petri Damsten <damu@iki.fi>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "wallpaperpreview.h"
#include <QPainter>
#include <QPaintEvent>
#include <Plasma/Wallpaper>
#include <Plasma/Svg>

WallpaperPreview::WallpaperPreview(QWidget *parent) : QWidget(parent), m_wallpaper(0)
{
    m_wallpaperOverlay = new Plasma::Svg(this);
    m_wallpaperOverlay->setImagePath("widgets/monitor");
    m_wallpaperOverlay->setContainsMultipleImages(true);
}

WallpaperPreview::~WallpaperPreview()
{
}

void WallpaperPreview::setWallpaper(Plasma::Wallpaper* wallpaper)
{
    m_wallpaper = wallpaper;
    if (m_wallpaper) {
        connect(m_wallpaper, SIGNAL(update(QRectF)),
                this, SLOT(updateRect(QRectF)));
        resizeEvent(0);
    }
}

void WallpaperPreview::resizeEvent(QResizeEvent* event)
{
    Q_UNUSED(event)
    if (m_wallpaper) {
        m_wallpaper->setBoundingRect(contentsRect());
    }
}

void WallpaperPreview::updateRect(const QRectF& rect)
{
    update(rect.toRect());
}

void WallpaperPreview::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    if (m_wallpaper) {
        m_wallpaper->paint(&painter, event->rect());
        m_wallpaperOverlay->paint(&painter, QRect(QPoint(0,0), size()), "glass");
    }
}

#include "wallpaperpreview.moc"
