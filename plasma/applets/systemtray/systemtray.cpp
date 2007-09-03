/***************************************************************************
 *   systemtray.cpp                                                        *
 *                                                                         *
 *   Copyright (C) 2007 Alexander Rodin <rodin.alexander@gmail.com>        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

// Own
#include "systemtray.h"
#include "systemtraywidget.h"

// KDE
#include <KWindowSystem>

SystemTray::SystemTray(QObject *parent, const QVariantList &arguments)
    : Plasma::Applet(parent, arguments),
      m_systemTrayWidget(new SystemTrayWidget(0,
                  Qt::FramelessWindowHint))
{
    setDrawStandardBackground(true);
    connect(m_systemTrayWidget, SIGNAL(sizeChanged()), SLOT(updateLayout()));
    m_systemTrayWidget->show();
    KWindowSystem::setState(m_systemTrayWidget->winId(),
            NET::SkipTaskbar | NET::SkipPager | NET::KeepBelow);
}

SystemTray::~SystemTray()
{
    delete m_systemTrayWidget;
}

QSizeF SystemTray::contentSizeHint() const
{
    return QSizeF(m_systemTrayWidget->size());
}

Qt::Orientations SystemTray::expandingDirections() const
{
    // simplify layouting by giving the system tray a fixed
    // size for now
    return 0;
}

QVariant SystemTray::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemPositionHasChanged) {
        m_systemTrayWidget->move(value.toPointF().toPoint());
    }
    return Plasma::Applet::itemChange(change, value);
}

#include "systemtray.moc"
