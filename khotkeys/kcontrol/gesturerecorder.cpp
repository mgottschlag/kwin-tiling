/****************************************************************************

 KHotKeys
 
 Copyright (C) 2003 Mike Pilone <mpilone@slac.com>
 Copyright (C) 2003 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

#include <qcolor.h>
#include <qevent.h>

#include "gesturerecorder.h"

namespace KHotKeys
{

GestureRecorder::GestureRecorder(QWidget *parent, const char *name)
  : QFrame(parent, name), _mouseButtonDown(false)
    {
    setBackgroundColor( colorGroup().base());
    setFrameStyle(QFrame::Sunken | QFrame::Panel);
    setLineWidth(2);
    setMidLineWidth(0);
    }

GestureRecorder::~GestureRecorder()
    {
    }

void GestureRecorder::mousePressEvent(QMouseEvent *ev)
    {
    if (ev->button() == QMouseEvent::LeftButton)
        {
        _mouseButtonDown = true;
        stroke.reset();
        QPoint pos = ev->pos();
        stroke.record(pos.x(), pos.y());
        }
    }

void GestureRecorder::mouseReleaseEvent(QMouseEvent *ev)
    {
    if ((ev->button() == QMouseEvent::LeftButton) && (_mouseButtonDown))
        {
        QPoint pos = ev->pos();
        stroke.record(pos.x(), pos.y());
        QString data( stroke.translate());
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

} // namespace KHotKeys

#include "gesturerecorder.moc"
