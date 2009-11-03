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

#include <QPropertyAnimation>

ResultWidget::ResultWidget(QGraphicsItem *parent)
   : Plasma::IconWidget(parent),
     m_animationLock(false)
{
    m_animation = new QPropertyAnimation(this, "animationPos", this);
    m_animation->setEasingCurve(QEasingCurve::OutBounce);
    m_animation->setDuration(250);
}

ResultWidget::~ResultWidget()
{
}

void ResultWidget::setAnimationPos(const QPointF &pos)
{
    m_animationLock = true;
    setPos(pos);
    m_animationLock = false;
}

QPointF ResultWidget::animationPos() const
{
    return pos();
}

void ResultWidget::setGeometry(const QRectF &rect)
{
    QPointF oldPos = pos();
    IconWidget::setGeometry(rect);

    if (m_animationLock) {
        return;
    }

    if (m_animation->state() == QAbstractAnimation::Running) {
        m_animation->stop();
    }

    QPointF newPos = pos();
    setPos(oldPos);
    m_animation->setEndValue(rect.topLeft());

    m_animation->start();
}
