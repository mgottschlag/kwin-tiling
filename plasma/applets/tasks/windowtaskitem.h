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

#ifndef WINDOWTASKITEM_H
#define WINDOWTASKITEM_H

// Own
#include "abstracttaskitem.h"

// KDE
#include <taskmanager/taskmanager.h>

/**
 * A task item for a task which represents a window on the desktop.
 */
class WindowTaskItem : public AbstractTaskItem
{
    Q_OBJECT

public:
    /** Constructs a new representation for a window task. */
    WindowTaskItem(QGraphicsItem *parent, QObject *parentObject, const bool showTooltip);

    /** Sets the starting task represented by this item. */
    void setStartupTask(TaskManager::StartupPtr task);

    /** Sets the window represented by this task. */
    void setWindowTask(TaskManager::TaskPtr task);
    /** Returns the window represented by this task. */
    TaskManager::TaskPtr windowTask() const;

    /** Tells the window manager the minimized task's geometry. */
    void publishIconGeometry();

    /** Switch on/off tooltips above tasks */
    void setShowTooltip(const bool showit);

    virtual void close();

    /** Overrided from LayoutItem */
    void setGeometry(const QRectF& geometry);

public slots:
    virtual void activate();

protected:
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    virtual void dragEnterEvent(QGraphicsSceneDragDropEvent *event);
    virtual void dragMoveEvent(QGraphicsSceneDragDropEvent *event);
    virtual void dragLeaveEvent(QGraphicsSceneDragDropEvent *event);

private slots:
    void updateTask();

private:
    TaskManager::TaskPtr _task;
    QTimer* _activateTimer;
    bool _showTooltip;
};

#endif
