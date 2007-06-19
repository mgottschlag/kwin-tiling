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
#include <QPainter>

#include <KDebug>

DefaultAnimator::DefaultAnimator(QObject *parent, const QStringList& list)
    : Plasma::Animator(parent, list)
{
    Q_UNUSED(list)
}

int DefaultAnimator::frameCount(Plasma::Phase::Animation animation)
{
    switch (animation) {
        case Plasma::Phase::Appear:
            return 12;

        default:
            return 0;
    }
}

int DefaultAnimator::elementFrameCount(Plasma::Phase::ElementAnimation animation)
{
    switch (animation) {
        case Plasma::Phase::ElementAppear:
            return 12;
        case Plasma::Phase::ElementDisappear:
            return 12;

        default:
            return 0;
    }
}

void DefaultAnimator::appear(qreal progress, QGraphicsItem* item)
{
    //kDebug() << "DefaultAnimator::appear(" << progress << ", " << item << ")" << endl;
    item->resetTransform();
    item->scale(progress, progress);
}

void DefaultAnimator::appearCompleted(QGraphicsItem* item)
{
    //kDebug() << "DefaultAnimator::appearCompleted(" << item << ")" << endl;
    item->resetTransform();
}

QPixmap DefaultAnimator::elementAppear(qreal progress, const QPixmap& pixmap)
{
    QPixmap alpha(pixmap.size());
    {
        QPainter painter(&alpha);
        painter.fillRect(alpha.rect(), Qt::black);
    }

    //kDebug() << "DefaultAnimator::elementAppear(" << progress <<  ")" << endl;
    QPixmap pix(pixmap.size());
    pix.setAlphaChannel(alpha);
    QPainter painter(&pix);
    painter.setOpacity(progress);
    painter.drawPixmap(0, 0, pixmap);
    return pix;
}

QPixmap DefaultAnimator::elementDisappear(qreal progress, const QPixmap& pixmap)
{
    QPixmap alpha(pixmap.size());
    {
        QPainter painter(&alpha);
        painter.fillRect(alpha.rect(), Qt::black);
    }

    //kDebug() << "DefaultAnimator::elementDisappear(" << progress <<  ")" << endl;
    QPixmap pix(pixmap.size());
    pix.setAlphaChannel(alpha);
    QPainter painter(&pix);

    if (progress < 1) {
        painter.setOpacity(1 - progress);
        painter.drawPixmap(0, 0, pixmap);
    }
    return pix;
}

#include "defaultAnimator.moc"
