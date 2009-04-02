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

#include "wallpaperwidget.h"

#include <QApplication>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneWheelEvent>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QWheelEvent>

#include <KAction>
#include <KConfigGroup>
#include <KDebug>
#include <KGlobal>
#include <KStandardAction>

#include <Plasma/Wallpaper>

WallpaperWidget::WallpaperWidget(const QString &paper, const QString &mode, QWidget *parent)
    : QWidget(parent),
      m_wallpaper(Plasma::Wallpaper::load(paper))
{
    if (m_wallpaper && !mode.isEmpty()) {
        m_wallpaper->setRenderingMode(mode);
        connect(m_wallpaper, SIGNAL(update(QRectF)), this, SLOT(updatePaper(QRectF)));
    }

    addAction(KStandardAction::quit(qApp, SLOT(quit()), this));
}

WallpaperWidget::~WallpaperWidget()
{
    delete m_wallpaper;
}

void WallpaperWidget::updatePaper(const QRectF &exposedRect)
{
    update(exposedRect.toRect());
}

void WallpaperWidget::paintEvent(QPaintEvent *event)
{
    if (m_wallpaper) {
        if (!m_wallpaper->isInitialized()) {
            // delayed paper initialization
            KConfigGroup wallpaperConfig(KGlobal::config(), "Wallpaper");
            wallpaperConfig = KConfigGroup(&wallpaperConfig, m_wallpaper->pluginName());
            m_wallpaper->restore(wallpaperConfig);
        }

        QPainter p(this);
        m_wallpaper->paint(&p, event->rect());
    }
}

void WallpaperWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)

    if (m_wallpaper) {
        m_wallpaper->setBoundingRect(rect());
    }
}

void WallpaperWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (m_wallpaper && m_wallpaper->isInitialized()) {
        /*
        QGraphicsSceneMouseEvent me(QEvent::MouseButtonPress, event->pos().toPoint(),
                                    event->button(), event->buttons(), event->modifiers());
        m_wallpaper->mouseMoveEvent(&me);

        if (me.isAccepted()) {
            return;
        }
        */
    }

    QWidget::mouseMoveEvent(event);
}

void WallpaperWidget::mousePressEvent(QMouseEvent *event)
{
    if (m_wallpaper && m_wallpaper->isInitialized()) {
        /*
        QGraphicsSceneMouseEvent me(QEvent::MouseButtonPress, event->pos().toPoint(),
                                    event->button(), event->buttons(), event->modifiers());
        m_wallpaper->mousePressEvent(&me);

        if (me.isAccepted()) {
            return;
        }
        */
    }

    QWidget::mousePressEvent(event);
}

void WallpaperWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_wallpaper && m_wallpaper->isInitialized()) {
        /*
        QGraphicsSceneMouseEvent me(QEvent::MouseButtonPress, event->pos().toPoint(),
                                    event->button(), event->buttons(), event->modifiers());
        m_wallpaper->mousePressEvent(&me);
        if (me.isAccepted()) {
            return;
        }
        */
    }

    QWidget::mouseReleaseEvent(event);
}

void WallpaperWidget::wheelEvent(QWheelEvent *event)
{
    if (m_wallpaper && m_wallpaper->isInitialized()) {
        QGraphicsSceneWheelEvent wheelEvent(QEvent::GraphicsSceneWheel);
        wheelEvent.setWidget(this);
        wheelEvent.setScenePos(event->pos());
        wheelEvent.setScreenPos(event->globalPos());
        wheelEvent.setButtons(event->buttons());
        wheelEvent.setModifiers(event->modifiers());
        wheelEvent.setDelta(event->delta());
        wheelEvent.setOrientation(event->orientation());
        wheelEvent.setAccepted(false);

        m_wallpaper->wheelEvent(&wheelEvent);

        if (wheelEvent.isAccepted()) {
            return;
        }
    }

    QWidget::wheelEvent(event);
}

#include "wallpaperwidget.moc"

