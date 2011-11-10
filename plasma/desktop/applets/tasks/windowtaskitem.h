/***************************************************************************
 *   Copyright (C) 2007 by Robert Knight <robertknight@gmail.com>          *
 *   Copyright (C) 2008 by Alexis Ménard <darktears31@gmail.com>           *
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


#ifndef WINDOWTASKITEM_H
#define WINDOWTASKITEM_H

#include <Plasma/BusyWidget>
#include "abstracttaskitem.h"
// Own
#include <taskmanager/taskmanager.h>
#include <taskmanager/taskitem.h>

/**
 * A task item for a task which represents a window on the desktop.
 */
class WindowTaskItem : public AbstractTaskItem
{
    Q_OBJECT

public:
    /** Constructs a new representation for a window task. */
    WindowTaskItem(QGraphicsWidget *parent, Tasks *applet);

    /** Sets the window/startup represented by this task. */
    void setTask(TaskManager::TaskItem* taskItem);

    /** Tells the window manager the minimized task's geometry. */
    void publishIconGeometry() const;

    // used by the group; for efficiency this avoids multiple calls to
    // AbstractTaskItem::iconScreenGeometry
    void publishIconGeometry(const QRect &rect) const; 

    virtual bool isWindowItem() const;
    virtual bool isActive() const;
    virtual void setAdditionalMimeData(QMimeData* mimeData);
    QGraphicsWidget *busyWidget() const;

signals:
    /** Emitted when a window is selected for activation, minimization, iconification */
    //void windowSelected(WindowTaskItem *); //what is it for?

public slots:
    void activate();

protected:
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void updateToolTip();

private slots:
    void updateTask(::TaskManager::TaskChanges changes);
    void gotTaskPointer();

private:
    /** Sets the starting task represented by this item. */
    void setStartupTask(TaskManager::TaskItem* task);

    /** Sets the window represented by this task. */
    void setWindowTask(TaskManager::TaskItem* taskItem);

    QWeakPointer<TaskManager::TaskItem> m_task;
    Plasma::BusyWidget *m_busyWidget;
};

#endif
