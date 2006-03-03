/*****************************************************************

Copyright (c) 2002 John Firebaugh <jfirebaugh@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#include <QApplication>
#include <QRect>
#include <QDesktopWidget>
#include <qtimer.h>
#include <qcursor.h>
#include <kdebug.h>

#include "unhidetrigger.h"
#include "unhidetrigger.moc"

UnhideTrigger* UnhideTrigger::self()
{
    static UnhideTrigger UnhideTrigger;
    return &UnhideTrigger;
}

UnhideTrigger::UnhideTrigger()
    : m_lastTrigger(Plasma::NoEdge),
      m_lastXineramaScreen(-1),
      m_enabledCount(0)
{
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), SLOT(pollMouse()));
}

void UnhideTrigger::setEnabled( bool enable )
{
    if (enable)
    {
        m_enabledCount++;
    }
    else
    {
        m_enabledCount--;
    }

    if (m_enabledCount > 0 && !m_timer->isActive())
    {
        m_timer->start( 100 );
    }
    else if (m_enabledCount <= 0)
    {
        m_timer->stop();
    }
}

bool UnhideTrigger::isEnabled() const
{
    return m_timer->isActive();
}

void UnhideTrigger::pollMouse()
{
    QPoint pos = QCursor::pos();
    for (int s = 0; s < QApplication::desktop()->numScreens(); ++s)
    {
        QRect r = QApplication::desktop()->screenGeometry(s);
        if (pos.x() == r.left())
        {
            if (pos.y() == r.top())
            {
                emitTrigger(Plasma::TopLeftEdge, s);
            }
            else if (pos.y() == r.bottom())
            {
                emitTrigger(Plasma::BottomLeftEdge, s);
            }
            else
            {
                emitTrigger(Plasma::LeftEdge, s);
            }
        }
        else if (pos.x() == r.right())
        {
            if (pos.y() == r.top())
            {
                emitTrigger(Plasma::TopRightEdge, s);
            }
            else if (pos.y() == r.bottom())
            {
                emitTrigger(Plasma::BottomRightEdge, s);
            }
            else
            {
                emitTrigger(Plasma::RightEdge, s);
            }
        }
        else if (pos.y() == r.top())
        {
            emitTrigger(Plasma::TopEdge, s);
        }
        else if (pos.y() == r.bottom())
        {
            emitTrigger(Plasma::BottomEdge, s);
        }
        else if (m_lastTrigger != Plasma::NoEdge)
        {
            emitTrigger(Plasma::NoEdge, -1);
        }
    }
}

void UnhideTrigger::resetTriggerThrottle()
{
    m_lastTrigger = Plasma::NoEdge;
    m_lastXineramaScreen = -1;
}

void UnhideTrigger::emitTrigger(Plasma::ScreenEdge t, int XineramaScreen)
{
    if (m_lastTrigger == t  && m_lastXineramaScreen == XineramaScreen)
    {
        return;
    }

    resetTriggerThrottle();
    emit triggerUnhide(t, XineramaScreen);
}

void UnhideTrigger::triggerAccepted(Plasma::ScreenEdge t, int XineramaScreen)
{
    m_lastTrigger = t;
    m_lastXineramaScreen = XineramaScreen;
}

