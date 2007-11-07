/*
 *   Copyright 2007 Aaron Seigo <aseigo@kde.org>
 *   Copyright 2007 Matt Broadstone <mbroadst@gmail.com>
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

#include "desktopview.h"

#include <QAction>
#include <QDesktopWidget>
#include <QFile>
#include <QWheelEvent>
#include <QCoreApplication>

#include <KAuthorized>
#include <KMenu>
#include <KRun>
#include <KToggleAction>

#include "plasma/applet.h"
#include "plasma/corona.h"
#include "plasma/containment.h"
#include "plasma/svg.h"

#include "plasmaapp.h"

DesktopView::DesktopView(QWidget *parent, int screen)
    : QGraphicsView(parent),
      m_screen(screen),
      m_zoomLevel(Plasma::DesktopZoom)
{
    setFrameShape(QFrame::NoFrame);
    setAutoFillBackground(true);
    setDragMode(QGraphicsView::RubberBandDrag);
    setCacheMode(QGraphicsView::CacheBackground);
    setInteractive(true);
    setAcceptDrops(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
//    setContextMenuPolicy(Qt::NoContextMenu);

    Plasma::Corona *corona = PlasmaApp::self()->corona();
    setScene(corona);

    Plasma::Containment * c = corona->containmentForScreen(screen);
    kDebug() << "desktop view on screen" << screen << "has containment" << (qint64)c;
    if (c) {
        setSceneRect(c->geometry());
        connect(c, SIGNAL(zoomIn()), this, SLOT(zoomIn()));
        connect(c, SIGNAL(zoomOut()), this, SLOT(zoomOut()));
    }
}

DesktopView::~DesktopView()
{
}

void DesktopView::zoomIn()
{
	qreal s = Plasma::scalingFactor(m_zoomLevel) / matrix().m11();
    if (m_zoomLevel == Plasma::GroupZoom) {
        m_zoomLevel = Plasma::DesktopZoom;
		s = Plasma::scalingFactor(m_zoomLevel) / matrix().m11();
		setSceneRect(geometry());
    } else if (m_zoomLevel == Plasma::OverviewZoom) {
        m_zoomLevel = Plasma::GroupZoom;
		qreal factor = Plasma::scalingFactor(m_zoomLevel);
		s = factor / matrix().m11();
		setSceneRect(QRectF(0, 0, width() * 1.0/factor, height() * 1.0/factor));
    }

    scale(s, s);
}

void DesktopView::zoomOut()
{
    if (m_zoomLevel == Plasma::DesktopZoom) {
        m_zoomLevel = Plasma::GroupZoom;
    } else if (m_zoomLevel == Plasma::GroupZoom) {
        m_zoomLevel = Plasma::OverviewZoom;
    }

	qreal factor = Plasma::scalingFactor(m_zoomLevel);
    qreal s = factor / matrix().m11();
	setSceneRect(QRectF(0, 0, width() * 1.0/factor, height() * 1.0/factor));
    scale(s, s);
}

void DesktopView::wheelEvent(QWheelEvent* event)
{
    if (scene() && scene()->itemAt(event->pos())) {
        QGraphicsView::wheelEvent(event);
        return;
    }

    if (event->modifiers() & Qt::ControlModifier) {
        if (event->delta() < 0) {
            zoomOut();
        } else {
            zoomIn();
        }
    }
}

#include "desktopview.moc"

