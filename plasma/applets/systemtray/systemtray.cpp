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
}

SystemTray::~SystemTray()
{
    // Get rid of our SystemTrayWidget if we still have one
    delete m_systemTrayWidget;
}

QSizeF SystemTray::contentSizeHint() const
{
    QGraphicsView *v = view();
    if (!(m_systemTrayWidget && v)) {
        return QSizeF();
    }
    QRect widgetRect(QPoint(0, 0), m_systemTrayWidget->minimumSizeHint());
    return mapFromView(v, widgetRect).size();
}

Qt::Orientations SystemTray::expandingDirections() const
{
    // Extra space isn't useful in either direction
    return 0;
}

void SystemTray::constraintsUpdated(Plasma::Constraints constraints)
{
    if (constraints & (Plasma::LocationConstraint | Plasma::SizeConstraint)) {
        updateWidgetGeometry();
    }
    if (constraints & Plasma::FormFactorConstraint) {
        updateWidgetOrientation();
    }
}

void SystemTray::updateSize()
{
    // contentSizeHint() will return a new size so let the layout know
    updateGeometry();
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
        return;
    }

    if (!m_systemTrayWidget || m_systemTrayWidget->parentWidget() != parentView) {
        if (m_systemTrayWidget) {
            m_systemTrayWidget->setParent(parentView);
        } else {
            m_systemTrayWidget = new SystemTrayWidget(parentView);
            connect(m_systemTrayWidget, SIGNAL(sizeShouldChange()),
                    this, SLOT(updateSize()));
        }
        updateWidgetOrientation();
        m_systemTrayWidget->setVisible(true);
    }

    // Set the system tray to its minimum size and centre it above the item
    QRect itemRect = mapToView(parentView, boundingRect());
    QRect widgetRect = QRect(QPoint(0, 0), m_systemTrayWidget->minimumSizeHint());
    widgetRect.moveTop(itemRect.top() + (itemRect.height() - widgetRect.height()) / 2);
    widgetRect.moveLeft(itemRect.left() + (itemRect.width() - widgetRect.width()) / 2);
    m_systemTrayWidget->setMaximumSize(itemRect.size());
    m_systemTrayWidget->setGeometry(widgetRect);
}

#include "systemtray.moc"
