/*
 *   Copyright (C) 2007 Petri Damsten <damu@iki.fi>
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

#include "monitorbutton.h"
#include <KDebug>
#include <KIcon>
#include <QPainter>
#include <QIcon>
#include <KPushButton>
#define MARGIN 2

class MonitorButton::Private
{
    public:
        Private() : imageSize(32, 32) { }

        QSizeF imageSize;
        QString image;
};

MonitorButton::MonitorButton(QGraphicsWidget *parent) :
        Plasma::PushButton(parent),
        d(new Private)
{
   setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
   setPreferredSize(d->imageSize.width() + 2 * MARGIN, d->imageSize.height() + 2 * MARGIN);
}

MonitorButton::~MonitorButton()
{
    delete d;
}

QString MonitorButton::image() const
{
    return d->image;
}

void MonitorButton::setImage(const QString &image)
{
    d->image = image;
    update();
}

void MonitorButton::paint(QPainter *p,
                          const QStyleOptionGraphicsItem *option,
                          QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    QIcon::Mode mode = QIcon::Normal;
    if (nativeWidget()->isChecked()) {
        mode = QIcon::Disabled;
    }
    p->drawPixmap(QPointF((size().width() - d->imageSize.width()) / 2,
                        (size().height() - d->imageSize.height()) / 2),
                  KIcon(d->image).pixmap(d->imageSize.toSize(), mode));
}

#include "monitorbutton.moc"
