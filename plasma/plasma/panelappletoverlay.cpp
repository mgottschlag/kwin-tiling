/*
 *   Copyright 2008 Aaron Seigo <aseigo@kde.org>
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

#include "panelappletoverlay.h"

#include <QPainter>

#include <plasma/applet.h>

PanelAppletOverlay::PanelAppletOverlay(Plasma::Applet *applet, QWidget *parent)
    : QWidget(parent),
      m_applet(applet)
{
    setGeometry(applet->geometry().toRect());
    connect(m_applet, SIGNAL(destroyed(QObject*)), this, SLOT(deleteLater()));
}

void PanelAppletOverlay::paintEvent(QPaintEvent *event)
{
    QPainter p(this);
    //p.fillRect(rect(), palette().brush(QPalette::Window));
}

#include "panelappletoverlay.moc"


