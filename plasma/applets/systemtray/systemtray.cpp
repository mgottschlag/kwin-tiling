/***************************************************************************
 *   systemtray.cpp                                                        *
 *                                                                         *
 *   Copyright (C) 2007 Alexander Rodin <rodin.alexander@gmail.com>        *
 *   Copyright (C) 2007 Jason Stubbs <jasonbstubbs@gmail.com>              *
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

// Qt
#include <QGraphicsView>

SystemTray::SystemTray(QObject *parent, const QVariantList &arguments)
    : Plasma::Applet(parent, arguments)
{
    resize(40,60);
    setSizePolicy(QSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred));
}

SystemTray::~SystemTray()
{
    // Get rid of our SystemTrayWidget if we still have one
    delete m_systemTrayWidget;
}

QVariant SystemTray::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemPositionHasChanged) {
        updateWidgetGeometry();
    }

    return Applet::itemChange(change, value);
}

void SystemTray::constraintsEvent(Plasma::Constraints constraints)
{   
    if (constraints & Plasma::SizeConstraint) {
        updateWidgetGeometry();
    }

    if (constraints & (Plasma::LocationConstraint | Plasma::FormFactorConstraint)) {
        updateWidgetOrientation();
    }
}

void SystemTray::updateSize()
{
    setPreferredSize(m_systemTrayWidget->sizeHint());
    resize(m_systemTrayWidget->sizeHint());
    updateWidgetGeometry();
}

void SystemTray::updateWidgetOrientation()
{
    if (!m_systemTrayWidget) {
        return;
    }
    // TODO: Handle other form factors
    if (formFactor() == Plasma::Horizontal) {
        m_systemTrayWidget->setOrientation(Qt::Horizontal);
    } else {
        m_systemTrayWidget->setOrientation(Qt::Vertical);
    }
}

void SystemTray::updateWidgetGeometry()
{
    QGraphicsView *parentView = view();
    if (!parentView) {
        kDebug()<<"Problem view is NULL";
        return;
    }

    if (!m_systemTrayWidget || m_systemTrayWidget->parentWidget() != parentView) {
        delete m_systemTrayWidget;
        m_systemTrayWidget = new SystemTrayWidget(parentView);
        m_systemTrayWidget->setOrientation(Qt::Horizontal);
        connect(m_systemTrayWidget, SIGNAL(sizeShouldChange()),
                this, SLOT(updateSize()));
        updateWidgetOrientation();
        m_systemTrayWidget->setVisible(true);

    }
    
    // Set the system tray to its minimum size and centre it above the item
    QRect itemRect = mapToView(parentView, geometry());
    QRect widgetRect = QRect(QPoint(0, 0), m_systemTrayWidget->minimumSizeHint());
    widgetRect.moveTop(itemRect.top() + (itemRect.height() - widgetRect.height()) / 2);
    widgetRect.moveLeft(itemRect.left() + (itemRect.width() - widgetRect.width()) / 2);
    m_systemTrayWidget->setMaximumSize(itemRect.size());
    m_systemTrayWidget->setGeometry(geometry().toRect());
}

#include "systemtray.moc"
