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

    setScene(PlasmaApp::self()->corona());

    QDesktopWidget desktop;
    setSceneRect(desktop.screenGeometry(screen));

    //TODO: should we delay the init of the actions until we actually need them?
    m_zoomInAction = new QAction(i18n("Zoom In"), this);
    connect(m_zoomInAction, SIGNAL(triggered(bool)), this, SLOT(zoomIn()));
    m_zoomInAction->setEnabled(false);
    m_zoomOutAction = new QAction(i18n("Zoom Out"), this);
    connect(m_zoomOutAction, SIGNAL(triggered(bool)), this, SLOT(zoomOut()));
}

DesktopView::~DesktopView()
{
}

void DesktopView::zoomIn()
{
    if (m_zoomLevel == Plasma::GroupZoom) {
        m_zoomLevel = Plasma::DesktopZoom;
        m_zoomInAction->setEnabled(false);
        m_zoomOutAction->setEnabled(true);
    } else if (m_zoomLevel == Plasma::OverviewZoom) {
        m_zoomLevel = Plasma::GroupZoom;
        m_zoomInAction->setEnabled(true);
        m_zoomOutAction->setEnabled(true);
    }

    qreal s = Plasma::scalingFactor(m_zoomLevel) / matrix().m11();
    scale(s, s);
}

void DesktopView::zoomOut()
{
    if (m_zoomLevel == Plasma::DesktopZoom) {
        m_zoomLevel = Plasma::GroupZoom;
        m_zoomInAction->setEnabled(true);
        m_zoomOutAction->setEnabled(true);
    } else if (m_zoomLevel == Plasma::GroupZoom) {
        m_zoomLevel = Plasma::OverviewZoom;
        m_zoomInAction->setEnabled(true);
        m_zoomOutAction->setEnabled(false);
    }

    qreal s = Plasma::scalingFactor(m_zoomLevel) / matrix().m11();
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

