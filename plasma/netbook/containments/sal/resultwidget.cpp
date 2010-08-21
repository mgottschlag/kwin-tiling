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

#include "resultwidget.h"

#include <QGraphicsSceneMouseEvent>
#include <QPropertyAnimation>

ResultWidget::ResultWidget(QGraphicsItem *parent)
   : Plasma::IconWidget(parent),
     m_shouldBeVisible(true)
{
    m_animation = new QPropertyAnimation(this, "pos", this);
    m_animation->setEasingCurve(QEasingCurve::InOutQuad);
    m_animation->setDuration(250);
    connect(m_animation, SIGNAL(finished()), this, SLOT(animationFinished()));
}

ResultWidget::~ResultWidget()
{
}

void ResultWidget::animateHide()
{
    m_shouldBeVisible = false;
    QGraphicsItem *parent = parentItem();
    if (parent) {
        animatePos(QPoint(parent->boundingRect().center().x(), parent->boundingRect().bottom()));
    }
}

void ResultWidget::animatePos(const QPointF &point)
{
    m_animation->stop();
    m_animation->setStartValue(pos());
    m_animation->setEndValue(point);
    m_animation->start();
}

void ResultWidget::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    Plasma::IconWidget::mousePressEvent(event);
}

void ResultWidget::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    const int distance = QPointF(boundingRect().center() - event->pos()).manhattanLength();

    //arbitrary drag distance: this has to be way more than the usual:
    //double of the average of width and height
    if (distance > ((size().width() + size().height())/2)*2) {
        emit dragStartRequested(this);
    }

    Plasma::IconWidget::mouseMoveEvent(event);
}

void ResultWidget::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Plasma::IconWidget::mouseReleaseEvent(event);
}

void ResultWidget::animationFinished()
{
    setVisible(m_shouldBeVisible);
}

QVariant ResultWidget::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == QGraphicsItem::ItemVisibleChange) {
        m_shouldBeVisible = value.toBool();
    }

    return QGraphicsWidget::itemChange(change, value);
}


#include "resultwidget.moc"
