/***************************************************************************
 *                                                                         *
 *   Copyright (C) 2009 Aaron Seigo <aseigo@kde.org>                       *
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

#include "dbussystemtraywidget.h"

#include <QDBusAbstractInterface>
#include <QGraphicsSceneWheelEvent>

#include <Plasma/Containment>
#include <Plasma/Corona>

namespace SystemTray
{

DBusSystemTrayWidget::DBusSystemTrayWidget(Plasma::Applet *parent, QDBusAbstractInterface *iface)
    : Plasma::IconWidget(parent),
      m_iface(iface),
      m_host(parent)
{
    connect(this, SIGNAL(clicked()), this, SLOT(calculateShowPosition()));
}

void DBusSystemTrayWidget::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::MidButton) {
        m_iface->call(QDBus::NoBlock, "SecondaryActivate", event->screenPos().x(), event->screenPos().y());
    }
    Plasma::IconWidget::mouseReleaseEvent(event);
}

void DBusSystemTrayWidget::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    //kDebug() << m_iface << event->delta();
    if (m_iface) {
        m_iface->call(QDBus::NoBlock, "Wheel", event->delta());
    }
}

void DBusSystemTrayWidget::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    if (m_iface) {
        m_iface->call(QDBus::NoBlock, "ContextMenu", event->screenPos().x(), event->screenPos().y());
    }
}

void DBusSystemTrayWidget::calculateShowPosition()
{
    if (m_iface) {
        Plasma::Corona *corona = m_host->containment()->corona();
        QSize s(1, 1);
        QPoint pos = corona->popupPosition(this, s);
        m_iface->call(QDBus::NoBlock, "Activate", pos.x(), pos.y());
    }
}

}

