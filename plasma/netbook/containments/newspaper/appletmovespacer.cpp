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

#include "appletmovespacer.h"

#include <QGraphicsSceneDragDropEvent>

#include <Plasma/FrameSvg>

AppletMoveSpacer::AppletMoveSpacer(QGraphicsWidget *parent)
    : QGraphicsWidget(parent)
{
    m_background = new Plasma::FrameSvg(this);
    m_background->setImagePath("widgets/frame");
    m_background->setElementPrefix("sunken");
}

AppletMoveSpacer::~AppletMoveSpacer()
{
}

void AppletMoveSpacer::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    event->setPos(mapToParent(event->pos()));
    emit dropRequested(event);
}

void AppletMoveSpacer::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    m_background->resizeFrame(event->newSize());
}

void AppletMoveSpacer::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    m_background->paintFrame(painter);
}

#include "appletmovespacer.moc"
