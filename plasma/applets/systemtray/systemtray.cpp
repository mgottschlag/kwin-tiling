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
#include <QGraphicsProxyWidget>

SystemTray::SystemTray(QObject *parent, const QVariantList &arguments)
    : Plasma::Applet(parent, arguments)
{
    m_proxyWidget = new QGraphicsProxyWidget(this);
    m_systemTrayWidget = new SystemTrayWidget(0);
    m_proxyWidget->setWidget(m_systemTrayWidget);
    connect(m_systemTrayWidget, SIGNAL(sizeShouldChange()),
            this, SLOT(updateWidgetGeometry()));
    setPreferredSize(m_systemTrayWidget->size());
    updateWidgetOrientation();
    m_systemTrayWidget->setVisible(true);
}

SystemTray::~SystemTray()
{
    // Get rid of our SystemTrayWidget if we still have one
    delete m_systemTrayWidget;
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
    setPreferredSize(m_systemTrayWidget->size());
    m_proxyWidget->resize(effectiveSizeHint(Qt::PreferredSize));
}

#include "systemtray.moc"
