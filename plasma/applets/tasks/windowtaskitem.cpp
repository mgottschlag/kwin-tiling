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
#include "windowtaskitem.h"

// Qt
#include <QGraphicsSceneContextMenuEvent>
#include <QGraphicsView>
#include <QTimer>

// KDE
#include <KAuthorized>
#include <KDebug>
#include <KIcon>
#include <KLocalizedString>
#include <taskmanager/taskrmbmenu.h>

WindowTaskItem::WindowTaskItem(QGraphicsItem *parent, QObject *parentObject, const bool showTooltip)
    : AbstractTaskItem(parent, parentObject),
    _activateTimer(0)
{
    setAcceptDrops(true);
    _showTooltip = showTooltip;
}

void WindowTaskItem::activate()
{
    // the Task class has a method called activateRaiseOrIconify() which
    // should perform the required action here.
    //
    // however it currently does not minimize the task's window if the item
    // is clicked whilst the window is active probably because the active window by
    // the time the mouse is released over the window task button is not the
    // task's window but instead the desktop window
    //
    // TODO: the Kicker panel in KDE 3.x has a feature whereby clicking on it
    // does not take away the focus from the active window (unless clicking
    // in a widget such as a line edit which does accept the focus)
    // this needs to be implemented for Plasma's own panels.
    if (_task) {
        _task->activateRaiseOrIconify();
    }
}

void WindowTaskItem::close()
{
    if (_task) {
        _task->close();
    }
    finished();
}

void WindowTaskItem::setShowTooltip(const bool showit)
{
    _showTooltip = showit;
    updateTask();
}

void WindowTaskItem::updateTask()
{
    Q_ASSERT( _task );

    // task flags
    if (_task->isActive()) {
        setTaskFlags(taskFlags() | TaskHasFocus);
        emit activated(this);
    } else {
        setTaskFlags(taskFlags() & ~TaskHasFocus);
    }

    if (_task->demandsAttention()) {
        setTaskFlags(taskFlags() | TaskWantsAttention);
    } else {
        setTaskFlags(taskFlags() & ~TaskWantsAttention);
    }

    // basic title and icon
    QPixmap iconPixmap = _task->icon(preferredIconSize().width(),
                                     preferredIconSize().height(),
                                     true);
    if (_showTooltip) {
      Plasma::ToolTipData data;
      data.mainText = _task->visibleName();
      data.subText = i18nc("Which virtual desktop a window is currently on", "On %1", KWindowSystem::desktopName(_task->desktop()));
      data.image = iconPixmap;
      setToolTip(data);
    } else {
        Plasma::ToolTipData data;
        setToolTip(data); // Clear
    }
    setIcon(QIcon(iconPixmap));
    setText(_task->visibleName());
    //redraw
    queueUpdate();
}

void WindowTaskItem::setStartupTask(Startup::StartupPtr task)
{
    setText(task->text());
    setIcon(KIcon(task->icon()));

    Plasma::ToolTipData tip;
    tip.mainText = task->text();
    tip.image = task->icon();
    setToolTip(tip);
}

void WindowTaskItem::setWindowTask(Task::TaskPtr task)
{
    if (_task)
        disconnect(_task.constData(), 0, this, 0);

    _task = task;

    connect(task.constData(), SIGNAL(changed()),
            this, SLOT(updateTask()));
    connect(task.constData(), SIGNAL(iconChanged()),
            this, SLOT(updateTask()));

    updateTask();

    qDebug() << "Task added, isActive = " << task->isActive();
}

Task::TaskPtr WindowTaskItem::windowTask() const
{
    return _task;
}

void WindowTaskItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *e)
{
    if(!KAuthorized::authorizeKAction("kwin_rmb") )
    {
        return;
    }
    e->accept();
    TaskRMBMenu menu( windowTask() );
    menu.exec( e->screenPos() );
}

void WindowTaskItem::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    event->accept();
    if (!_activateTimer) {
        _activateTimer = new QTimer();
        _activateTimer->setSingleShot(true);
        _activateTimer->setInterval(300);
        connect(_activateTimer, SIGNAL(timeout()), this, SLOT(activate()));
    }
    _activateTimer->start();
}

void WindowTaskItem::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    Q_UNUSED(event);

    // restart the timer so that activate() is only called after the mouse
    // stops moving
    _activateTimer->start();
}

void WindowTaskItem::dragLeaveEvent(QGraphicsSceneDragDropEvent *event)
{
    Q_UNUSED(event);

    delete _activateTimer;
    _activateTimer = 0;
}

void WindowTaskItem::setGeometry(const QRectF& geometry)
{
    AbstractTaskItem::setGeometry(geometry);
    publishIconGeometry();
}

void WindowTaskItem::publishIconGeometry()
{
    QGraphicsView *parentView = view();
    if (!parentView || !_task) {
        return;
    }
    QRect rect = mapToView(parentView, boundingRect());
    rect.moveTopLeft(parentView->mapToGlobal(rect.topLeft()));
    _task->publishIconGeometry(rect);
}

#include "windowtaskitem.moc"
