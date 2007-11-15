/*
 *   Copyright 2007 Aaron Seigo <aseigo@kde.org>
 *   Copyright 2007 Matt Broadstone <mbroadst@gmail.com>
 *   Copyright 2007 André Duffeck <duffeck@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
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

#include "dashboardview.h"

#include "plasma/applet.h"
#include "plasma/corona.h"
#include "plasma/containment.h"
#include "plasma/svg.h"

#include "plasmaapp.h"

DashBoardView::DashBoardView(QWidget *parent, int screen)
    : QGraphicsView(parent),
      m_screen(screen)
{
    setFrameShape(QFrame::NoFrame);
    setAutoFillBackground(true);
    setDragMode(QGraphicsView::RubberBandDrag);
    setCacheMode(QGraphicsView::CacheBackground);
    setInteractive(true);
    setAcceptDrops(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setContextMenuPolicy(Qt::NoContextMenu);
    setWindowFlags( Qt::X11BypassWindowManagerHint | Qt::WindowStaysOnTopHint );
    setWindowOpacity( 0.9 );

    Plasma::Corona *corona = PlasmaApp::self()->corona();
    setScene(corona);

    Plasma::Containment * c = corona->containmentForScreen(screen);
    kDebug() << "dashboard view on screen" << screen << "has containment" << (qint64)c;
    if (c) {
        setSceneRect(c->geometry());
        connect(c, SIGNAL(geometryChanged()), this, SLOT(updateSceneRect()));
    }

    hide();
}

DashBoardView::~DashBoardView()
{
}

void DashBoardView::toggleVisibility()
{
    if (isHidden()) {
      show();
      raise();
    } else {
      hide();
    }
}

int DashBoardView::screen() const
{
    return m_screen;
}

void DashBoardView::updateSceneRect()
{
    Plasma::Containment * c =  PlasmaApp::self()->corona()->containmentForScreen(m_screen);
    if (c) {
        setSceneRect(c->geometry());
    }
}

#include "dashboardview.moc"
