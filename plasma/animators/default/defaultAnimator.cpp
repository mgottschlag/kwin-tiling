/*
 *   Copyright (C) 2007 Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
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

#include "defaultAnimator.h"

#include <QGraphicsItem>

#include <KDebug>

DefaultAnimator::DefaultAnimator(QObject *parent, const QStringList& list)
    : Plasma::Animator(parent, list)
{
    Q_UNUSED(list)
}

int DefaultAnimator::appearFrames()
{
    kDebug() << "DefaultAnimator::appearFrames()" << endl;
    return 12;
}

void DefaultAnimator::appear(qreal frame, QGraphicsItem* item)
{
    kDebug() << "DefaultAnimator::appear(" << frame << ", " << item << ")" << endl;
    item->resetTransform();
    item->scale(frame, frame);
    Q_UNUSED(frame)
    Q_UNUSED(item)
}

void DefaultAnimator::appearCompleted(QGraphicsItem* item)
{
    kDebug() << "DefaultAnimator::appearCompleted(" << item << ")" << endl;
    item->resetTransform();
}

int DefaultAnimator::disappearFrames()
{
    return 0;
}

void DefaultAnimator::disappear(qreal frame, QGraphicsItem* item)
{
    Q_UNUSED(frame)
    Q_UNUSED(item)
}

void DefaultAnimator::disappearCompleted(QGraphicsItem* item)
{
    Q_UNUSED(item)
}

int DefaultAnimator::activateFrames()
{
    return 0;
}

void DefaultAnimator::activate(qreal frame, QGraphicsItem* item)
{
    Q_UNUSED(frame)
    Q_UNUSED(item)
}

void DefaultAnimator::activateCompleted(QGraphicsItem* item)
{
    Q_UNUSED(item)
}

int DefaultAnimator::frameAppearFrames()
{
    return 0;
}

void DefaultAnimator::frameAppear(qreal frame, QGraphicsItem* item, const QRegion& drawable)
{
    Q_UNUSED(frame)
    Q_UNUSED(item)
    Q_UNUSED(drawable)
}

void DefaultAnimator::frameAppearCompleted(QGraphicsItem* item, const QRegion& drawable)
{
    Q_UNUSED(item)
    Q_UNUSED(drawable)
}

void DefaultAnimator::renderBackground(QImage& background)
{
    Q_UNUSED(background)
}


#include "defaultAnimator.moc"
