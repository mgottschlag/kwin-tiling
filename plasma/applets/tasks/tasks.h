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
#include <QSize>

// KDE
#include <taskmanager/taskmanager.h>
#include <taskmanager/abstractgroupableitem.h>
#include <taskmanager/groupmanager.h>
#include <taskmanager/taskitem.h>
#include <taskmanager/startup.h>
#include <KDialog>

// Plasma
#include <plasma/applet.h>

class QGraphicsLinearLayout;

class KColorScheme;

namespace Plasma
{
    class LayoutAnimator;
    class FrameSvg;
} // namespace Plasma

class WindowTaskItem;
class TaskGroupItem;
class AbstractTaskItem;

using TaskManager::StartupPtr;
using TaskManager::TaskPtr;
using TaskManager::StartupPtr;
using TaskManager::GroupPtr;
using TaskManager::AbstractItemPtr;
using TaskManager::AbstractGroupableItem;
using TaskManager::GroupManager;
using TaskManager::TaskItem;

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

        Plasma::FrameSvg *itemBackground();
        QPixmap *taskAlphaPixmap(const QSize &size);
        KColorScheme *colorScheme();

        qreal itemLeftMargin() { return m_leftMargin; }
        qreal itemRightMargin() { return m_rightMargin; }
        qreal itemTopMargin() { return m_topMargin; }
        qreal itemBottomMargin() { return m_bottomMargin; }
        void resizeItemBackground(const QSizeF &newSize);

        TaskGroupItem*  rootGroupItem();
        WindowTaskItem* windowItem(TaskPtr);
        TaskGroupItem*  groupItem(GroupPtr);
        AbstractTaskItem* abstractItem(AbstractItemPtr);

        AbstractTaskItem* createAbstractItem(AbstractItemPtr groupableItem);
        TaskGroupItem* createNewGroup(QList <AbstractTaskItem *> members);

        void removeItem(AbstractTaskItem *item);

        TaskManager::GroupManager &groupManager() const;

        Qt::KeyboardModifiers groupModifierKey() const;


signals:
    /**
    * emitted whenever we receive a constraintsEvent
    */
    void constraintsChanged(Plasma::Constraints);

    void desktopChanged(int,int);
    void settingsChanged();

protected slots:
        void configAccepted();
        void themeRefresh();

protected:
        void createConfigurationInterface(KConfigDialog *parent);
        QSizeF sizeHint(Qt::SizeHint which, const QSizeF & constraint = QSizeF()) const;
        void adjustGroupingStrategy();

private slots:
        /**
        * Somthing has changed in the tree of the GroupingStrategy
        */
        void reload();
        void itemRemoved(AbstractGroupableItem*);
        void changeSizeHint(Qt::SizeHint which);
        void dialogGroupingChanged(int index);
        //startupRemoved(TaskManager::AbstractGroupableItem*);

private:
        QHash<TaskPtr,WindowTaskItem*> m_windowTaskItems;
        QHash<GroupPtr,TaskGroupItem*> m_groupTaskItems;
        QHash<StartupPtr,WindowTaskItem*> m_startupTaskItems;
        QHash<AbstractItemPtr,AbstractTaskItem*> m_items;

        WindowTaskItem * createWindowTask(TaskManager::TaskItem* task);
        TaskGroupItem * createTaskGroup(GroupPtr);
        WindowTaskItem *createStartingTask(TaskManager::TaskItem* task);
        void removeStartingTask(StartupPtr);

        bool m_showTooltip;
        Plasma::LayoutAnimator *m_animator;
        QGraphicsLinearLayout *layout;

        Ui::tasksConfig m_ui;
        QTimer m_screenTimer;

        Plasma::FrameSvg *m_taskItemBackground;
        QPixmap *m_taskAlphaPixmap;
        QHash <int,QPixmap*> m_taskAlphaPixmapList;
        KColorScheme *m_colorScheme;
        qreal m_leftMargin;
        qreal m_topMargin;
        qreal m_rightMargin;
        qreal m_bottomMargin;

        TaskGroupItem *m_rootGroupItem;
        GroupManager *m_groupManager;
        TaskManager::GroupManager::TaskGroupingStrategy m_groupingStrategy;
        bool m_groupWhenFull;
        Qt::KeyboardModifier m_groupModifierKey;

        int m_currentDesktop;

};

#endif
