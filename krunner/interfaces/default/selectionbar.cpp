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
      m_animId(0)
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
    connect(Plasma::Animator::self(), SIGNAL(movementFinished(QGraphicsItem*)),
            this, SLOT(movementFinished(QGraphicsItem*)));
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

ResultItem *SelectionBar::targetItem()
{
    QList<QGraphicsItem *> selection = scene()->selectedItems();

    if (selection.count() != 1) {
        return 0;
    }

    return dynamic_cast<ResultItem *>(selection.first());
}

void SelectionBar::movementFinished(QGraphicsItem *movedItem)
{
    if (movedItem != this) {
        return;
    }

    m_animId = 0;

    if (!isVisible()) {
        return;
    }

    ResultItem *item = targetItem();
    if (item) {
        QRectF rect(item->targetPos(), item->size());
        resize(rect.size());
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

void SelectionBar::itemSelected()
{
    if (m_animId) {
        Plasma::Animator::self()->stopItemMovement(m_animId);
    }

    ResultItem *item = targetItem();

    if (!item) {
        //TODO: animate the hide
        m_hideTimer->start();
        return;
    }

    m_hideTimer->stop();

    QRectF rect(item->targetPos(), item->size());

    if (!isVisible()) {
        resize(rect.size());
        setPos(rect.topLeft());
        show();
    } else {
        m_animId = Plasma::Animator::self()->moveItem(this, Plasma::Animator::SlideInMovement,
                                                      rect.topLeft().toPoint());
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

void SelectionBar::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    Q_UNUSED(event)
    m_frame->resizeFrame(rect().size());
    update();
}

#include <selectionbar.moc>

