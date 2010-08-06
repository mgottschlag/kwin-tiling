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
#include "appletscontainer.h"
#include "appletsview.h"
#include "../common/appletmovespacer.h"

#include <QGraphicsSceneMouseEvent>
#include <QGraphicsLinearLayout>
#include <QGraphicsBlurEffect>
#include <QPainter>
#include <QTimer>

#include <KGlobalSettings>
#include <KIconLoader>

#include <Plasma/Applet>
#include <Plasma/Svg>
#include <Plasma/FrameSvg>
#include <Plasma/PaintUtils>
#include <Plasma/ScrollWidget>

AppletOverlay::AppletOverlay(QGraphicsWidget *parent, Newspaper *newspaper)
    : QGraphicsWidget(parent),
      m_newspaper(newspaper),
      m_spacer(0),
      m_spacerLayout(0),
      m_spacerIndex(0),
      m_scrollDown(false),
      m_clickDrag(false)
{
    foreach(Plasma::Applet *applet, newspaper->applets()) {
        QGraphicsBlurEffect *effect = new QGraphicsBlurEffect(applet);
        effect->setBlurHints(QGraphicsBlurEffect::PerformanceHint);
        effect->setBlurRadius(0);
        applet->setGraphicsEffect(effect);
    }


    setAcceptHoverEvents(true);
    setAcceptDrops(true);
    setZValue(900);
    m_scrollTimer = new QTimer(this);
    m_scrollTimer->setSingleShot(false);
    connect(m_scrollTimer, SIGNAL(timeout()), this, SLOT(scrollTimeout()));

    m_icons = new Plasma::Svg(this);
    m_icons->setImagePath("widgets/configuration-icons");
    m_icons->setContainsMultipleImages(true);
}

AppletOverlay::~AppletOverlay()
{
    m_newspaper->m_container->setGraphicsEffect(0);
    foreach(Plasma::Applet *applet, m_newspaper->applets()) {
        applet->setGraphicsEffect(0);
    }
}

void AppletOverlay::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget)

    QColor c = QColor(0, 0, 0, 98);

    painter->fillRect(option->exposedRect, c);

    if (m_applet) {
        QRectF geom = m_applet.data()->contentsRect();
        geom.translate(m_applet.data()->pos());
        //FIXME: calculate the offset ONE time, mmkay?
        QPointF offset = m_newspaper->m_container->pos() + m_newspaper->m_scrollWidget->pos();
        geom.moveTopLeft(geom.topLeft() + offset);
        geom = geom.intersected(m_newspaper->m_scrollWidget->geometry());

        QRect iconRect(0, 0, KIconLoader::SizeLarge, KIconLoader::SizeLarge);
        iconRect.moveCenter(geom.center().toPoint());
        m_icons->paint(painter, iconRect, "move");
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
            m_spacerLayout->removeItem(m_applet.data());
            m_applet.data()->raise();
        }
        if (m_spacer) {
            m_spacer->setMinimumHeight(m_applet.data()->size().height());
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

    QPointF offset = m_newspaper->m_container->pos() + m_newspaper->m_scrollWidget->pos();


    //FIXME: is there a way more efficient than this linear one? scene()itemAt() won't work because it would always be == this
    foreach (Plasma::Applet *applet, m_newspaper->applets()) {
        if (applet->geometry().contains(event->pos()-offset)) {
            m_applet = applet;
            break;
        }
    }
}

