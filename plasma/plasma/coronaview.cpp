/*
 *   Copyright (C) 2007 Aaron Seigo <aseigo@kde.org>
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

#include <QFile>
#include <QWheelEvent>

#include "plasma/svg.h"
#include "plasma/corona.h"

#include "coronaview.h"

CoronaView::CoronaView(QWidget *parent)
    : QGraphicsView(parent),
      m_background(0),
      m_bitmapBackground(0)
{
    setFrameShape(QFrame::NoFrame);
    setAutoFillBackground(true);

    setScene(new Plasma::Corona(rect(), this));
    scene()->setItemIndexMethod(QGraphicsScene::NoIndex);
    //TODO: Figure out a way to use rubberband and ScrollHandDrag
    setDragMode(QGraphicsView::RubberBandDrag);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse); // Why isn't this working???
    setCacheMode(QGraphicsView::CacheBackground);
    setInteractive(true);
    setAcceptDrops(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    //TODO: make this a real background renderer
    KConfigGroup config(KGlobal::config(), "General");
    m_wallpaperPath = config.readEntry("wallpaper", QString());

    //kDebug() << "wallpaperPath is " << m_wallpaperPath << " " << QFile::exists(m_wallpaperPath) << endl;
    if (m_wallpaperPath.isEmpty() ||
        !QFile::exists(m_wallpaperPath)) {
        m_background = new Plasma::Svg("widgets/wallpaper", this);
    }
}

CoronaView::~CoronaView()
{
}

void CoronaView::zoomIn()
{
    //TODO: Change level of detail when zooming
    // 10/8 == 1.25
    scale(1.25, 1.25);
}

void CoronaView::zoomOut()
{
    // 8/10 == .8
    scale(.8, .8);
}

Plasma::Corona* CoronaView::corona()
{
    return static_cast<Plasma::Corona*>(scene());
}

void CoronaView::drawBackground(QPainter * painter, const QRectF & rect)
{
    if (m_background) {
        m_background->paint(painter, rect);
    } else if (m_bitmapBackground) {
        painter->drawPixmap(rect, *m_bitmapBackground, rect);
    }
}

void CoronaView::resizeEvent(QResizeEvent* event)
{
    Q_UNUSED(event)
    if (testAttribute(Qt::WA_PendingResizeEvent)) {
        return; // lets not do this more than necessary, shall we?
    }

    scene()->setSceneRect(rect());

    if (m_background) {
        m_background->resize(width(), height());
    } else if (!m_wallpaperPath.isEmpty()) {
        delete m_bitmapBackground;
        m_bitmapBackground = new QPixmap(m_wallpaperPath);
        (*m_bitmapBackground) = m_bitmapBackground->scaled(size());
    }
}

void CoronaView::wheelEvent(QWheelEvent* event)
{
    if (event->modifiers() & Qt::ControlModifier) {
        if (event->delta() < 0) {
            zoomOut();
        } else {
            zoomIn();
        }
    }
}

#include "coronaview.moc"

