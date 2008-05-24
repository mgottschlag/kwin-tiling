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

#ifndef TASKS_H
#define TASKS_H

// Own
#include "ui_tasksConfig.h"

// Qt
#include <QTimer>

// KDE
#include <taskmanager/taskmanager.h>
#include <KDialog>

// Plasma
#include <plasma/applet.h>

class QGraphicsLinearLayout;

class KColorScheme;

namespace Plasma
{
    class LayoutAnimator;
    class PanelSvg;
} // namespace Plasma

class WindowTaskItem;

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

        void constraintsEvent(Plasma::Constraints constraints);

        Plasma::PanelSvg *itemBackground();
        KColorScheme *colorScheme();

        qreal itemLeftMargin() { return m_leftMargin; }
        qreal itemRightMargin() { return m_rightMargin; }
        qreal itemTopMargin() { return m_topMargin; }
        qreal itemBottomMargin() { return m_bottomMargin; }
        void resizeItemBackground(const QSizeF &newSize);

protected slots:
        void configAccepted();
        virtual void wheelEvent(QGraphicsSceneWheelEvent *);
        void themeRefresh();
        void updateActive(WindowTaskItem *task);

protected:
        void createConfigurationInterface(KConfigDialog *parent);

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

        // remove all tasks from the taskbar
        void removeAllWindowTasks();

        void insertItemBeforeSpacer(QGraphicsWidget * item);

        bool isOnMyScreen(TaskPtr task);
        void reconnect();

        QHash<TaskPtr,WindowTaskItem*> m_windowTaskItems;
        QHash<StartupPtr,WindowTaskItem*> m_startupTaskItems;
        QHash<TaskPtr,WindowTaskItem*>::iterator m_activeTask;

        bool m_showTooltip;
        bool m_showOnlyCurrentDesktop;
        bool m_showOnlyCurrentScreen;
        Plasma::LayoutAnimator *m_animator;
        QGraphicsLinearLayout *m_layout;
        Ui::tasksConfig m_ui;
        QList<TaskPtr> m_tasks;
        QTimer m_screenTimer;

        Plasma::PanelSvg *m_taskItemBackground;
        KColorScheme *m_colorScheme;
        qreal m_leftMargin;
        qreal m_topMargin;
        qreal m_rightMargin;
        qreal m_bottomMargin;
};

#endif
