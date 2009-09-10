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

#include "appletoverlay.h"
#include "newspaper.h"

#include <QGraphicsLinearLayout>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QTimer>

#include <KGlobalSettings>

#include <Plasma/Applet>
#include <Plasma/PaintUtils>
#include <Plasma/ScrollWidget>


class AppletMoveSpacer : public QGraphicsWidget
{
public:
    AppletMoveSpacer(QGraphicsWidget *parent)
        : QGraphicsWidget(parent)
    {
    }

    AppletOverlay *overlay;

protected:
    void dropEvent(QGraphicsSceneDragDropEvent *event)
    {
        event->setPos(mapToParent(event->pos()));
        overlay->dropEvent(event);
    }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget * widget = 0)
    {
        Q_UNUSED(option)
        Q_UNUSED(widget)

        //TODO: make this a pretty gradient?
        painter->setRenderHint(QPainter::Antialiasing);
        QPainterPath p = Plasma::PaintUtils::roundedRectangle(contentsRect().adjusted(1, 1, -2, -2), 4);
        QColor c = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
        c.setAlphaF(0.3);

        painter->fillPath(p, c);
    }

};

AppletOverlay::AppletOverlay(QGraphicsWidget *parent, Newspaper *newspaper)
    : QGraphicsWidget(parent),
      m_applet(0),
      m_newspaper(newspaper),
      m_spacer(0),
      m_spacerLayout(0),
      m_spacerIndex(0),
      m_scrollDown(false),
      m_clickDrag(false)
{
    setAcceptHoverEvents(true);
    setAcceptDrops(true);
    setZValue(900);
    m_scrollTimer = new QTimer(this);
    m_scrollTimer->setSingleShot(false);
    connect(m_scrollTimer, SIGNAL(timeout()), this, SLOT(scrollTimeout()));
}

AppletOverlay::~AppletOverlay()
{
}

void AppletOverlay::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget)

    QColor c = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
    c.setAlphaF(0.25);

    painter->fillRect(option->exposedRect, c);

    if (m_applet) {
        QRectF geom = m_applet->geometry();
        //FIXME: calculate the offset ONE time, mmkay?
        QPointF offset = m_newspaper->m_mainWidget->pos() + m_newspaper->m_scrollWidget->pos();
        //FIXME: why 4,4 (some layout margins not taken into account i suppose)?
        geom.moveTopLeft(geom.topLeft() + offset + QPoint(4,4));
        geom = geom.intersected(m_newspaper->m_scrollWidget->geometry());
        c.setAlphaF(0.3);
        QPainterPath p = Plasma::PaintUtils::roundedRectangle(geom, 4);
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->fillPath(p, c);
        painter->restore();
    }
}

void AppletOverlay::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        //Hack to make scene::itemAt() work
        int z = zValue();
        setZValue(-100);
        //FIXME:here we don't have a screen pos in the event
        m_newspaper->showContextMenu(event->pos(), event->pos().toPoint());
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
        if (m_spacerLayout) {
            m_spacerLayout->removeItem(m_applet);
            m_applet->raise();
        }
        if (m_spacer) {
            m_spacer->setMinimumHeight(m_applet->size().height());
        }
    }
}

void AppletOverlay::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    if (m_clickDrag) {
        //Cheat and pretend a mousemoveevent is arrived
        QGraphicsSceneMouseEvent me;
        me.setPos(event->pos());
        me.setLastPos(event->lastPos());
        mouseMoveEvent(&me);
        return;
    }

    QPointF offset = m_newspaper->m_mainWidget->pos() + m_newspaper->m_scrollWidget->pos();
    m_applet = 0;

    Plasma::Applet *oldApplet;

    //FIXME: is there a way more efficient than this linear one? scene()itemAt() won't work because it would always be == this
    foreach (Plasma::Applet *applet, m_newspaper->applets()) {
        if (applet->geometry().contains(event->pos()-offset)) {
            //TODO: connect to m_aplet::desroyed()
            m_applet = applet;
            break;
        }
    }
    if (m_applet != oldApplet) {
        update();
    }
}

void AppletOverlay::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_spacer) {
        QPointF delta = event->pos()-event->lastPos();
        m_applet->moveBy(delta.x(), delta.y());
        showSpacer(event->pos());
    }

    if (event->pos().y() > size().height()*0.70) {
        m_scrollTimer->start(50);
        m_scrollDown = true;
    } else if (event->pos().y() < size().height()*0.30) {
        m_scrollTimer->start(50);
        m_scrollDown = false;
    } else {
        m_scrollTimer->stop();
    }

    update();
}

