/*
 *   Copyright 2010 Marco Martin <notmart@gmail.com>
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

#include "appletsview.h"

#include <QTimer>
#include <QGraphicsSceneMouseEvent>

#include <KGlobalSettings>

#include <Plasma/Containment>

AppletsView::AppletsView(QGraphicsItem *parent)
    : Plasma::ScrollWidget(parent)
{
}

AppletsView::~AppletsView()
{
}

void AppletsView::setAppletsContainer(AppletsContainer *appletsContainer)
{
    m_appletsContainer = appletsContainer;
    setWidget(appletsContainer);
}

AppletsContainer *AppletsView::appletsContainer() const
{
    return m_appletsContainer;
}

bool AppletsView::sceneEventFilter(QGraphicsItem *watched, QEvent *event)
{
    if (m_appletsContainer->expandAll() && m_appletsContainer->orientation() == Qt::Vertical) {
        return Plasma::ScrollWidget::sceneEventFilter(watched, event);
    }

    if (!m_appletsContainer->containment()) {
        return Plasma::ScrollWidget::sceneEventFilter(watched, event);
    }

    if (event->type() == QEvent::GraphicsSceneMousePress) {
        m_appletsContainer->m_appletActivationTimer->stop();
        foreach (Plasma::Applet *applet, m_appletsContainer->containment()->applets()) {
            if (applet->isAncestorOf(watched)) {
                if (applet == m_appletsContainer->m_currentApplet.data() || applet == m_appletsContainer->m_pendingCurrentApplet.data()) {
                    return Plasma::ScrollWidget::sceneEventFilter(watched, event);
                }

                event->ignore();
                return Plasma::ScrollWidget::sceneEventFilter(watched, event);
            }
        }
    } else if (event->type() == QEvent::GraphicsSceneMouseMove) {
        QGraphicsSceneMouseEvent *me = static_cast<QGraphicsSceneMouseEvent *>(event);
        if (QPointF(me->pos() - me->buttonDownPos(me->button())).manhattanLength() > KGlobalSettings::dndEventDelay()) {
            m_appletsContainer->m_appletActivationTimer->stop();
        }

        if (!m_appletsContainer->m_currentApplet || !m_appletsContainer->m_currentApplet.data()->isAncestorOf(watched)) {
            Plasma::ScrollWidget::sceneEventFilter(watched, event);
            event->ignore();
            return true;
        } else if (m_appletsContainer->m_currentApplet.data()->isAncestorOf(watched)) {
            return false;
        }
    //don't manage wheel events over the current applet
    } else if (event->type() == QEvent::GraphicsSceneWheel && m_appletsContainer->m_currentApplet && m_appletsContainer->m_currentApplet.data()->isAncestorOf(watched)) {
        return false;
    } else if (event->type() == QEvent::GraphicsSceneMouseRelease) {
        foreach (Plasma::Applet *applet, m_appletsContainer->containment()->applets()) {
            if (applet->isAncestorOf(watched)) {

                QGraphicsSceneMouseEvent *me = static_cast<QGraphicsSceneMouseEvent *>(event);

                if (QPointF(me->pos() - me->buttonDownPos(me->button())).manhattanLength() > KGlobalSettings::dndEventDelay()) {
                    m_appletsContainer->m_appletActivationTimer->stop();
                    return Plasma::ScrollWidget::sceneEventFilter(watched, event);
                }

                if (m_appletsContainer->m_currentApplet.data()) {
                    m_appletsContainer->m_currentApplet.data()->setPreferredHeight(m_appletsContainer->optimalAppletSize(m_appletsContainer->m_currentApplet.data(), false).height());
                }
                m_appletsContainer->m_pendingCurrentApplet = applet;
                m_appletsContainer->m_currentApplet.clear();
                applet->setPreferredHeight(m_appletsContainer->optimalAppletSize(applet, true).height());

                m_appletsContainer->m_appletActivationTimer->start(500);

                return Plasma::ScrollWidget::sceneEventFilter(watched, event);
            }
        }

        if (!m_appletsContainer->m_currentApplet || !m_appletsContainer->m_currentApplet.data()->isAncestorOf(watched)) {
            return Plasma::ScrollWidget::sceneEventFilter(watched, event);
        }
    }

    if (watched == m_appletsContainer->m_currentApplet.data()) {
        return false;
    } else {
        return Plasma::ScrollWidget::sceneEventFilter(watched, event);
    }
}

#include "appletsview.moc"

