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
#include <QDebug>

#include "clock.h"
#include "clock.moc"

namespace Plasma
{

Clock::Clock(QGraphicsItem * parent)
    :   DataVisualization(),
        QGraphicsItem(parent)
{
    setFlags(QGraphicsItem::ItemIsMovable); // | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable);

    m_rect = QRectF(0, 0, 300, 300);

    shade = 0;
    prefix = QString("/home/kde4/clock/");
    drawClock();
}

QRectF Clock::boundingRect() const
{
    return m_rect;
}

void Clock::data(const DataSource::Data &data)
{
}

void Clock::setPath(QString str)
{
    prefix  = str+"/";
}

void Clock::drawClock()
{
    qDebug() << "prefix is: " << prefix;

    _clock_bg = QImage (prefix + "background.png");
    _clock_bg = _clock_bg.convertDepth (32);
    _clock_bg.setAlphaBuffer (true);

    //this->//resize (_clock_bg.width (), _clock_bg.height ());

    /*gloss stuff*/
    gloss = QImage(prefix + "gloss.png");
    gloss.convertDepth(32);
    gloss.setAlphaBuffer(true);

    face = QPixmap (prefix + "face.png");
    thedot = QPixmap (prefix + "thedot.png");
    //date = QPixmap(prefix+"date_field.png");


    /**seconds first **/
    _secs_hand = QPixmap (QImage (prefix + "second-hand-long.png"));
    sec_timer = new QTimer (this);
    connect (sec_timer, SIGNAL (timeout ()), this, SLOT (drawSeconds ()));
    sec_timer->start (500, FALSE);
    /** Mins after that **/
    _mins_hand = QPixmap (QImage (prefix + "second-hand.png"));
    _hour_hand = QPixmap (QImage (prefix + "second-hand.png"));


    /**Createsa a nice Lense**/
    /** Prerender into Qimage to save Processing in Painter event**/
    double rad = ((face.width () / 2)) - 8.0;
    int offset = 28;
    QRect bounds (0, 0, face.width () - offset, face.height () - offset);
    QPainter p;

    lens =
    QImage (QSize (face.width () - offset, face.height () - offset),
            QImage::Format_ARGB32_Premultiplied);

    lens.fill (0);

    p.begin (&lens);
    QRadialGradient gr (rad, rad, rad, 3 * rad / 5, 3 * rad / 5);
    gr.setColorAt (0.0, QColor (255, 255, 255, 191));
    gr.setColorAt (0.2, QColor (255, 255, 231, 191));
    gr.setColorAt (0.9, QColor (150, 150, 200, 65));
    gr.setColorAt (0.95, QColor (0, 0, 0, 0));
    gr.setColorAt (1, QColor (0, 0, 0, 0));
    p.setRenderHint (QPainter::Antialiasing);
    p.setBrush (gr);
    p.setPen (Qt::NoPen);
    p.drawEllipse (0, 0, bounds.width (), bounds.height ());
    p.end ();
}

Clock::~Clock()
{
}

void Clock::drawSeconds()
{
    time = QTime::currentTime ();
    _secs = 6.0 * time.second ();
    _mins = 6.0 * time.minute ();
    _hour =  30.0 * time.hour ();
    update ();
}

void Clock::paint(QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget)

    QRectF r = option->exposedRect;
    p->setRenderHint (QPainter::SmoothPixmapTransform);
    //  if (shade == 0)
    //  {
        p->drawImage(_clock_bg.rect(), _clock_bg);
        shade = 1;
    // }

    p->drawPixmap (QRect (16, 16, face.width(), face.height()), face);

    /*Draw Hours*/
    p->save ();
    p->setRenderHint(QPainter::SmoothPixmapTransform);
    p->translate(_clock_bg.width() / 2, _clock_bg.height() / 2);
    p->rotate(_hour);
    p->drawPixmap(QRect
                (-(ceil (_hour_hand.width() / 2)),
                    -(_hour_hand.height() - 32), _hour_hand.width(),
                    _hour_hand.height()), _hour_hand);
    p->restore();

    /* Draw Mins */
    p->save();
    p->setRenderHint(QPainter::SmoothPixmapTransform);
    p->translate(_clock_bg.width() / 2, _clock_bg.height() / 2);
    p->rotate(_mins);
    p->drawPixmap(QRect
                (-(ceil (_mins_hand.width() / 2)),
                    -(_mins_hand.height() - 16), _mins_hand.width(),
                    _mins_hand.height()), _mins_hand);
    p->restore ();

    /*Draw Secs*/
    p->save();
    p->setRenderHint (QPainter::SmoothPixmapTransform);
    p->translate(_clock_bg.width() / 2, _clock_bg.width() / 2);
    p->rotate(_secs);
    p->drawPixmap(QRect
                (-(ceil (_secs_hand.width() / 2)),
                    -(_secs_hand.height() - 32), _secs_hand.width(),
                    _secs_hand.height()), _secs_hand);
    p->restore();

    p->save();
    p->translate(_clock_bg.width() / 2, _clock_bg.width() / 2);
    p->drawPixmap(QRect
                (-(thedot.width() / 2), -(thedot.height() / 2),
                    thedot.width(), thedot.height()), QPixmap (thedot));
    p->restore();

    p->drawImage(QRect(29, 29, gloss.width(), gloss.height()), gloss);
    p->drawImage(QRect(28, 28, lens.width(), lens.height()), lens);

    p->drawPixmap(QRect
                ((_clock_bg.width() / 4) * 3, _clock_bg.height() / 2,
                    date.width(), date.height()), date);

    // p.end ();
}

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
}



} // Plasma namespace

