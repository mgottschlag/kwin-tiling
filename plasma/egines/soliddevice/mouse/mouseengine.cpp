/*
 *   Copyright © 2007 Fredrik Höglund <fredrik@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
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

#include <QCursor>

#include "mouseengine.h"
#include "mouseengine.moc"



MouseEngine::MouseEngine(QObject* parent, const QStringList& args)
    : Plasma::DataEngine(parent), timerId(0)
{
    Q_UNUSED(args)
}


MouseEngine::~MouseEngine()
{
    if (timerId)
        killTimer(timerId);
}


QStringList MouseEngine::sources() const
{
    return QStringList() << QLatin1String("Position");
}


void MouseEngine::init()
{
    if (!timerId)
        timerId = startTimer(40);

    QPoint pos = QCursor::pos();
    setData(QLatin1String("Position"), QVariant(pos));
    lastPosition = pos;

    checkForUpdates();
}


void MouseEngine::timerEvent(QTimerEvent *)
{
    QPoint pos = QCursor::pos();

    if (pos != lastPosition)
    {
        setData(QLatin1String("Position"), QVariant(pos));
        lastPosition = pos;

        checkForUpdates();
    }
}

