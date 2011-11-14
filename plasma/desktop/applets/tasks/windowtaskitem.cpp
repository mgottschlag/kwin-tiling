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
#include <QVarLengthArray>
#include <QTextDocument>

// KDE
#include <KAuthorized>
#include <KDebug>
#include <KIcon>
#include <KIconLoader>

#include <taskmanager/taskactions.h>
#include <taskmanager/task.h>
#include <taskmanager/taskmanager.h>
#include <taskmanager/taskgroup.h>

#include <Plasma/Theme>
#include <Plasma/FrameSvg>
#include <Plasma/ToolTipManager>
#include <Plasma/Corona>
#include <Plasma/Containment>

#include "tasks.h"

#ifdef Q_WS_X11

#include <X11/Xlib.h>
#include <fixx11h.h>
#endif

WindowTaskItem::WindowTaskItem(QGraphicsWidget *parent, Tasks *applet)
    : AbstractTaskItem(parent, applet),
      m_busyWidget(0)
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
    if (m_task && m_task.data()->task()) {
        m_task.data()->task()->activateRaiseOrIconify();
       // emit windowSelected(this);
    }
}

void WindowTaskItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->buttons() & Qt::MidButton) {
        if (isGrouped() && parentGroup()) {
            parentGroup()->collapse();
        }
    } else {
        AbstractTaskItem::mousePressEvent(event);
    }

    event->accept();
}

void WindowTaskItem::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
    {
        activate();
    }
    else
    {
        QGraphicsWidget::keyPressEvent(event);
    }
}

void WindowTaskItem::publishIconGeometry() const
{
    if (!m_task || !m_task.data()->task()) {
        return;
    }

    QRect rect = iconGeometry();
    m_task.data()->task()->publishIconGeometry(rect);
}

void WindowTaskItem::publishIconGeometry(const QRect &rect) const
{
    if (m_task && m_task.data()->task()) {
        m_task.data()->task()->publishIconGeometry(rect);
    }
}

void WindowTaskItem::updateTask(::TaskManager::TaskChanges changes)
{
    if (!m_task) {
        return;
    }

    bool needsUpdate = false;
    TaskFlags flags = m_flags;

    if (changes & TaskManager::StateChanged) {
        if (m_task.data()->isActive()) {
            flags |= TaskHasFocus;
            if (!(m_flags & TaskHasFocus)) {
                emit activated(this);
            }
        } else {
            flags &= ~TaskHasFocus;
        }

        if (m_task.data()->isMinimized()) {
            flags |= TaskIsMinimized;
        } else {
            flags &= ~TaskIsMinimized;
        }

    }

    if (changes & TaskManager::AttentionChanged) {
        if (m_task.data()->demandsAttention()) {
            flags |= TaskWantsAttention;
        } else {
            flags &= ~TaskWantsAttention;
        }
    }

    if (m_flags != flags) {
        needsUpdate = true;
        setTaskFlags(flags);
    }

    // basic title and icon
    if (changes & TaskManager::IconChanged) {
        needsUpdate = true;
    }

    if (changes & TaskManager::NameChanged) {
        needsUpdate = true;
        textChanged();
    }

    if (Plasma::ToolTipManager::self()->isVisible(this) &&
        (changes & TaskManager::IconChanged ||
         changes & TaskManager::NameChanged ||
         changes & TaskManager::DesktopChanged)) {
        updateToolTip();
    }

    if (needsUpdate) {
        //redraw
        //kDebug() << m_task.data()->name();
        queueUpdate();
    }
}

