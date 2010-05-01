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

#include <KAction>

#include <Plasma/Containment>
#include <Plasma/Corona>
#include <Plasma/Theme>

namespace SystemTray
{

    DBusSystemTrayWidget::DBusSystemTrayWidget(Plasma::Applet *parent, Plasma::Service *service)
    : Plasma::IconWidget(parent),
      m_service(service),
      m_host(parent)
{
    connect(this, SIGNAL(clicked()), this, SLOT(calculateShowPosition()));

    KAction *action = new KAction(this);
    setAction(action);
    connect(action, SIGNAL(triggered()), this, SLOT(calculateShowPosition()));
}

void DBusSystemTrayWidget::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    Plasma::IconWidget::mousePressEvent(event);

    if (event->button() == Qt::MidButton) {
        event->accept();
    }
}

void DBusSystemTrayWidget::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::MidButton) {
        KConfigGroup params = m_service->operationDescription("SecondaryActivate");
        params.writeEntry("x", event->screenPos().x());
        params.writeEntry("y", event->screenPos().y());
        m_service->startOperationCall(params);
    }

    Plasma::IconWidget::mouseReleaseEvent(event);
}

void DBusSystemTrayWidget::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    KConfigGroup params = m_service->operationDescription("Scroll");
    params.writeEntry("delta", event->delta());
    params.writeEntry("direction", "Vertical");
    m_service->startOperationCall(params);
}

void DBusSystemTrayWidget::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    KConfigGroup params = m_service->operationDescription("ContextMenu");
    params.writeEntry("x", event->screenPos().x());
    params.writeEntry("y", event->screenPos().y());
    m_service->startOperationCall(params);
}

void DBusSystemTrayWidget::calculateShowPosition()
{
    Plasma::Corona *corona = m_host->containment()->corona();
    QSize s(1, 1);
    QPoint pos = corona->popupPosition(this, s);
    KConfigGroup params = m_service->operationDescription("Activate");
    params.writeEntry("x", pos.x());
    params.writeEntry("y", pos.y());
    m_service->startOperationCall(params);
}

void DBusSystemTrayWidget::setIcon(const QString &iconName, const QIcon &icon)
{
    if (!iconName.isEmpty()) {
        QString name = QString("icons/") + iconName.split("-").first();
        if (Plasma::Theme::defaultTheme()->imagePath(name).isEmpty()) {
            Plasma::IconWidget::setIcon(icon);
        } else {
            setSvg(name, iconName);
        }
    } else {
        Plasma::IconWidget::setIcon(icon);
    }
}


}

