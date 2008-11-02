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

// Own
#include "windowtaskitem.h"
#include "taskgroupitem.h"

// Qt
#include <QGraphicsSceneContextMenuEvent>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsView>
#include <QTimer>
#include <QApplication>

// KDE
#include <KAuthorized>
#include <KDebug>
#include <KIcon>
#include <KLocalizedString>
#include <KGlobalSettings>
#include <KIconLoader>

#include <taskmanager/taskactions.h>
#include <taskmanager/task.h>
#include <taskmanager/taskmanager.h>
#include <taskmanager/taskgroup.h>

#include "plasma/theme.h"
#include "plasma/framesvg.h"
#include "plasma/tooltipmanager.h"
#include "plasma/corona.h"
#include "plasma/containment.h"

#include "tasks.h"

WindowTaskItem::WindowTaskItem(QGraphicsWidget *parent, Tasks *applet, const bool showTooltip)
    : AbstractTaskItem(parent, applet, showTooltip),
      m_task(0)
{
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
    //kDebug();
    if (m_task) {
        m_task->task()->activateRaiseOrIconify();
       // emit windowSelected(this);
    }
}

void WindowTaskItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->buttons() & Qt::MidButton) {
        if (isGrouped()) {
            parentGroup()->collapse();
        }
    } else {
        AbstractTaskItem::mousePressEvent(event);
    }

    event->accept();
}

//destroy this item
void WindowTaskItem::close()
{
    //kDebug();
    m_task = 0;
    finished();
}


void WindowTaskItem::publishIconGeometry()
{
    if (!scene()) {
        return;
    }

    QGraphicsView *parentView = 0L;
    // The following was taken from Plasma::Applet, it doesn't make sense to make the item an applet, and this was the easiest way around it.
    foreach (QGraphicsView *view, scene()->views()) {
        if (view->sceneRect().intersects(sceneBoundingRect()) ||
            view->sceneRect().contains(scenePos())) {
            parentView = view;
        }
    }
    if (!parentView || !m_task) {
        return;
    }
    if( !boundingRect().isValid() )
        return;

    QRect rect = parentView->mapFromScene(mapToScene(boundingRect())).boundingRect().adjusted(0, 0, 1, 1);
    rect.moveTopLeft(parentView->mapToGlobal(rect.topLeft()));
    if (m_task) {
        m_task->task()->publishIconGeometry(rect);
    }
}

void WindowTaskItem::updateTask(::TaskManager::TaskChanges changes)
{
    if (!m_task) {
        return;
    }

    // task flags
    bool needsUpdate = false;
    TaskFlags flags = m_flags;
    if (m_task->isActive()) {
        flags |= TaskHasFocus;
        emit activated(this);
    } else {
        flags &= ~TaskHasFocus;
    }

    if (m_task->demandsAttention()) {
        flags |= TaskWantsAttention;
    } else {
        flags &= ~TaskWantsAttention;
    }

    if (m_task->isMinimized()) {
        flags |= TaskIsMinimized;
    } else {
        flags &= ~TaskIsMinimized;
    }

    if (m_flags != flags) {
        needsUpdate = true;
        setTaskFlags(flags);
    }

    // basic title and icon
    if (changes & TaskManager::IconChanged) {
        needsUpdate = true;
        setIcon(m_task->icon());
    }

    if (changes & TaskManager::NameChanged) {
        needsUpdate = true;
        setText(m_task->name());
    }

    if (m_showingTooltip &&
        (changes & TaskManager::IconChanged ||
         changes & TaskManager::NameChanged ||
         changes & TaskManager::DesktopChanged)) {
        updateToolTip();
    }

    if (needsUpdate) {
        //redraw
        //kDebug() << m_task->name();
        queueUpdate();
    }
}

void WindowTaskItem::updateToolTip()
{
    if (!m_task) {
        return;
    }

    Plasma::ToolTipContent data(m_task->name(),
                                i18nc("Which virtual desktop a window is currently on", "On %1",
                                      KWindowSystem::desktopName(m_task->desktop())),
                                m_task->task()->icon(KIconLoader::SizeSmall, KIconLoader::SizeSmall, false));
    data.setWindowToPreview(m_task->task()->window());

    Plasma::ToolTipManager::self()->setContent(this, data);
}

void WindowTaskItem::setStartupTask(TaskItem *task)
{
    //kDebug();
    if (!task->startup()) {
        kDebug() << "Error";
        return;
    }
    m_abstractItem = qobject_cast<TaskManager::AbstractGroupableItem *>(task);
    if (!m_abstractItem) {
        kDebug() << "error";
    }
    connect(task, SIGNAL(gotTaskPointer()), this, SLOT(gotTaskPointer()));
    setText(task->startup()->text());
    setIcon(KIcon(task->startup()->icon()));
}

void WindowTaskItem::gotTaskPointer()
{
    //kDebug();
    TaskManager::TaskItem *item = qobject_cast<TaskManager::TaskItem*>(sender());
    if (item) {
        setWindowTask(item);
    }
}

void WindowTaskItem::setWindowTask(TaskManager::TaskItem* taskItem)
{
    if (m_task) {
        disconnect(m_task->task().constData(), 0, this, 0);
    }
    m_task = taskItem;
    m_abstractItem = qobject_cast<TaskManager::AbstractGroupableItem *>(taskItem);
    if (!m_abstractItem) {
        kDebug() << "error";
    }

    connect(m_task, SIGNAL(changed(::TaskManager::TaskChanges)),
            this, SLOT(updateTask(::TaskManager::TaskChanges)));

    updateTask(::TaskManager::EverythingChanged);
    publishIconGeometry();

    //kDebug() << "Task added, isActive = " << task->isActive();
}


TaskManager::TaskPtr WindowTaskItem::windowTask() const
{
    return m_task->task();
}

void WindowTaskItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *e)
{
    if (!KAuthorized::authorizeKAction("kwin_rmb") || !m_task) {
        QGraphicsWidget::contextMenuEvent(e);
        return;
    }

    QList <QAction*> actionList;
    QAction *a(0);
    if (m_task->isGrouped()) {
        a = new QAction(i18n("Collapse Parent Group"), this);
        actionList.append(a);
        connect(a, SIGNAL(triggered()), m_applet->groupItem(m_task->parentGroup()), SLOT(collapse()));
    }

    TaskManager::BasicMenu menu(0, m_task, &m_applet->groupManager(), actionList);
    menu.adjustSize();
    Q_ASSERT(m_applet->containment());
    Q_ASSERT(m_applet->containment()->corona());
    menu.exec(m_applet->containment()->corona()->popupPosition(this, menu.size()));
    delete a;
}



bool WindowTaskItem::isWindowItem() const
{
    return true;
}

bool WindowTaskItem::isActive() const
{
    if (!m_task) {
        //kDebug() << "no task set";
        return false;
    }
    return m_task->isActive();
}

#include "windowtaskitem.moc"

