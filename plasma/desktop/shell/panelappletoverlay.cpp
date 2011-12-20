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
#include "panelapplethandle.h"

#include <QApplication>
#include <QGraphicsLinearLayout>
#include <QGraphicsSceneContextMenuEvent>
#include <QPainter>
#include <QTimer>
#include <QAction>
#include <QMenu>

#include <KDebug>
#include <KGlobalSettings>
#include <KIcon>

#include <Plasma/Applet>
#include <Plasma/Containment>
#include <Plasma/Corona>
#include <Plasma/PaintUtils>
#include <Plasma/Theme>
#include <Plasma/View>


PanelAppletHandle *PanelAppletOverlay::s_appletHandle = 0;
int PanelAppletOverlay::s_appletHandleCount = 0;

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
      m_layout(dynamic_cast<QGraphicsLinearLayout*>(applet->containment()->layout())), // ++assumptions;
      m_index(0),
      m_clickDrag(false)
{
    if (!s_appletHandle) {
        s_appletHandle = new PanelAppletHandle();
    }

    ++s_appletHandleCount;

    connect(s_appletHandle, SIGNAL(mousePressed(Plasma::Applet*,QMouseEvent*)), 
            this, SLOT(handleMousePressed(Plasma::Applet*,QMouseEvent*)));
    connect(s_appletHandle, SIGNAL(mouseMoved(Plasma::Applet*,QMouseEvent*)), 
            this, SLOT(handleMouseMoved(Plasma::Applet*,QMouseEvent*)));
    connect(s_appletHandle, SIGNAL(mouseReleased(Plasma::Applet*,QMouseEvent*)), 
            this, SLOT(handleMouseReleased(Plasma::Applet*,QMouseEvent*)));

    syncIndex();
    syncOrientation();
    syncGeometry();
    setMouseTracking(true);


    connect(m_applet, SIGNAL(destroyed(QObject*)), this, SLOT(appletDestroyed()));
    connect(m_applet, SIGNAL(geometryChanged()), this, SLOT(delaySyncGeometry()));
}

