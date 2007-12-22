/***************************************************************************
 *   Copyright (C) 2007 by Robert Knight                                   *
 *   robertknight@gmail.com                                                *
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

// Own
#include "startuptaskitem.h"

// KDE
#include <KIcon>

StartupTaskItem::StartupTaskItem(QGraphicsItem *parent, QObject *parentObject)
    : AbstractTaskItem(parent, parentObject)
{
}

void StartupTaskItem::setStartupTask(Startup::StartupPtr task)
{
    _task = task;

    setText(task->text());
    setIcon(KIcon(task->icon()));
    Plasma::ToolTipData tip;
    tip.mainText = task->text();
    tip.image = task->icon();
}

Startup::StartupPtr StartupTaskItem::startupTask() const
{
    return _task;
}

#include "startuptaskitem.moc"
