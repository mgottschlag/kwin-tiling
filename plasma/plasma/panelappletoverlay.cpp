/*
 *   Copyright 2008 Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2,
 *   or (at your option) any later version.
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

#include "panelappletoverlay.h"

#include <QGraphicsLinearLayout>
#include <QPainter>

#include <plasma/applet.h>
#include <plasma/containment.h>
#include <plasma/paintutils.h>

PanelAppletOverlay::PanelAppletOverlay(Plasma::Applet *applet, QWidget *parent)
    : QWidget(parent),
      m_applet(applet),
      m_orientation(applet->formFactor() == Plasma::Horizontal ? Qt::Horizontal : Qt::Vertical),
      m_layout(static_cast<QGraphicsLinearLayout*>(applet->containment()->layout())), // ++assumptions;
      m_index(0)
{
    int i = 0;
    for (; i < m_layout->count(); ++i) {
        QGraphicsLayoutItem *l = m_layout->itemAt(i);
        QGraphicsWidget *w = dynamic_cast<QGraphicsWidget*>(l);
        if (w == m_applet) {
            m_index = i;
            break;
        }
        m_prevGeom = l->geometry();
    }

    if (++i < m_layout->count()) {
        // we have an item after us!
        m_nextGeom = m_layout->itemAt(i)->geometry();
    }

    syncGeometry();
    connect(m_applet, SIGNAL(destroyed(QObject*)), this, SLOT(deleteLater()));
    connect(m_applet, SIGNAL(geometryChanged()), this, SLOT(syncGeometry()));
}

void PanelAppletOverlay::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter p(this);
    QPainterPath path = Plasma::PaintUtils::roundedRectangle(rect(), 6);
    p.fillPath(path, palette().brush(QPalette::Window));
}

void PanelAppletOverlay::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event)

    grabMouse();
}

void PanelAppletOverlay::mouseMoveEvent(QMouseEvent *event)
{
    Q_UNUSED(event)

    QPoint p = mapToParent(event->pos());
    QRect g = geometry();
    //kDebug() << p << g << (QObject*)m_applet << m_prevGeom << m_nextGeom;

    if (m_orientation == Qt::Horizontal) {
        g.moveLeft(p.x());
    } else {
        g.moveTop(p.y());
    }

    setGeometry(g);

    // swap items if we pass completely over the next/previou item or cross
    // more than halfway across it, whichever comes first
    if (m_orientation == Qt::Horizontal) {
        if (m_prevGeom.isValid() &&
            (g.x() < m_prevGeom.center().x() || g.right() < m_prevGeom.right())) {
            swapWithPrevious();
        } else if (m_nextGeom.isValid() && 
                   (g.right() > m_nextGeom.center().x() || g.left() > m_nextGeom.left())) {
            swapWithNext();
        }
    } else if (m_prevGeom.isValid() &&
               (g.y() < m_prevGeom.center().y() || g.bottom() < m_prevGeom.bottom())) {
        swapWithPrevious();
    } else if (m_nextGeom.isValid() && 
               (g.y() > m_nextGeom.center().y() || g.top() > m_nextGeom.top())) {
        swapWithNext();
    }

    //kDebug() << "=================================";
}

void PanelAppletOverlay::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event)

    //kDebug();
    releaseMouse();
    syncGeometry();
}

void PanelAppletOverlay::swapWithPrevious()
{
    //kDebug();
    --m_index;

    if (m_index > 0) {
        m_prevGeom = m_layout->itemAt(m_index - 1)->geometry();
    } else {
        m_prevGeom = QRectF();
    }

    m_nextGeom = m_layout->itemAt(m_index + 1)->geometry();
    m_layout->removeItem(m_applet);
    m_layout->insertItem(m_index, m_applet);
    releaseMouse();
}

void PanelAppletOverlay::swapWithNext()
{
    //kDebug();
    ++m_index;

    if (m_index < m_layout->count() - 1) {
        m_nextGeom = m_layout->itemAt(m_index + 1)->geometry();
    } else {
        m_nextGeom = QRectF();
    }

    m_prevGeom = m_layout->itemAt(m_index - 1)->geometry();
    m_layout->removeItem(m_applet);
    m_layout->insertItem(m_index, m_applet);
    releaseMouse();
}

void PanelAppletOverlay::syncGeometry()
{
    //kDebug();
    setGeometry(m_applet->geometry().toRect());
}

#include "panelappletoverlay.moc"