PanelAppletOverlay::~PanelAppletOverlay()
{
    bool mover = mouseGrabber() == this;
    if (mover) {
        kDebug() << "MOVER!" << m_layout << m_index;
        releaseMouse();
        if (m_layout && m_applet) {
            m_layout->insertItem(m_index, m_applet);
        }
    }

    if (m_spacer) {
        if (m_layout) {
            m_layout->removeItem(m_spacer);
        }

        m_spacer->deleteLater();
        m_spacer = 0;
    }

    --s_appletHandleCount;
    if (s_appletHandleCount < 1) {
        delete s_appletHandle;
        s_appletHandle = 0;
        s_appletHandleCount = 0;
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


    if (!m_applet) {
        return;
    }

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
    m_lastGlobalPos = event->globalPos();
    //kDebug() << m_clickDrag;
    if (m_clickDrag) {
        setMouseTracking(false);
        m_clickDrag = false;
        m_origin = QPoint();
        return;
    }

    if (!m_applet || event->button() != Qt::LeftButton) {
        return;
    }

    m_clickDrag = false;
    if (!m_spacer) {
        m_spacer = new AppletMoveSpacer(m_applet);
    } else if (m_layout) {
        m_layout->removeItem(m_spacer);
    }

    m_origin = mapToParent(event->pos());
    m_spacer->setMinimumSize(m_applet->geometry().size());
    m_spacer->setMaximumSize(m_applet->geometry().size());
    if (m_layout) {
        m_layout->removeItem(m_applet);
        m_layout->insertItem(m_index, m_spacer);
    }
    m_applet->raise();

    if (m_orientation == Qt::Horizontal) {
        m_offset = geometry().x() - m_origin.x();
    } else {
        m_offset = geometry().y() - m_origin.y();
    }

    m_dragAction = Move;

    const int margin = 9;
    if (m_applet->inherits("PanelSpacer")) {
        if (m_applet->formFactor() == Plasma::Horizontal) {
            if (event->pos().x() < margin) {
                m_dragAction = LeftResize;
            } else if (event->pos().x() > m_applet->size().width() - margin) {
                m_dragAction = RightResize;
            }
        } else if (m_applet->formFactor() == Plasma::Vertical) {
            if (event->pos().y() < margin) {
                m_dragAction = LeftResize;
            } else if (event->pos().y() > m_applet->size().height() - margin) {
                m_dragAction = RightResize;
            }
        }
    }
}

void PanelAppletOverlay::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_layout || !m_applet) {
        //kDebug() << "no layout";
        return;
    }

    const int margin = 9;
    if (m_applet->inherits("PanelSpacer")) {
        if (m_applet->formFactor() == Plasma::Horizontal) {
            if (event->pos().x() < margin || event->pos().x() > m_applet->size().width() - margin) {
                setCursor(Qt::SizeHorCursor);
            } else {
                setCursor(Qt::ArrowCursor);
            }
        } else if (m_applet->formFactor() == Plasma::Vertical) {
            if (event->pos().y() < margin || event->pos().y() > m_applet->size().height() - margin) {
                setCursor(Qt::SizeVerCursor);
            } else {
                setCursor(Qt::ArrowCursor);
            }
        }
    }

    if (!m_clickDrag && !(event->buttons() & Qt::LeftButton)) {
        //kDebug() << "no left button and we aren't click dragging";
        return;
    }

    Plasma::FormFactor f = m_applet->formFactor();


    if (!m_applet->inherits("PanelSpacer") &&
          (((f != Plasma::Horizontal && f != Plasma::Vertical) && rect().intersects(m_applet->rect().toRect())) ||
          ((f == Plasma::Horizontal || f == Plasma::Vertical) && !rect().contains(event->globalPos()))) ) {
        Plasma::View *view = Plasma::View::topLevelViewAt(event->globalPos());
        //kDebug() << "checking view" << view << m_applet->view();

        if (!view) {
            view = dynamic_cast<Plasma::View*>(parent());
        }

        if (!view) {
            return;
        }

        QPointF pos = view->mapFromGlobal(event->globalPos());
        if (view != m_applet->view() && (event->buttons() & Qt::LeftButton)) {
            Plasma::Containment *c = view->containment();
            if (!c) {
                return;
            }

            syncOrientation();
            syncGeometry();

            if (m_spacer) {
                if (m_layout) {
                    m_layout->removeItem(m_spacer);
                }
                m_spacer->deleteLater();
                m_spacer = 0;
            }

            QPointF pos = c->view()->mapFromGlobal(event->globalPos());
            QRectF g = m_applet->geometry();
            pos += QPoint(m_offset, m_offset);
            g.moveTo(pos);
            m_applet->setGeometry(g);
            m_layout = 0;
            c->addApplet(m_applet, pos, true);
            m_applet->flushPendingConstraintsEvents();
            m_applet->setPos(pos);
            releaseMouse();
            emit moved(this);
            return;
        }
    } else if (m_applet->inherits("PanelSpacer") && m_dragAction != Move) {
        if (m_applet->formFactor() == Plasma::Horizontal) {
            if (m_dragAction == LeftResize) {
                int fixedWidth = m_applet->size().width()+(m_lastGlobalPos.x() - event->globalPos().x());
                m_applet->setPos(m_applet->pos().x()-(fixedWidth-m_applet->size().width()), m_applet->pos().y());
                m_applet->setMinimumWidth(fixedWidth);
                m_applet->setMaximumWidth(fixedWidth);
            } else if (m_dragAction == RightResize) {
                int fixedWidth = m_applet->size().width()-(m_lastGlobalPos.x() - event->globalPos().x());
                m_applet->setMinimumWidth(fixedWidth);
                m_applet->setMaximumWidth(fixedWidth);
            }
        } else if (m_applet->formFactor() == Plasma::Vertical) {
            if (m_dragAction == LeftResize) {
                int fixedHeight = m_applet->size().height()+(m_lastGlobalPos.y() - event->globalPos().y());
                m_applet->setPos(m_applet->pos().x(), m_applet->pos().y()-(fixedHeight-m_applet->size().height()));
                m_applet->setMinimumHeight(fixedHeight);
                m_applet->setMaximumHeight(fixedHeight);
            } else if (m_dragAction == RightResize) {
                int fixedHeight = m_applet->size().height()-(m_lastGlobalPos.y() - event->globalPos().y());
                m_applet->setMinimumHeight(fixedHeight);
                m_applet->setMaximumHeight(fixedHeight);
            }
        }
        m_lastGlobalPos = event->globalPos();
        return;
    }

    if (!m_spacer) {
        m_spacer = new AppletMoveSpacer(m_applet);
        m_spacer->setMinimumSize(m_applet->geometry().size());
        m_spacer->setMaximumSize(m_applet->geometry().size());
        if (m_layout) {
            m_layout->removeItem(m_applet);
            m_layout->insertItem(m_index, m_spacer);
        }
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

    //FIXME: assumption on how panel containment works, presence of a non applet spacer in last position (if they were swapped would be impossible to save and restore)
    if ((m_index > 0 && m_layout->itemAt(m_index - 1)) || m_index == 0) {
        const bool prevIsApplet = dynamic_cast<Plasma::Applet*>(m_layout->itemAt(m_index - 1)) != 0;
        const bool nextIsApplet = dynamic_cast<Plasma::Applet*>(m_layout->itemAt(m_index + 1)) != 0;

        QPointF mousePos = event->pos() + g.topLeft();

        // swap items if we pass completely over the next/previous item or cross
        // more than halfway across it, whichever comes first
        if (m_orientation == Qt::Horizontal) {
            //kDebug() << prevIsApplet << m_prevGeom << g << nextIsApplet << m_nextGeom;
            if (QApplication::layoutDirection() == Qt::RightToLeft) {
                if (prevIsApplet && m_prevGeom.isValid() && mousePos.x() >= m_prevGeom.right()) {
                    swapWithPrevious();
                } else if (nextIsApplet && m_nextGeom.isValid() && mousePos.x() <= m_nextGeom.left()) {
                    swapWithNext();
                }
            } else if (prevIsApplet && m_prevGeom.isValid() && mousePos.x() <= m_prevGeom.left()) {
                swapWithPrevious();
            } else if (nextIsApplet && m_nextGeom.isValid() && mousePos.x() >= m_nextGeom.right()) {
                swapWithNext();
            }

        } else if (prevIsApplet && m_prevGeom.isValid() && mousePos.y() <= m_prevGeom.top()) {
            swapWithPrevious();
        } else if (nextIsApplet && m_nextGeom.isValid() && mousePos.y() >= m_nextGeom.bottom()) {
            swapWithNext();
        }
    }

    m_lastGlobalPos = event->globalPos();
    //kDebug() << "=================================";
}

