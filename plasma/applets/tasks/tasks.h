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

#ifndef TASKS_H
#define TASKS_H

// Own
#include "ui_tasksConfig.h"

// KDE
#include <taskmanager/taskmanager.h>
#include <KDialog>

// Qt
#include <QTimer>

// Plasma
#include <plasma/applet.h>

namespace Plasma
{
    class LayoutAnimator;
} // namespace Plasma

class AbstractTaskItem;
class TaskGroupItem;

using TaskManager::StartupPtr;
using TaskManager::TaskPtr;

/**
 * An applet which provides a visual representation of running
 * graphical tasks (ie. tasks that have some form of visual interface),
 * and allows the user to perform various actions on those tasks such
 * as bringing them to the foreground, sending them to the background
 * or closing them.
 */
class Tasks : public Plasma::Applet
{
    Q_OBJECT
public:
        /**
         * Constructs a new tasks applet
         * With the specified parent.
         */
        explicit Tasks(QObject *parent, const QVariantList &args = QVariantList());
        ~Tasks();

        void init();

        /**
         * TODO: Sets the strategy used to automatically group tasks
         * together.
         *
         * In addition to manual grouping of tasks which the user can
         * do by dragging tasks on top of one another, the Tasks
         * applet supports automatic grouping of tasks.
         */
        //void setGroupingStrategy(AbstractGroupingStrategy* strategy);

        // we re-implement boundingRect() instead of contentSizeHint() because
        // of issues with using childrenBoundingRect().size().  It shouldn't
        // cause a problem as long as we don't ask Plasma::Applet to draw
        // a background
        //QRectF boundingRect() const;

        void constraintsUpdated(Plasma::Constraints constraints);
        void showConfigurationInterface();

protected slots:
        void configAccepted();
        virtual void wheelEvent(QGraphicsSceneWheelEvent *);
        void themeRefresh();

private slots:
        void addWindowTask(TaskPtr);
        void removeWindowTask(TaskPtr);

        void addStartingTask(StartupPtr);
        void removeStartingTask(StartupPtr);

        void currentDesktopChanged(int);
        void taskMovedDesktop(TaskPtr task);
        void windowChangedGeometry(TaskPtr task);
        void checkScreenChange();

private:
        // creates task representations for existing windows
        // and sets up connections to listen for addition or removal
        // of windows
        void registerWindowTasks();

        void addItemToRootGroup(AbstractTaskItem* item);
        void removeItemFromRootGroup(AbstractTaskItem* item);

        // remove all tasks from the taskbar
        void removeAllWindowTasks();

        bool isOnMyScreen(TaskPtr task);
        void reconnect();

        TaskGroupItem* m_rootTaskGroup;

        QHash<TaskPtr,AbstractTaskItem*> m_windowTaskItems;
        QHash<StartupPtr,AbstractTaskItem*> m_startupTaskItems;

        bool m_showTooltip;
        bool m_showOnlyCurrentDesktop;
        bool m_showOnlyCurrentScreen;
        Plasma::LayoutAnimator *m_animator;
        KDialog *m_dialog;
        Ui::tasksConfig m_ui;
        QList<TaskPtr> m_tasks;
        QTimer m_screenTimer;
};

K_EXPORT_PLASMA_APPLET(tasks, Tasks)

#endif
