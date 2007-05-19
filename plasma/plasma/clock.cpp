/***************************************************************************
 *   Copyright (C) 2005,2006,2007 by Siraj Razick                          *
 *   siraj@kdemail.net                                                     *
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

#include <math.h>

#include <QMatrix>
#include <QPixmap>
#include <QPaintEvent>
#include <QBitmap>
#include <QPainter>
#include <QGraphicsScene>
#include <QStyleOptionGraphicsItem>

#include <KDebug>
#include <KLocale>

#include "svg.h"
#include "interface.h"

#include "clock.h"

Clock::Clock(QGraphicsItem * parent)
    : Plasma::DataVisualization(),
      QGraphicsItem(parent)
{
    setFlags(QGraphicsItem::ItemIsMovable); // | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable);

    Plasma::DataEngine* timeEngine = Plasma::Interface::self()->loadDataEngine("time");
    if (timeEngine) {
        timeEngine->connectSource("time", this);
    }

    m_theme = new Plasma::Svg("widgets/clock", this);
    m_theme->resize(300, 300);
}

QRectF Clock::boundingRect() const
{
    //FIXME: this needs to be settable / adjustable
    return QRectF(0, 0, 300, 300);
}

void Clock::updated(const Plasma::DataSource::Data &data)
{
    m_time = data[i18n("Local")].toTime();
    update();
}

Clock::~Clock()
{
    Plasma::Interface::self()->unloadDataEngine("time");
}

void Clock::paint(QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget)

    qreal seconds = 6.0 * m_time.second();
    qreal minutes = 6.0 * m_time.minute();
    qreal hours = 30.0 * m_time.hour();

    QRectF boundRect = boundingRect();

    QRectF r = option->exposedRect;
    p->setRenderHint(QPainter::SmoothPixmapTransform);

    QSizeF clockSize = m_theme->elementSize("ClockFace");
    m_theme->paint(p, boundRect, "ClockFace");
/*
    p->save();
    p->translate(clockSize.width() / 2, clockSize.height() / 2);
    p->rotate(hours);
    QSizeF elementSize = m_theme->elementSize("HourHand");
    QRectF rect = QRectF(-(elementSize.width() / 2), -(elementSize.height() - 32),
                         elementSize.width(), elementSize.height());
    m_theme->paint(p, rect, "HourHand");
    p->restore();

    p->save();
    p->translate(clockSize.width() / 2, clockSize.height() / 2);
    p->rotate(minutes);
    elementSize = m_theme->elementSize("MinuteHand");
    rect = QRectF(-(elementSize.width() / 2),-(elementSize.height() - 16),
                  elementSize.width(), elementSize.height());
    m_theme->paint(p, rect, "MinuteHand");
    p->restore();

    p->save();
    p->translate(clockSize.width() / 2, clockSize.width() / 2);
    p->rotate(seconds);
    elementSize = m_theme->elementSize("SecondHand");
    rect = QRectF(-(elementSize.width() / 2),-(elementSize.height() - 16),
                  elementSize.width(), elementSize.height());
    m_theme->paint(p, rect, "SecondHand");
    p->restore();

    p->save();
    p->translate(clockSize.width() / 2, clockSize.width() / 2);
    m_theme->paint(p, 0, 0, "HandCenterScrew");
    p->restore();*/

    m_theme->paint(p, boundRect, "Glass");
    p->drawText(100, 100, m_time.toString());
//     p->drawRect(boundRect);
}
/*
QVariant Clock::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemPositionChange && scene())
    {
        QPointF pos = value.toPointF();
        QRectF sceneRect = scene()->sceneRect();
        if (!sceneRect.contains(pos))
        {
            pos.setX(qMin(sceneRect.right(), qMax(pos.x(), sceneRect.left())));
            pos.setY(qMin(sceneRect.bottom(), qMax(pos.y(), sceneRect.top())));
            return pos;
        }
    }

    return QGraphicsItem::itemChange(change, value);
}*/

#include "clock.moc"
