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

#include <QApplication>
#include <QGraphicsLinearLayout>
#include <QPainter>
#include <QTimer>

#include <KGlobalSettings>
#include <KIcon>

#include <Plasma/Applet>
#include <Plasma/Containment>
#include <Plasma/PaintUtils>
#include <Plasma/Theme>
#include <Plasma/View>

class AppletMoveSpacer : public QGraphicsWidget
{
public:
    AppletMoveSpacer(Plasma::Applet *applet)
        : QGraphicsWidget(applet->containment()),
          m_applet(applet)
    {
    }

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget * widget = 0)
    {
        Q_UNUSED(option)
        Q_UNUSED(widget)

        /*
           results in odd painting corruption
        if (collidesWithItem(m_applet, Qt::IntersectsItemBoundingRect)) {
            painter->fillRect(contentsRect(), Qt::transparent);
            return;
        }
        */

        //TODO: make this a pretty gradient?
        painter->setRenderHint(QPainter::Antialiasing);
        QPainterPath p = Plasma::PaintUtils::roundedRectangle(contentsRect().adjusted(1, 1, -2, -2), 4);
        QColor c = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
        c.setAlphaF(0.3);

        painter->fillPath(p, c);
    }

private:
    QGraphicsWidget *m_applet;
};

PanelAppletOverlay::PanelAppletOverlay(Plasma::Applet *applet, QWidget *parent)
    : QWidget(parent),
      m_applet(applet),
      m_spacer(0),
      m_layout(static_cast<QGraphicsLinearLayout*>(applet->containment()->layout())), // ++assumptions;
      m_index(0),
      m_clickDrag(false)
{
    int i = 0;
    for (; i < m_layout->count(); ++i) {
        QGraphicsWidget *w = dynamic_cast<QGraphicsWidget*>(m_layout->itemAt(i));
        if (w == m_applet) {
            m_index = i;
            break;
        }
    }

    syncOrientation();
    syncGeometry();

    connect(m_applet, SIGNAL(destroyed(QObject*)), this, SLOT(deleteLater()));
    connect(m_applet, SIGNAL(geometryChanged()), this, SLOT(delaySyncGeometry()));
}

PanelAppletOverlay::~PanelAppletOverlay()
{
    if (m_spacer) {
        if (m_layout) {
            m_layout->removeItem(m_spacer);
        }

        m_spacer->deleteLater();
        m_spacer = 0;
    }
}

void PanelAppletOverlay::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QStyleOption op;
    op.initFrom(this);

    bool hovered = op.state & QStyle::State_MouseOver;
    bool mover = mouseGrabber() == this;
    if (!hovered || mover) {
        return;
    }

    QPainter p(this);
    KIcon icon("transform-move");
    int iconSize;
    QRect iconRect;

    if (m_orientation == Qt::Horizontal) {
        iconSize = qMin(qMin(height(), int(m_applet->size().width())), 64);
        iconRect = QRect(rect().center() - QPoint(iconSize / 2, iconSize / 2), QSize(iconSize, iconSize));
    } else {
        iconSize = qMin(qMin(width(), int(m_applet->size().height())), 64);
        iconRect = QRect(rect().center() - QPoint(iconSize / 2, iconSize / 2), QSize(iconSize, iconSize));
    }

    p.drawPixmap(iconRect, icon.pixmap(iconSize, iconSize));
}

void PanelAppletOverlay::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event)

    //kDebug() << m_clickDrag;
    if (m_clickDrag) {
        setMouseTracking(false);
        m_clickDrag = false;
        m_origin = QPoint();
        return;
    }

    if (event->button() != Qt::LeftButton) {
        //kDebug() << "sending even to" << (QWidget*)parent();
        Plasma::View *view = dynamic_cast<Plasma::View*>(parent());

        if (view && view->containment()) {
            view->containment()->showContextMenu(mapToParent(event->pos()), event->globalPos());
        }

        return;
    }

    m_clickDrag = false;
    if (!m_spacer) {
        m_spacer = new AppletMoveSpacer(m_applet);
    } else {
        m_layout->removeItem(m_spacer);
    }

    m_origin = mapToParent(event->pos());
    m_spacer->setMinimumSize(m_applet->geometry().size());
    m_spacer->setMaximumSize(m_applet->geometry().size());
    m_layout->removeItem(m_applet);
    m_layout->insertItem(m_index, m_spacer);
    m_applet->setZValue(m_applet->zValue() + 1);

    if (m_orientation == Qt::Horizontal) {
        m_offset = geometry().x() - m_origin.x();
    } else {
        m_offset = geometry().y() - m_origin.y();
    }

    grabMouse();
}

