/****************************************************************************

 KHotKeys

 Copyright (C) 2003 Mike Pilone <mpilone@slac.com>
 Copyright (C) 2003 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.

****************************************************************************/

#include <QtCore/QEvent>
#include <QtGui/QColor>
#include <QtGui/QFrame>
#include <QtGui/QMouseEvent>

#include "gesture_recorder.h"

GestureRecorder::GestureRecorder(QWidget *parent, const char *name)
  : QFrame(parent), _mouseButtonDown(false)
    {
    setObjectName(name);
    QPalette p;
    p.setColor( backgroundRole(), palette().color( QPalette::Base ) );
    setPalette( p );
    setFrameStyle(QFrame::Sunken | QFrame::Panel);
    setLineWidth(2);
    setMidLineWidth(0);
    setMinimumSize(200,200);
    }


GestureRecorder::~GestureRecorder()
    {
    }


void GestureRecorder::mousePressEvent(QMouseEvent *ev)
    {
    if (ev->button() == Qt::LeftButton)
        {
        _mouseButtonDown = true;
        stroke.reset();
        QPoint pos = ev->pos();
        stroke.record(pos.x(), pos.y());
        }
    }


void GestureRecorder::mouseReleaseEvent(QMouseEvent *ev)
    {
    if ((ev->button() == Qt::LeftButton) && (_mouseButtonDown))
        {
        QPoint pos = ev->pos();
        stroke.record(pos.x(), pos.y());
        KHotKeys::StrokePoints data( stroke.processData() );

        if( !data.isEmpty())
            emit recorded(data);
        }
    }


void GestureRecorder::mouseMoveEvent(QMouseEvent *ev)
    {
    if (_mouseButtonDown)
        {
        QPoint pos = ev->pos();
        stroke.record(pos.x(), pos.y());
        }
    }


#include "moc_gesture_recorder.cpp"
