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

#include <QSvgRenderer>

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

void Clock::updated(const Plasma::DataEngine::Data &data)
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
    p->setRenderHint(QPainter::SmoothPixmapTransform);

    qreal seconds = 6.0 * m_time.second();
    qreal minutes = 6.0 * m_time.minute();
    qreal hours = 30.0 * m_time.hour();

    QRectF rrr(0, 0, 0, 0);
    QRectF boundRect = boundingRect();
    QSizeF clockSize = m_theme->elementSize("ClockFace");
    m_theme->paint(p, boundRect, "ClockFace");
    QSize elementSize;

    p->save();
      p->translate(clockSize.height()/2, clockSize.width()/2);
      p->rotate(seconds-90);
      elementSize = m_theme->elementSize("SecondHand");
      m_theme->resize(elementSize);
      rrr.setSize(elementSize);
      m_theme->paint(p, rrr, "SecondHand");
//     p->drawRect(rrr);
    p->restore();

    elementSize = QSize(0, 0);

    p->save();
      p->translate(clockSize.height()/2, clockSize.width()/2);
      p->rotate(hours-90);
#warning FIXME: Need to uncomment these 3 linse, but (dunno why) if I calculate the size, I get 0 witdth.
//     elementSize = m_theme->elementSize("HourHand");
//     m_theme->resize(elementSize);
//     rrr.setSize(elementSize);
      m_theme->paint(p, rrr, "HourHand");
    p->restore();

    elementSize = QSize(0, 0);

    p->save();
      p->translate(clockSize.height()/2, clockSize.width()/2);
      p->rotate(minutes-90);
//       elementSize = m_theme->elementSize("MinuteHand");
//       m_theme->resize(elementSize);
//       rrr.setSize(elementSize);
      m_theme->paint(p, rrr, "MinuteHand");
//     p->drawRect(rrr);
    p->restore();

//FIXME: re-enable this.
//     elementSize = QSize(0, 0);
// 
//     p->save();
//     p->translate(clockSize.width() / 2, clockSize.width() / 2);
//     elementSize = m_theme->elementSize("HandCenterScrew");
//     m_theme->resize(elementSize);
//     rrr.setSize(elementSize);
//     m_theme->paint(p, 0, 0, "HandCenterScrew");
//     p->restore();
    p->drawText(boundingRect().width()/2, (boundingRect().height()/2)+32, m_time.toString());
    m_theme->resize(boundingRect().size());
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
