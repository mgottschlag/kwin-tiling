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

#include "clock.h"

#include <math.h>

#include <QApplication>
#include <QBitmap>
#include <QGraphicsScene>
#include <QMatrix>
#include <QPaintEvent>
#include <QPainter>
#include <QPixmap>
#include <QStyleOptionGraphicsItem>

#include <KDebug>
#include <KLocale>

#include <svg.h>
#include <interface.h>

using namespace Plasma;

Clock::Clock(QGraphicsItem * parent,
             int appletId)
    : Plasma::Applet(parent, "plasma-clock-default", appletId)
{
    setFlags(QGraphicsItem::ItemIsMovable); // | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable);
    Plasma::DataEngine* timeEngine = Plasma::Interface::self()->loadDataEngine("time");
    if (timeEngine) {
        timeEngine->connectSource("time", this);
    }

    m_theme = new Plasma::Svg("widgets/clock", this);
    m_theme->resize();
}

QRectF Clock::boundingRect() const
{
    //FIXME: this needs to be settable / adjustable
    QSize s = m_theme->size();
    return QRectF(0, 0, s.width(), s.height());
}

void Clock::updated(const QString& source, const Plasma::DataEngine::Data &data)
{
    Q_UNUSED(source)
    m_time = data[i18n("Local")].toTime();
    QGraphicsItem::update();
}

Clock::~Clock()
{
    Plasma::Interface::self()->unloadDataEngine("time");
}

void Clock::paint(QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget)
    Q_UNUSED(option)
    p->setRenderHint(QPainter::SmoothPixmapTransform);

    qreal seconds = 6.0 * m_time.second() - 180;
    qreal minutes = 6.0 * m_time.minute() - 180;
    qreal hours = 30.0 * m_time.hour() - 180 + ((m_time.minute() / 59.0) * 30.0);

    QRectF tempRect(0, 0, 0, 0);
    QRectF boundRect = boundingRect();
    QSizeF boundSize = boundRect.size();
    //QSizeF clockSize = m_theme->elementSize("ClockFace");
    //kDebug() << "painting clock face at " << boundRect << endl;
    m_theme->paint(p, boundRect, "ClockFace");

    p->save();
    p->translate(boundSize.width()/2, boundSize.height()/2);
    p->rotate(seconds);
    QSize elementSize = m_theme->elementSize("SecondHand");
    p->translate(-elementSize.width()/2, -5.5); //hardcoded since this is the difference between the centre of the clockface and the y-zero-position of the SVG element
    m_theme->resize(elementSize);
    tempRect.setSize(elementSize);
    m_theme->paint(p, tempRect, "SecondHand");
    p->restore();

    p->save();
    p->translate(boundSize.width()/2, boundSize.height()/2);
    p->rotate(hours);
    m_theme->resize(boundSize);
    elementSize = m_theme->elementSize("HourHand");
    p->translate(-elementSize.width()/2, -3.75);
    m_theme->resize(elementSize);
    tempRect.setSize(elementSize);
    m_theme->paint(p, tempRect, "HourHand");
    p->restore();

    p->save();
    p->translate(boundSize.width()/2, boundSize.height()/2);
    p->rotate(minutes);
    m_theme->resize(boundSize);
    elementSize = m_theme->elementSize("MinuteHand");
    p->translate(-elementSize.width()/2, -2.5);
    m_theme->resize(elementSize);
    tempRect.setSize(elementSize);
    m_theme->paint(p, tempRect, "MinuteHand");
    p->restore();

    p->save();
    m_theme->resize(boundSize);
    elementSize = m_theme->elementSize("HandCenterScrew");
    m_theme->resize(elementSize);
    tempRect.setSize(elementSize);
    p->translate(boundSize.width() / 2 - elementSize.width() / 2, boundSize.height() / 2 - elementSize.height() / 2);
    m_theme->paint(p, tempRect, "HandCenterScrew");
    p->restore();

    //FIXME: temporary time output
    QString time = m_time.toString();
    QFontMetrics fm(QApplication::font());
    p->drawText(static_cast<int>(boundRect.width()/2 - fm.width(time) / 2),
                static_cast<int>((boundRect.height()/2) - fm.xHeight()*3), m_time.toString());

    m_theme->resize(boundSize);
    m_theme->paint(p, boundRect, "Glass");
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
