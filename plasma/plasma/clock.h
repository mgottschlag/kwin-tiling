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

#ifndef CLOCK
#define CLOCK

#include <QImage>
#include <QPaintDevice>
#include <QLabel>
#include <QPixmap>
#include <QTimer>
#include <QPaintEvent>
#include <QPainter>
#include <QTime>
#include <QX11Info>
#include <QWidget>
#include <QGraphicsItem>
#include <QColor>

#include <X11/Xlib.h>
#include <X11/extensions/composite.h>
#include <X11/extensions/Xcomposite.h>
#include <X11/extensions/Xrender.h>
#include <X11/extensions/shape.h>
#include <X11/extensions/Xdamage.h>

#include <kdemacros.h>

#include "datavisualization.h"


class QTimer;
namespace Plasma
{

class Clock : public DataVisualization,
              public QGraphicsItem
{
        Q_OBJECT
    public:
        Clock (QGraphicsItem *parent = 0);
        ~Clock();

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget =0);
        void setPath(QString);
        void drawClock();
        QRectF boundingRect() const;

    public slots:
        void drawSeconds();
        void data(const DataSource::Data& data) {}


    protected:
        QVariant itemChange(GraphicsItemChange change, const QVariant &value);

    private:
        QRectF m_rect;

        QTimer *sec_timer;
        QTimer *mins_timer;
        QTimer *hours_timer;

        QPixmap  _secs_hand;
        QPixmap  _mins_hand;
        QPixmap  _hour_hand;

        QStringList _hour_path;
        QStringList _mins_path;
        QStringList _secs_path;

        QImage _clock_bg;
        QImage _bg;
        QString prefix;

        double _secs;
        double _mins;
        double _hour;

        int seconds;
        int s_a;
        QTime time;
        QPoint clickPos;
        QPixmap face;

        /*additional Clock images : Opera Clock seems to do so*/
        QImage gloss;
        QPixmap thedot;
        int shade;
        QImage lens;
        QPixmap date;

};

}   // Plasma namespace

#endif
