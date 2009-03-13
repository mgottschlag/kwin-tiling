/****************************************************************************

 KHotKeys

 Copyright (C) 2003 Mike Pilone <mpilone@slac.com>
 Copyright (C) 2003 Lubos Lunak <l.lunak@kde.org>
 Copyright (C) 2009 Michael Jansen <kde@michael-jansen.biz>

 Distributed under the terms of the GNU General Public License version 2.

****************************************************************************/

#include <QtGui/QColor>
#include <QtGui/QPainter>
#include <QtGui/QFrame>
#include <QtGui/QPaintEvent>

#include "gesture_drawer.h"

GestureDrawer::GestureDrawer(QWidget *parent, const char *name)
  : QFrame(parent), _data(KHotKeys::StrokePoints())
    {
    setObjectName(name);
    QPalette p;
    p.setColor( backgroundRole(), palette().color( QPalette::Base ) );
    setPalette( p );
    setFrameStyle(QFrame::Panel | QFrame::Sunken);
    setMinimumSize(30, 30);
    }


GestureDrawer::~GestureDrawer()
    {
    }


KHotKeys::StrokePoints GestureDrawer::pointData() const
    {
    return _data;
    }


void GestureDrawer::setPointData(const KHotKeys::StrokePoints &data)
    {
    _data = data;

    repaint();
    }

void GestureDrawer::paintEvent(QPaintEvent *ev)
    {
    const int n = _data.size();

    if( n < 2 )
        {
        QFrame::paintEvent(ev);
        return;
        }

    const int border=6;

    const int l = width() < height() ? width() : height();
    const int x_offset = border + ( width()-l )/2;
    const int y_offset = border + ( height()-l )/2;


    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    QPen pen;
    pen.setWidth(4);
    pen.setCapStyle(Qt::RoundCap);

    // starting point
    double x = x_offset + _data[0].x * (l - 2*border);
    double y = y_offset + _data[0].y * (l - 2*border);

    for(int i=0; i<n-1; i++)
        {

        double nextx = x_offset + _data[i+1].x * (l - 2*border);
        double nexty = y_offset + _data[i+1].y * (l - 2*border);

        pen.setBrush(QColor(0,(1-_data[i].s)*255,_data[i].s*255));
        p.setPen(pen);
        p.drawLine(x, y, nextx, nexty);

        x=nextx;
        y=nexty;
        }


    QFrame::paintEvent(ev);
    }

#include "moc_gesture_drawer.cpp"
