/*
    Copyright (C) 2007 Robert Knight <robertknight@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the
    Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .
 */

#ifndef STARTUPTASKITEM_H
#define STARTUPTASKITEM_H

// Own
#include "abstracttaskitem.h"

// KDE
#include <taskmanager/taskmanager.h>

/**
 * A task item which represents a newly started task which does not yet have
 * a window visible on screen.
 *
 * Startup tasks are short-lived and disappear if
 */
class StartupTaskItem : public AbstractTaskItem
{
public:
    /** Constructs a new representation for a starting task. */
    StartupTaskItem(QGraphicsItem *parent, QObject *parentObject);

    /** Sets the starting task represented by this item. */
    void setStartupTask(Startup::StartupPtr task);
    /** Returns the starting task represented by this item. */
    Startup::StartupPtr startupTask() const;

    // reimplemented, does nothing because there is no window to show
    virtual void activate() {};

private:
    Startup::StartupPtr _task;
};

#endif
