/***************************************************************************
 *   Copyright 2009 by Aaron Seigo <aseigo@kde.org>                        *
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

#include "selectionbar.h"

#include <QGraphicsScene>
#include <QStyleOptionGraphicsItem>
#include <QTimer>

#include <KDebug>

#include <Plasma/Animator>
#include <Plasma/FrameSvg>

#include "resultitem.h"

SelectionBar::SelectionBar(QGraphicsWidget *parent)
    : QGraphicsWidget(parent),
      m_frame(new Plasma::FrameSvg(this)),
      m_animId(0),
      m_target(0)
{
    setCacheMode(DeviceCoordinateCache);
    setZValue(-1000);
    setFlag(ItemIsMovable, false);
    setFlag(ItemIsSelectable, false);
    setFlag(ItemIsFocusable, false);

    m_hideTimer = new QTimer(this);
    m_hideTimer->setInterval(100);
    m_hideTimer->setSingleShot(true);
    connect(m_hideTimer, SIGNAL(timeout()), this, SLOT(disappear()));

    m_frame->setImagePath("widgets/viewitem");
    m_frame->setCacheAllRenderedFrames(true);
    m_frame->setElementPrefix("hover");

    connect(m_frame, SIGNAL(repaintNeeded()), this, SLOT(frameSvgChanged()));
    connect(Plasma::Animator::self(), SIGNAL(customAnimationFinished(int)),
            this, SLOT(movementFinished(int)));
}

void SelectionBar::getMargins(qreal &left, qreal &top, qreal &right, qreal &bottom) const
{
    return m_frame->getMargins(left, top, right, bottom);
}

void SelectionBar::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget)
    m_frame->paintFrame(painter, option->exposedRect, option->exposedRect);
}

void SelectionBar::animateAndCenter(qreal t)
{
    setPos(m_animStartRect.topLeft()*(1-t)+m_animEndRect.topLeft()*t);
    if (m_target && !m_target->mouseHovered()) emit ensureVisibility(this);
}

void SelectionBar::acquireTarget()
{
    if (m_target) {
        disconnect(m_target, SIGNAL(destroyed(QObject*)), this, SLOT(targetDestroyed()));
        m_target->removeSceneEventFilter(this);
    }

    m_target = 0;
    QList<QGraphicsItem *> selection = scene()->selectedItems();

    if (selection.count() != 1) {
        return;
    }

    m_target = dynamic_cast<ResultItem *>(selection.first());

    if (m_target) {
        connect(m_target, SIGNAL(destroyed(QObject*)), this, SLOT(targetDestroyed()));
        m_target->installSceneEventFilter(this);
    }
}

void SelectionBar::movementFinished(int id)
{
    if (id != m_animId) {
        return;
    }

    m_animId = 0;

    if (m_target) {
        resize(m_target->size());
    }
}

void SelectionBar::frameSvgChanged()
{
    update();
    emit graphicsChanged();
}

void SelectionBar::disappear()
{
    hide();
}

void SelectionBar::targetDestroyed()
{
    m_target = 0;
}

void SelectionBar::itemSelected()
{
    if (m_animId) {
        Plasma::Animator::self()->stopCustomAnimation(m_animId);
    }

    acquireTarget();

    if (!m_target) {
        //TODO: animate the hide
        m_hideTimer->start();
        return;
    }

    m_hideTimer->stop();

    QRectF rect(m_target->geometry());

    if (!isVisible()) {
        resize(rect.size());
        setPos(rect.topLeft());
        show();
    } else {
        m_animStartRect = geometry();
        m_animEndRect = rect;
        m_animId = Plasma::Animator::self()->customAnimation(ANIM_FRAMES,ANIM_DURATION, Plasma::Animator::EaseInCurve, this, "animateAndCenter");
    }
}

QVariant SelectionBar::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    switch (change) {
        case ItemSceneChange: {
            if (scene()) {
                disconnect(scene(), SIGNAL(selectionChanged()), this, SLOT(itemSelected()));
            }

            QGraphicsScene *newScene = value.value<QGraphicsScene*>();
            if (newScene) {
                connect(newScene, SIGNAL(selectionChanged()), this, SLOT(itemSelected()));
            }
        }
        break;

        default:
        break;
    }

    return QGraphicsWidget::itemChange(change, value);
}

bool SelectionBar::sceneEventFilter(QGraphicsItem *watched, QEvent *event)
{
    if (m_target == watched) {
        switch (event->type()) {
            case QEvent::GraphicsSceneResize: {
                QResizeEvent *resizeEvent = static_cast<QResizeEvent*>(event);
                //kDebug() << "resizing to" << resizeEvent->oldSize() << resizeEvent->size();
                resize(resizeEvent->size());
            }
            break;

            case QEvent::GraphicsSceneMove:
                setPos(m_target->pos());
            break;

            default:
            break;
        }
    }

    return QGraphicsWidget::sceneEventFilter(watched, event);
}

void SelectionBar::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    Q_UNUSED(event)
    m_frame->resizeFrame(rect().size());
    update();
}

#include <selectionbar.moc>

