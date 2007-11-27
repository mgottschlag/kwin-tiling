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

SystemTray::SystemTray(QObject *parent, const QVariantList &arguments)
    : Plasma::Applet(parent, arguments)
{ }

SystemTray::~SystemTray()
{
    // Get rid of our SystemTrayWidget if we still have one
    delete m_systemTrayWidget;
}

QSizeF SystemTray::contentSizeHint() const
{
    if (!m_currentView) {
        return QSizeF();
    }

    QRect widgetRect;
    widgetRect.setSize(m_systemTrayWidget->minimumSizeHint());

    // Transform the size into the coordinates used by our QGraphicsView
    // Using mapToScene() causes us to lose QSize(1, 1) so we add it back
    QSizeF size = m_currentView->mapToScene(widgetRect).boundingRect().size() + QSize(1, 1);
    return size;
}

Qt::Orientations SystemTray::expandingDirections() const
{
    // Extra space isn't useful in either direction
    return 0;
}

QVariant SystemTray::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    // We've been added to a scene
    if (change == ItemSceneChange) {
        // If we were previously part of a different scene, stop monitoring it
        // for changes
        if (m_currentScene) {
            disconnect(m_currentScene, SIGNAL(changed(const QList<QRectF> &)),
                       this, SLOT(handleSceneChange(const QList<QRectF> &)));
        }
        // Make a note of what scene we're on and start
        // monitoring it for changes
        // We need to monitor all changes because a QGraphicsItem normally
        // only gets notified of changes relative to its parent but we need
        // to know about changes relative to the view to be able to correctly
        // place our SystemTrayWidget
        m_currentScene = value.value<QGraphicsScene *>();
        if (m_currentScene) {
            connect(m_currentScene, SIGNAL(changed(const QList<QRectF> &)),
                    this, SLOT(handleSceneChange(const QList<QRectF> &)));
        }
    }
    return Plasma::Applet::itemChange(change, value);
}

void SystemTray::handleSceneChange(const QList<QRectF> &region)
{
    // Create or reparent our system tray to the current view and update the
    // widget's geometry to match this item.

    // Don't do anything if none of the scene changes affect us
    if (!intersectsRegion(region)) {
        return;
    }

    // Find out which QGraphicsView (if any) that we are visible on
    QGraphicsView *view = findView();
    if (!view) {
        return;
    }

    // If the view is different to the view we were previously visible on or we
    // had no view up until now, (re)create or reparent our SystemTrayWidget
    if (view != m_currentView) {
        m_currentView = view;
        if (m_systemTrayWidget) {
            m_systemTrayWidget->setParent(m_currentView);
        } else {
            m_systemTrayWidget = new SystemTrayWidget(view);
            connect(m_systemTrayWidget, SIGNAL(sizeShouldChange()), this, SLOT(updateSize()));
        }
        m_systemTrayWidget->setVisible(true);
    }

    // Set our SystemTrayWidget's size and position equal to that of this item
    // Using mapFromScene() causes us to gain QSize(1, 1) so we take it off
    QRect rect = m_currentView->mapFromScene(sceneBoundingRect()).boundingRect().adjusted(0, 0, -1, -1);
    m_systemTrayWidget->setMaximumSize(rect.size());
    m_systemTrayWidget->setGeometry(rect);
}

bool SystemTray::intersectsRegion(const QList<QRectF> &region)
{
   foreach (const QRectF &rect, region) {
        if (rect.intersects(sceneBoundingRect())) {
            return true;
        }
    }
    return false;
}

QGraphicsView * SystemTray::findView()
{
    // We may be visible on more than one QGraphicsView but we only take the
    // first because we are only able to display one system tray
    foreach (QGraphicsView *view, m_currentScene->views()) {
        if (view->sceneRect().contains(scenePos())) {
            return view;
        }
    }
    return 0;
}

void SystemTray::updateSize()
{
    // Just ask our parent's layout to give us an appropriate size
    updateGeometry();
}

#include "systemtray.moc"
