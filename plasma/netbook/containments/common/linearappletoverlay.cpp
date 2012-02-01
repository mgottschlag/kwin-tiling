/*
 *   Copyright 2009 Marco Martin <notmart@gmail.com>
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

#include "linearappletoverlay.h"

#include "../common/appletmovespacer.h"

#include <QGraphicsLinearLayout>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>

#include <KGlobalSettings>

#include <Plasma/Applet>
#include <Plasma/Containment>
#include <Plasma/PaintUtils>
#include <Plasma/ScrollWidget>

LinearAppletOverlay::LinearAppletOverlay(Plasma::Containment *parent, QGraphicsLinearLayout *layout)
    : QGraphicsWidget(parent),
      m_applet(0),
      m_containment(parent),
      m_layout(layout),
      m_spacer(0),
      m_spacerIndex(0),
      m_clickDrag(false)
{
    setAcceptHoverEvents(true);
    setAcceptDrops(true);
    setZValue(900);
}

LinearAppletOverlay::~LinearAppletOverlay()
{
}

void LinearAppletOverlay::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget)

    QColor c = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
    c.setAlphaF(0.15);

    painter->fillRect(option->exposedRect, c);

    if (m_applet) {
        QRectF geom = m_applet->geometry();
        geom.moveTopLeft(geom.topLeft());
        c = Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor);
        c.setAlphaF(0.30);
        QPainterPath p = Plasma::PaintUtils::roundedRectangle(geom, 4);
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->fillPath(p, c);
        painter->restore();
    }
}

void LinearAppletOverlay::appletDestroyed()
{
    m_applet = 0;
}

void LinearAppletOverlay::spacerRequestedDrop(QGraphicsSceneDragDropEvent *event)
{
    dropEvent(event);
}

void LinearAppletOverlay::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        //Hack to make scene::itemAt() work
        int z = zValue();
        setZValue(-100);
        //FIXME:here we don't have a screen pos in the event
        m_containment->showContextMenu(event->pos(), event->pos().toPoint());
        setZValue(z);
        return;
    }

    if (m_clickDrag) {
        m_clickDrag = false;
        m_origin = QPoint();
        return;
    }

    if (m_applet) {
        m_origin = event->pos();
        showSpacer(event->pos());
        if (m_layout) {
            m_layout->removeItem(m_applet);
            m_applet->raise();
        }
        if (m_spacer) {
            m_spacer->setMinimumHeight(m_applet->size().height());
        }
    }
}

void LinearAppletOverlay::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    if (m_clickDrag) {
        //Cheat and pretend a mousemoveevent is arrived
        QGraphicsSceneMouseEvent me;
        me.setPos(event->pos());
        me.setLastPos(event->lastPos());
        mouseMoveEvent(&me);
        return;
    }

    if (m_applet) {
        disconnect(m_applet, SIGNAL(destroyed()), this, SLOT(appletDestroyed()));
    }
    m_applet = 0;

    Plasma::Applet *oldApplet = 0;

    //FIXME: is there a way more efficient than this linear one? scene()itemAt() won't work because it would always be == this
    foreach (Plasma::Applet *applet, m_containment->applets()) {
        if (applet->geometry().contains(event->pos())) {
            m_applet = applet;
            connect(applet, SIGNAL(destroyed()), this, SLOT(appletDestroyed()));
            break;
        }
    }
    if (m_applet != oldApplet) {
        update();
    }
}

void LinearAppletOverlay::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_spacer) {
        QPointF delta = event->pos()-event->lastPos();
        if (m_applet) {
            if (m_containment->formFactor() == Plasma::Vertical) {
                m_applet->moveBy(0, delta.y());
            } else {
                m_applet->moveBy(delta.x(), 0);
            }
        }
        showSpacer(event->pos());
    }

    update();
}

void LinearAppletOverlay::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event)

    QPoint delta = event->pos().toPoint() - m_origin.toPoint();
    if (m_origin != QPointF() && delta.manhattanLength() < KGlobalSettings::dndEventDelay()) {
        m_clickDrag = true;
        m_origin = QPointF();
        return;
    }

    if (m_spacer && m_layout) {
        m_layout->removeItem(m_spacer);
        if (m_applet) {
            m_layout->insertItem(m_spacerIndex, m_applet);
        }
    }

    delete m_spacer;
    m_spacer = 0;
    m_spacerIndex = 0;
}

void LinearAppletOverlay::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    showSpacer(event->pos());
    event->accept();
}

void LinearAppletOverlay::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    showSpacer(event->pos());
}

void LinearAppletOverlay::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    event->setPos(mapToParent(event->pos()));
    emit dropRequested(event);

    if (m_layout) {
        m_layout->removeItem(m_spacer);
    }

    if (m_spacer) {
        m_spacer->deleteLater();
    }

    m_spacer = 0;
    m_spacerIndex = 0;
}

void LinearAppletOverlay::showSpacer(const QPointF &pos)
{
    if (!scene()) {
        return;
    }

    if (pos == QPoint()) {
        if (m_spacer) {
            m_layout->removeItem(m_spacer);
            m_spacer->hide();
        }
        return;
    }

    //lucky case: the spacer is already in the right position
    if (m_spacer && m_spacer->geometry().contains(pos)) {
        return;
    }

    int insertIndex = -1;

    for (int i = 0; i < m_layout->count(); ++i) {
        if (!dynamic_cast<Plasma::Applet *>(m_layout->itemAt(i)) &&
            !dynamic_cast<AppletMoveSpacer *>(m_layout->itemAt(i))) {
            continue;
        }
        QRectF siblingGeometry = m_layout->itemAt(i)->geometry();

        if (m_containment->formFactor() != Plasma::Vertical) {
            qreal middle = siblingGeometry.center().x();
            if (pos.x() < middle) {
                insertIndex = i;
                break;
            } else if (pos.x() <= siblingGeometry.right()) {
                insertIndex = i + 1;
                break;
            }
        } else { // Vertical
            qreal middle = siblingGeometry.center().y();

            if (pos.y() < middle) {
                insertIndex = i;
                break;
            } else if (pos.y() <= siblingGeometry.bottom()) {
                insertIndex = i + 1;
                break;
            }
        }
    }

    if (m_spacerIndex < insertIndex) {
        --insertIndex;
    }

    //if is -1 let's see if there are spacers, zero, one or two
    if (insertIndex < 0) {
        bool firstSpacer = (!dynamic_cast<Plasma::Applet *>(m_layout->itemAt(0)) &&
                            !dynamic_cast<AppletMoveSpacer *>(m_layout->itemAt(0)));
        bool lastSpacer = (!dynamic_cast<Plasma::Applet *>(m_layout->itemAt(m_layout->count() - 1)) &&
                           !dynamic_cast<AppletMoveSpacer *>(m_layout->itemAt(m_layout->count() - 1)));

        if (firstSpacer && lastSpacer && m_layout->count() > 1) {
            insertIndex = m_layout->count() - 2;
        } else if (lastSpacer) {
            insertIndex = 0;
        }
    }

    m_spacerIndex = insertIndex;
    if (insertIndex != -1) {
        if (!m_spacer) {
            m_spacer = new AppletMoveSpacer(this);
            connect (m_spacer, SIGNAL(dropRequested(QGraphicsSceneDragDropEvent*)), 
                     this, SLOT(spacerRequestedDrop(QGraphicsSceneDragDropEvent*)));
        }

        m_layout->removeItem(m_spacer);
        m_spacer->show();
        m_layout->insertItem(insertIndex, m_spacer);
    }
}

#include <linearappletoverlay.moc>

