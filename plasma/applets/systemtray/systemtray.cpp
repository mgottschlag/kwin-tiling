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

//Plasma
#include <plasma/panelsvg.h>

SystemTray::SystemTray(QObject *parent, const QVariantList &arguments)
    : Plasma::Applet(parent, arguments)
{
    m_background = new Plasma::PanelSvg(this);
    m_background->setImagePath("widgets/systemtray");
    resize(40,60);
    m_background->resizePanel(size());
    connect(this, SIGNAL(geometryChanged()), this, SLOT(updateWidgetGeometry()));
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
        m_background->resizePanel(size());
    }

    if (constraints & (Plasma::LocationConstraint | Plasma::FormFactorConstraint)) {
        updateWidgetOrientation();
    }
}

void SystemTray::paintInterface(QPainter *painter,
                                const QStyleOptionGraphicsItem *option,
                                const QRect& contentsRect)
{
    Q_UNUSED(option)

    m_background->paintPanel(painter, contentsRect);
}

void SystemTray::updateSize()
{
    setPreferredSize(m_systemTrayWidget->sizeHint());
    updateGeometry();
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
        updateWidgetOrientation();
        connect(m_systemTrayWidget, SIGNAL(sizeShouldChange()),
                this, SLOT(updateSize()));
        m_systemTrayWidget->setVisible(true);
    }

    // Set the widget's maximum size to the size that this applet
    // has been allocated. The widget will use that size to calculate
    // whether to add icons horizontally or vertically.
    QRect r = mapToView(parentView, rect());
    m_systemTrayWidget->setMaximumSize(r.size());

    // Center the widget within the applet area
    QSize s = m_systemTrayWidget->minimumSizeHint();
    r.moveLeft(r.left() + (r.width() - s.width()) / 2);
    r.moveTop(r.top() + (r.height() - s.height()) / 2);
    r.setSize(s);
    m_systemTrayWidget->setGeometry(r);
}

#include "systemtray.moc"