void AppletOverlay::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event)

    m_scrollTimer->stop();

    QPoint delta = event->pos().toPoint() - m_origin.toPoint();
    if (m_origin != QPointF() && delta.manhattanLength() < KGlobalSettings::dndEventDelay()) {
        m_clickDrag = true;
        m_origin = QPointF();
        return;
    }

    if (m_spacer && m_spacerLayout) {
        m_spacerLayout->insertItem(m_spacerIndex, m_applet);
        m_spacerLayout->removeItem(m_spacer);
    }

    delete m_spacer;
    m_spacer = 0;
    m_spacerLayout = 0;
    m_spacerIndex = 0;
}

void AppletOverlay::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    showSpacer(event->pos());
    m_newspaper->m_dragging = true;
    event->accept();
}

void AppletOverlay::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    if (event->pos().y() > size().height()*0.70) {
        m_scrollTimer->start(50);
        m_scrollDown = true;
    } else if (event->pos().y() < size().height()*0.30) {
        m_scrollTimer->start(50);
        m_scrollDown = false;
    } else {
        m_scrollTimer->stop();
    }

    showSpacer(event->pos());
}

void AppletOverlay::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    m_newspaper->dropEvent(event);
    m_newspaper->m_dragging = false;

    if (m_spacerLayout) {
        m_spacerLayout->removeItem(m_spacer);
    }

    if (m_spacer) {
        m_spacer->deleteLater();
    }

    m_spacer = 0;
    m_spacerLayout = 0;
    m_spacerIndex = 0;
}

void AppletOverlay::showSpacer(const QPointF &pos)
{
    if (!scene()) {
        return;
    }

    QPointF translatedPos = pos - m_newspaper->m_mainWidget->pos() - m_newspaper->m_scrollWidget->pos();

    QGraphicsLinearLayout *lay;

    if ((m_newspaper->m_orientation == Qt::Horizontal && translatedPos.y() < m_newspaper->m_leftLayout->geometry().bottom()) ||
        (m_newspaper->m_orientation == Qt::Vertical && translatedPos.x() < m_newspaper->m_leftLayout->geometry().right())) {
        lay = m_newspaper->m_leftLayout;
    } else {
        lay = m_newspaper->m_rightLayout;
    }

    if (pos == QPoint()) {
        if (m_spacer) {
            lay->removeItem(m_spacer);
            m_spacer->hide();
        }
        return;
    }

    //lucky case: the spacer is already in the right position
    if (m_spacer && m_spacer->geometry().contains(translatedPos)) {
        return;
    }

    int insertIndex = -1;

    for (int i = 0; i < lay->count(); ++i) {
        QRectF siblingGeometry = lay->itemAt(i)->geometry();

        if (m_newspaper->m_orientation == Qt::Horizontal) {
            qreal middle = siblingGeometry.center().x();
            if (translatedPos.x() < middle) {
                insertIndex = i;
                break;
            } else if (translatedPos.x() <= siblingGeometry.right()) {
                insertIndex = i + 1;
                break;
            }
        } else { // Vertical
            qreal middle = siblingGeometry.center().y();

            if (translatedPos.y() < middle) {
                insertIndex = i;
                break;
            } else if (translatedPos.y() <= siblingGeometry.bottom()) {
                insertIndex = i + 1;
                break;
            }
        }
    }

    if (m_spacerLayout == lay && m_spacerIndex < insertIndex) {
        --insertIndex;
    }
    if (insertIndex >= lay->count() - 1) {
        --insertIndex;
    }

    m_spacerIndex = insertIndex;
    if (insertIndex != -1) {
        if (!m_spacer) {
            m_spacer = new AppletMoveSpacer(this);
            m_spacer->overlay = this;
        }
        if (m_spacerLayout) {
            m_spacerLayout->removeItem(m_spacer);
        }
        m_spacer->show();
        lay->insertItem(insertIndex, m_spacer);
        m_spacerLayout = lay;
    }
}

void AppletOverlay::scrollTimeout()
{
    if (!m_applet) {
        return;
    }

    if (m_scrollDown) {
        if (m_newspaper->m_mainWidget->geometry().bottom() > m_newspaper->m_scrollWidget->geometry().bottom()) {
            m_newspaper->m_mainWidget->moveBy(0, -5);
            m_applet->moveBy(0, 5);
        }
    } else {
        if (m_newspaper->m_mainWidget->pos().y() < 0) {
            m_newspaper->m_mainWidget->moveBy(0, 5);
            m_applet->moveBy(0, -5);
        }
    }
}

#include <appletoverlay.moc>

