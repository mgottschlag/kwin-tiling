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

protected:
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
      m_spacerIndex(0)
{
    setAcceptHoverEvents(true);
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

    //FIXME: Plasma::Theme
    painter->fillRect(option->exposedRect, QColor(0,0,0,30));

    if (m_applet) {
        QRectF geom = m_applet->geometry();
        //FIXME: calculate the offset ONE time, mmkay?
        QPointF offset = m_newspaper->m_mainWidget->pos() + m_newspaper->m_scrollWidget->pos();
        geom.moveTopLeft(geom.topLeft() + offset);
        painter->fillRect(geom, QColor(0,0,0,70));
    }
}

void AppletOverlay::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_applet) {
        QPointF offset = m_newspaper->m_mainWidget->pos() + m_newspaper->m_scrollWidget->pos();
        showSpacer(event->pos());
        if (m_spacerLayout) {
            m_spacerLayout->removeItem(m_applet);
        }
        if (m_spacer) {
            m_spacer->setMinimumHeight(m_applet->size().height());
        }
    }
}

void AppletOverlay::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    QPointF offset = m_newspaper->m_mainWidget->pos() + m_newspaper->m_scrollWidget->pos();
    m_applet = 0;

    Plasma::Applet *oldApplet;

    //FIXME: is there a way more efficient than this linear one? scene()itemAt() won't work because it would always be == this
    foreach (Plasma::Applet *applet, m_newspaper->applets()) {
        if (applet->geometry().contains(event->pos()-offset)) {
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

    if (m_spacer && m_spacerLayout) {
        m_spacerLayout->removeItem(m_spacer);
        m_spacerLayout->insertItem(m_spacerIndex, m_applet);
    }

    delete m_spacer;
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

    if (m_newspaper->m_leftLayout->geometry().contains(translatedPos)) {
        lay = m_newspaper->m_leftLayout;
    } else if (m_newspaper->m_rightLayout->geometry().contains(translatedPos)) {
        lay = m_newspaper->m_rightLayout;
    } else {
        m_spacerLayout = 0;
        return;
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

    m_spacerIndex = insertIndex;
    if (insertIndex != -1) {
        if (!m_spacer) {
            m_spacer = new AppletMoveSpacer(this);
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
            kWarning()<<m_newspaper->m_mainWidget->geometry();
            m_newspaper->m_mainWidget->moveBy(0, 5);
            m_applet->moveBy(0, -5);
        }
    }
}

#include <appletoverlay.moc>