void WindowTaskItem::updateToolTip()
{
    if (!m_task || !m_task.data()->task()) {
        return;
    }

    bool showToolTip = true;
    TaskGroupItem *group = parentGroup();

    if (group) {
        QWidget *groupPopupDialog = parentGroup()->popupDialog();
        QWidget *dialog = m_applet->popupDialog();

        if (dialog && dialog->isVisible()) {
            if (groupPopupDialog && groupPopupDialog == dialog) {
                showToolTip = true;
            } else {
                showToolTip = false;
            }
        }
    }

    if (showToolTip) {
        QPixmap p = m_task.data()->task()->icon(KIconLoader::SizeLarge, KIconLoader::SizeLarge, false);
        if (p.height() > KIconLoader::SizeLarge) {
            p = p.scaled(QSize(KIconLoader::SizeLarge, KIconLoader::SizeLarge),
                                Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }

        Plasma::ToolTipContent data(Qt::escape(m_task.data()->name()), QString(), p);
        if (m_task.data()->desktop() != 0 && 
            (!m_applet->groupManager().showOnlyCurrentDesktop() || !m_task.data()->isOnCurrentDesktop())) {
            data.setSubText(i18nc("Which virtual desktop a window is currently on", "On %1",
                                  KWindowSystem::desktopName(m_task.data()->desktop())));
        }
        data.setWindowsToPreview(QList<WId>() << m_task.data()->task()->window());
        data.setClickable(true);
        data.setInstantPopup(true);
        data.setHighlightWindows(m_applet->highlightWindows());

        if (group && group->collapsed()) {
            data.setGraphicsWidget(parentWidget());
        }

        Plasma::ToolTipManager::self()->setContent(this, data);
    } else {
        clearToolTip();
    }
}

void WindowTaskItem::setStartupTask(TaskItem *task)
{
    //kDebug();
    if (!task->startup()) {
        kDebug() << "Error";
        return;
    }

    m_abstractItem = task;

    if (m_abstractItem) {
        connect(m_abstractItem, SIGNAL(destroyed(QObject*)), this, SLOT(clearAbstractItem()));
        textChanged();

        connect(task, SIGNAL(gotTaskPointer()), this, SLOT(gotTaskPointer()));

        if (!m_busyWidget) {
            m_busyWidget = new Plasma::BusyWidget(this);
            m_busyWidget->hide();
        }
    }
}

void WindowTaskItem::gotTaskPointer()
{
    //kDebug();
    TaskManager::TaskItem *item = qobject_cast<TaskManager::TaskItem*>(sender());
    if (item) {
        delete m_busyWidget;
        m_busyWidget = 0;

        setWindowTask(item);
    }
}


void WindowTaskItem::setWindowTask(TaskManager::TaskItem* taskItem)
{
    if (m_task && m_task.data()->task()) {
        disconnect(m_task.data()->task(), 0, this, 0);
    }

    m_task = taskItem;
    m_abstractItem = qobject_cast<TaskManager::AbstractGroupableItem *>(taskItem);

    if (m_abstractItem) {
        connect(m_abstractItem, SIGNAL(destroyed(QObject*)), this, SLOT(clearAbstractItem()));
    }

    if (m_task) {
        connect(m_task.data(), SIGNAL(changed(::TaskManager::TaskChanges)),
                this, SLOT(updateTask(::TaskManager::TaskChanges)));
    }

    updateTask(::TaskManager::EverythingChanged);
    publishIconGeometry();

    //kDebug() << "Task added, isActive = " << task->isActive();
}

void WindowTaskItem::setTask(TaskManager::TaskItem* taskItem)
{
    if (!taskItem->startup() && !taskItem->task()) {
        kDebug() << "Error";
        return;
    }

    if (!taskItem->task()) {
        setStartupTask(taskItem);
    } else {
        setWindowTask(taskItem);
    }
}

void WindowTaskItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *e)
{
    if (!KAuthorized::authorizeKAction("kwin_rmb") || !m_task) {
        QGraphicsWidget::contextMenuEvent(e);
        return;
    }

    QList <QAction*> actionList;
    QAction *a(0);
    if (m_task.data()->isGrouped()) {
        a = new QAction(i18n("Collapse Parent Group"), 0);
        actionList.append(a);
        TaskGroupItem *group = qobject_cast<TaskGroupItem*>(m_applet->rootGroupItem()->abstractTaskItem(m_task.data()->parentGroup()));
        connect(a, SIGNAL(triggered()), group, SLOT(collapse()));
    }

    QAction *configAction = m_applet->action("configure");
    if (configAction && configAction->isEnabled()) {
        actionList.append(configAction);
    }

    TaskManager::BasicMenu menu(0, m_task.data(), &m_applet->groupManager(), actionList);
    menu.adjustSize();

    if (m_applet->formFactor() != Plasma::Vertical) {
        menu.setMinimumWidth(size().width());
    }

    Q_ASSERT(m_applet->containment());
    Q_ASSERT(m_applet->containment()->corona());
    stopWindowHoverEffect();
    menu.exec(m_applet->containment()->corona()->popupPosition(this, menu.size()));
    delete a;
}

bool WindowTaskItem::isWindowItem() const
{
    return true;
}

bool WindowTaskItem::isActive() const
{
    return m_task ? m_task.data()->isActive() : false;
}

void WindowTaskItem::setAdditionalMimeData(QMimeData* mimeData)
{
    if (m_task) {
        m_task.data()->addMimeData(mimeData);
    }
}

QGraphicsWidget *WindowTaskItem::busyWidget() const
{
    return m_busyWidget;
}

#include "windowtaskitem.moc"

