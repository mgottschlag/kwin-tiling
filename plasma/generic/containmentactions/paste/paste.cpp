/*
 *   Copyright 2009 by Chani Armitage <chani@kde.org>
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

#include "paste.h"

#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneWheelEvent>

#include <KDebug>

Paste::Paste(QObject *parent, const QVariantList &args)
    : Plasma::ContainmentActions(parent, args)
{
}

void Paste::contextEvent(QEvent *event)
{
    QPointF scenePos;
    QPoint screenPos;
    switch (event->type()) {
        case QEvent::GraphicsSceneMousePress:
            event->accept();
            return;
        case QEvent::GraphicsSceneMouseRelease: {
            QGraphicsSceneMouseEvent *e = static_cast<QGraphicsSceneMouseEvent*>(event);
            scenePos = e->scenePos();
            screenPos = e->screenPos();
            break; }
        case QEvent::GraphicsSceneWheel: {
            QGraphicsSceneWheelEvent *e = static_cast<QGraphicsSceneWheelEvent*>(event);
            scenePos = e->scenePos();
            screenPos = e->screenPos();
            break; }
        default: //can't get the pos
            return;
    }
    paste(scenePos, screenPos);
}

#include "paste.moc"