void AppletOverlay::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (!m_applet) {
        return;
    }

    if (m_spacer) {
        QPointF delta = event->pos()-event->lastPos();
        m_applet.data()->moveBy(delta.x(), delta.y());
        showSpacer(event->pos());
    }

    if (m_newspaper->orientation() == Qt::Vertical) {
        if (m_newspaper->m_scrollWidget->pos().y() + event->pos().y() > m_newspaper->m_scrollWidget->size().height()*0.70) {
            m_scrollTimer->start(50);
            m_scrollDown = true;
        } else if (m_newspaper->m_scrollWidget->pos().y() + event->pos().y() < m_newspaper->m_scrollWidget->size().height()*0.30) {
            m_scrollTimer->start(50);
            m_scrollDown = false;
        } else {
            m_scrollTimer->stop();
        }
    } else {
        if (m_newspaper->m_scrollWidget->pos().x() + event->pos().x() > m_newspaper->m_scrollWidget->size().width()*0.70) {
            m_scrollTimer->start(50);
            m_scrollDown = true;
        } else if (m_newspaper->m_scrollWidget->pos().x() + event->pos().x() < m_newspaper->m_scrollWidget->size().width()*0.30) {
            m_scrollTimer->start(50);
            m_scrollDown = false;
        } else {
            m_scrollTimer->stop();
        }
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
        m_spacerLayout->insertItem(m_spacerIndex, m_applet.data());
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

void AppletOverlay::spacerRequestedDrop(QGraphicsSceneDragDropEvent *event)
{
    dropEvent(event);
}

void AppletOverlay::showSpacer(const QPointF &pos)
{
    if (!scene()) {
        return;
    }

    QPointF translatedPos = pos - m_newspaper->m_container->pos() - m_newspaper->m_scrollWidget->pos();

    QGraphicsLinearLayout *lay = 0;

    for (int i = 0; i < m_newspaper->m_container->count(); ++i) {
        QGraphicsLinearLayout *candidateLay = dynamic_cast<QGraphicsLinearLayout *>(m_newspaper->m_container->itemAt(i));

        //normally should never happen
        if (!candidateLay) {
            continue;
        }

        if (m_newspaper->m_orientation == Qt::Horizontal) {
            if (pos.y() < candidateLay->geometry().bottom()) {
                lay = candidateLay;
                break;
            }
        //vertical
        } else {
            if (pos.x() < candidateLay->geometry().right()) {
                lay = candidateLay;
                break;
            }
        }
    }

    //couldn't decide: is the last column empty?
    if (!lay) {
        QGraphicsLinearLayout *candidateLay = dynamic_cast<QGraphicsLinearLayout *>(m_newspaper->m_container->itemAt(m_newspaper->m_container->count()-1));

        if (candidateLay && candidateLay->count() <= 2) {
            lay = candidateLay;
        }
    }

    //give up, make a new column
    if (!lay) {
        lay = m_newspaper->m_container->addColumn();
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
    if (lay->count() > 1 && insertIndex >= lay->count() - 1) {
        --insertIndex;
    }

    m_spacerIndex = insertIndex;
    if (insertIndex != -1) {
        if (!m_spacer) {
            m_spacer = new AppletMoveSpacer(this);
            connect (m_spacer, SIGNAL(dropRequested(QGraphicsSceneDragDropEvent *)),
                     this, SLOT(spacerRequestedDrop(QGraphicsSceneDragDropEvent *)));
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

    if (m_newspaper->orientation() == Qt::Vertical) {
        if (m_scrollDown) {
            if (m_newspaper->m_container->geometry().bottom() > m_newspaper->m_scrollWidget->geometry().bottom()) {
                m_newspaper->m_container->moveBy(0, -10);
                m_applet.data()->moveBy(0, 10);
            }
        } else {
            if (m_newspaper->m_container->pos().y() < 0) {
                m_newspaper->m_container->moveBy(0, 10);
                m_applet.data()->moveBy(0, -10);
            }
        }
    } else {
        if (m_scrollDown) {
            if (m_newspaper->m_container->geometry().right() > m_newspaper->m_scrollWidget->geometry().right()) {
                m_newspaper->m_container->moveBy(-10, 0);
                m_applet.data()->moveBy(10, 0);
            }
        } else {
            if (m_newspaper->m_container->pos().x() < 0) {
                m_newspaper->m_container->moveBy(10, 0);
                m_applet.data()->moveBy(-10, 0);
            }
        }
    }
}

#include <appletoverlay.moc>

