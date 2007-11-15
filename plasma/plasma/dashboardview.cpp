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

DashBoardView::DashBoardView(int screen, QWidget *parent)
    : Plasma::View(screen, PlasmaApp::self()->corona(), parent)
{
    setContextMenuPolicy(Qt::NoContextMenu);
    setWindowFlags( Qt::X11BypassWindowManagerHint | Qt::WindowStaysOnTopHint );
    setWindowOpacity( 0.9 );
    setDrawWallpaper(false);
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

#include "dashboardview.moc"

