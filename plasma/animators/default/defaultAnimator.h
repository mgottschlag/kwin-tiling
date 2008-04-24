/*
 *   Copyright 2007 Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
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

#ifndef DEFAULTANIMATOR_H
#define DEFAULTANIMATOR_H

#include <plasma/animator.h>

class DefaultAnimator : public Plasma::Animator
{
    Q_OBJECT

public:
    explicit DefaultAnimator(QObject *parent = 0, const QVariantList& list = QVariantList());

    int animationFPS(Plasma::Phase::Animation animation) const;
    int elementAnimationFPS(Plasma::Phase::ElementAnimation animation) const;

    void itemAppear(qreal progress, QGraphicsItem* item);
    void itemDisappear(qreal progress, QGraphicsItem* item);


    QPixmap elementAppear(qreal progress, const QPixmap& pixmap);
    QPixmap elementDisappear(qreal progress, const QPixmap& pixmap);
};

K_EXPORT_PLASMA_ANIMATOR(default, DefaultAnimator)

#endif // multiple inclusion guard