void PanelAppletOverlay::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    if (!m_spacer || !m_applet) {
        //kDebug() << "releasing as we don't have a spacer";
        releaseMouse();
        setMouseTracking(false);
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
            grabMouse();
            setMouseTracking(true);
            event->setAccepted(false);
            return;
        }
    }

    releaseMouse();
    //kDebug() << "released mouse";
    if (m_layout) {
        m_layout->removeItem(m_spacer);
    }

    m_spacer->deleteLater();
    m_spacer = 0;

    if (m_layout) {
        m_layout->insertItem(m_index, m_applet);
    }

    m_applet->setZValue(m_applet->zValue() - 1);
}

void PanelAppletOverlay::handleMousePressed(Plasma::Applet *applet, QMouseEvent *event)
{
    if (applet == m_applet) {
        QMouseEvent ownEvent(event->type(), mapFromGlobal(event->globalPos()), event->globalPos(), event->button(), event->buttons(), event->modifiers());
        mousePressEvent(&ownEvent);
    }
}

void PanelAppletOverlay::handleMouseMoved(Plasma::Applet *applet, QMouseEvent *event)
{
    if (applet == m_applet) {
        QMouseEvent ownEvent(event->type(), mapFromGlobal(event->globalPos()), event->globalPos(), event->button(), event->buttons(), event->modifiers());
        mouseMoveEvent(&ownEvent);
    }
}

void PanelAppletOverlay::handleMouseReleased(Plasma::Applet *applet, QMouseEvent *event)
{
    if (applet == m_applet) {
        QMouseEvent ownEvent(event->type(), mapFromGlobal(event->globalPos()), event->globalPos(), event->button(), event->buttons(), event->modifiers());
        mouseReleaseEvent(&ownEvent);
    }
}

void PanelAppletOverlay::contextMenuEvent(QContextMenuEvent *event)
{
    if (m_applet) {
        Plasma::Containment *c = m_applet->containment();
        if (c) {
            c->showContextMenu(mapToParent(event->pos()), event->globalPos());
        }
    }
}

void PanelAppletOverlay::enterEvent(QEvent *event)
{
    Q_UNUSED(event)
    update();
    s_appletHandle->setApplet(m_applet);
}

void PanelAppletOverlay::leaveEvent(QEvent *event)
{
    setCursor(Qt::ArrowCursor);
    Q_UNUSED(event)
    s_appletHandle->startHideTimeout();
    update();
}

void PanelAppletOverlay::swapWithPrevious()
{
    if (!m_layout) {
        return;
    }

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
    emit moved(this);
}

void PanelAppletOverlay::swapWithNext()
{
    if (!m_layout) {
        return;
    }

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
    emit moved(this);
}

void PanelAppletOverlay::appletDestroyed()
{
    m_applet = 0;
    emit removedWithApplet(this);
    deleteLater();
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
    if (!m_layout || !m_applet) {
        return;
    }

    //kDebug();
    setGeometry(m_applet->geometry().toRect());

    if (m_index > 0 && m_layout->itemAt(m_index - 1)) {
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

    if (m_applet->containment() && m_applet->containment()->corona()) {
        s_appletHandle->move(m_applet->containment()->corona()->popupPosition(m_applet, s_appletHandle->size(), Qt::AlignCenter));
    }
}

void PanelAppletOverlay::syncIndex()
{
    if (!m_layout || !m_applet) {
        m_index = -1;
        return;
    }

    for (int i = 0; i < m_layout->count(); ++i) {
        QGraphicsWidget *w = dynamic_cast<QGraphicsWidget*>(m_layout->itemAt(i));
        if (w == m_applet) {
            m_index = i;
            break;
        }
    }
}

void PanelAppletOverlay::syncOrientation()
{
    if (!m_applet) {
        return;
    }

    m_orientation = m_applet->formFactor() == Plasma::Horizontal ? Qt::Horizontal : Qt::Vertical;
}

Plasma::Applet *PanelAppletOverlay::applet() const
{
    return m_applet;
}

#include "panelappletoverlay.moc"