void PanelAppletOverlay::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_layout) {
        return;
    }

    Plasma::FormFactor f = m_applet->formFactor();

    if ( ((f != Plasma::Horizontal && f != Plasma::Vertical) && rect().intersects(m_applet->rect().toRect())) ||
          ((f == Plasma::Horizontal || f == Plasma::Vertical) && !rect().contains(event->globalPos())) ) {
        Plasma::View *view = Plasma::View::topLevelViewAt(event->globalPos());
        kDebug() << "checking view" << view << m_applet->view();

        if (!view) {
            return;
        }

        QPointF pos = view->mapFromGlobal(event->globalPos());
        if (view != m_applet->view()) {
            Plasma::Containment *c = view->containment();

            syncOrientation();
            syncGeometry();

            if (m_spacer) {
                m_layout->removeItem(m_spacer);
                m_spacer->deleteLater();
                m_spacer = 0;
            }

            QPointF pos = m_applet->view()->mapFromGlobal(event->globalPos());
            QRectF g = m_applet->geometry();
            pos += QPoint(m_offset, m_offset);
            g.moveTo(pos);
            m_applet->setGeometry(g);
            m_layout->removeItem(m_spacer);
            m_spacer->deleteLater();
            m_layout = 0;
            m_spacer = 0;
            c->addApplet(m_applet, pos, false);
            releaseMouse();
            return;
        }
    }

    if (!m_spacer) {
        m_spacer = new AppletMoveSpacer(m_applet);
        m_spacer->setMinimumSize(m_applet->geometry().size());
        m_spacer->setMaximumSize(m_applet->geometry().size());
        m_layout->removeItem(m_applet);
        m_layout->insertItem(m_index, m_spacer);
    }

    QPoint p = mapToParent(event->pos());
    QRectF g = m_applet->geometry();

    //kDebug() << p << g << "<-- movin'?";
    if (m_orientation == Qt::Horizontal) {
        g.moveLeft(p.x() + m_offset);
    } else {
        g.moveTop(p.y() + m_offset);
    }

    m_applet->setGeometry(g);

    // swap items if we pass completely over the next/previous item or cross
    // more than halfway across it, whichever comes first
    if (m_orientation == Qt::Horizontal) {
        //kDebug() << m_prevGeom << g << m_nextGeom;
        if (m_prevGeom.isValid() && g.left() <= m_prevGeom.left()) {
            swapWithPrevious();
        } else if (m_nextGeom.isValid() && g.right() >= m_nextGeom.right()) {
            swapWithNext();
        }
    } else if (m_prevGeom.isValid() && g.top() <= m_prevGeom.top()) {
        swapWithPrevious();
    } else if (m_nextGeom.isValid() && g.bottom() >= m_nextGeom.bottom()) {
        swapWithNext();
    }

    //kDebug() << "=================================";
}

void PanelAppletOverlay::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    if (!m_spacer) {
        releaseMouse();
        return;
    }

    if (!m_origin.isNull()) {
        //kDebug() << m_clickDrag << m_origin << mapToParent(event->pos());
        if (m_orientation == Qt::Horizontal) {
            m_clickDrag = abs(mapToParent(event->pos()).x() - m_origin.x()) < KGlobalSettings::dndEventDelay();
        } else {
            m_clickDrag = abs(mapToParent(event->pos()).y() - m_origin.y()) < KGlobalSettings::dndEventDelay();
        }

        if (m_clickDrag) {
            //kDebug() << "click dragging." << this << mouseGrabber();
            setMouseTracking(true);
            event->setAccepted(false);
            return;
        }
    }

    releaseMouse();
    //kDebug();
    m_layout->removeItem(m_spacer);
    m_spacer->deleteLater();
    m_spacer = 0;

    m_layout->insertItem(m_index, m_applet);
    m_applet->setZValue(m_applet->zValue() - 1);
}

void PanelAppletOverlay::enterEvent(QEvent *event)
{
    Q_UNUSED(event)
    update();
}

void PanelAppletOverlay::leaveEvent(QEvent *event)
{
    Q_UNUSED(event)
    update();
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
    m_layout->removeItem(m_spacer);
    m_layout->insertItem(m_index, m_spacer);
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
    m_layout->removeItem(m_spacer);
    m_layout->insertItem(m_index, m_spacer);
}

void PanelAppletOverlay::delaySyncGeometry()
{
    // we need to do this because it gets called in a round-about-way
    // from our own mouseMoveEvent. if we call syncGeometry directly,
    // we end up with a maze of duplicated and confused mouseMoveEvents
    // of which only half are real (the other half being caused by the
    // immediate call to syncGeometry!)
    QTimer::singleShot(0, this, SLOT(syncGeometry()));
}

void PanelAppletOverlay::syncGeometry()
{
    if (!m_layout) {
        return;
    }

    //kDebug();
    setGeometry(m_applet->geometry().toRect());

    if (m_index > 0) {
        m_prevGeom = m_layout->itemAt(m_index - 1)->geometry();
    } else {
        m_prevGeom = QRectF();
    }

    //kDebug() << m_index << m_layout->count();
    if (m_index < m_layout->count() - 1) {
        m_nextGeom = m_layout->itemAt(m_index + 1)->geometry();
    } else {
        m_nextGeom = QRectF();
    }
}

void PanelAppletOverlay::syncOrientation()
{
    m_orientation = m_applet->formFactor() == Plasma::Horizontal ? Qt::Horizontal : Qt::Vertical;
}

#include "panelappletoverlay.moc"

