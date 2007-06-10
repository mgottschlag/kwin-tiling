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

#ifndef DEFAULTANIMATOR_H
#define DEFAULTANIMATOR_H

#include "plasma/animator.h"

class DefaultAnimator : public Plasma::Animator
{
    Q_OBJECT

public:
    explicit DefaultAnimator(QObject *parent = 0, const QStringList& list = QStringList());

    int appearFrames();
    void appear(qreal frame, QGraphicsItem* item);
    void appearCompleted(QGraphicsItem* item);

    int disappearFrames();
    void disappear(qreal frame, QGraphicsItem* item);
    void disappearCompleted(QGraphicsItem* item);

    int frameAppearFrames();
    void frameAppear(qreal frame, QGraphicsItem* item, const QRegion& drawable);
    void frameAppearCompleted(QGraphicsItem* item, const QRegion& drawable);

    int activateFrames();
    void activate(qreal frame, QGraphicsItem* item);
    void activateCompleted(QGraphicsItem* item);

    void renderBackground(QImage& background);
};

K_EXPORT_PLASMA_ANIMATOR(default, DefaultAnimator)

#endif // multiple inclusion guard

