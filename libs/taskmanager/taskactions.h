/*****************************************************************

Copyright (c) 2001 Matthias Elter <elter@kde.org>
Copyright (c) 2001 John Firebaugh <jfirebaugh@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#ifndef TASKACTIONS_H
#define TASKACTIONS_H

// Qt
#include <QMenu>
#include <QList>
#include <QPair>
#include <QAction>

// Own
#include <taskmanager/groupmanager.h>
#include <taskmanager/task.h>
#include <taskmanager/taskgroup.h>
#include <taskmanager/taskitem.h>
#include <taskmanager/taskmanager_export.h>

namespace TaskManager
{


/** Maximize a window or all windows in a group*/
class TASKMANAGER_EXPORT MaximizeAction : public QAction
{
    Q_OBJECT
public:
    MaximizeAction(QObject *parent, AbstractPtr task);
};


/** Minimize a window or all windows in a group*/
class TASKMANAGER_EXPORT MinimizeAction : public QAction
{
    Q_OBJECT
public:
    MinimizeAction(QObject *parent, AbstractPtr task);
};

/** Move window to current desktop*/
class TASKMANAGER_EXPORT ToCurrentDesktopAction : public QAction
{
    Q_OBJECT
public:
    ToCurrentDesktopAction(QObject *parent, AbstractPtr task);
private slots:
    void slotToCurrentDesktop();
private:
    AbstractPtr m_task;
};

/** Shade a window or all windows in a group*/
class TASKMANAGER_EXPORT ShadeAction : public QAction
{
    Q_OBJECT
public:
    ShadeAction(QObject *parent, AbstractPtr task);
};

/** Resize a window or all windows in a group*/
class TASKMANAGER_EXPORT ResizeAction : public QAction
{
    Q_OBJECT
public:
    ResizeAction(QObject *parent, TaskItem* task);
};

/** Move a window or all windows in a group*/
class TASKMANAGER_EXPORT MoveAction : public QAction
{
    Q_OBJECT
public:
    MoveAction(QObject *parent, TaskItem* task);
};

/** Shade a window or all windows in a group*/
class TASKMANAGER_EXPORT CloseAction : public QAction
{
    Q_OBJECT
public:
    CloseAction(QObject *parent, AbstractPtr task);
};

/** Send a Task to a specific Desktop*/
class TASKMANAGER_EXPORT ToDesktopAction : public QAction
{
    Q_OBJECT
public:
    ToDesktopAction(QObject *parent, AbstractPtr task, int desktop);
private slots:
    void slotToDesktop();
private:
    int m_desktop;
    AbstractPtr m_task;
};


/** Set a window or all windows in a group to FullScreen*/
class TASKMANAGER_EXPORT ViewFullscreenAction : public QAction
{
    Q_OBJECT
public:
    ViewFullscreenAction(QObject *parent, AbstractPtr task);
};

/** Keep a Window or all windows in a group above the rest */
class TASKMANAGER_EXPORT KeepAboveAction : public QAction
{
    Q_OBJECT
public:
    KeepAboveAction(QObject *parent, AbstractPtr task);
};

/** Keep a Window or all windows in a group below the rest*/
class TASKMANAGER_EXPORT KeepBelowAction : public QAction
{
    Q_OBJECT
public:
    KeepBelowAction(QObject *parent, AbstractPtr task);
};


/** Leave current Group*/
class TASKMANAGER_EXPORT LeaveGroupAction : public QAction
{
    Q_OBJECT
public:
    LeaveGroupAction(QObject *parent, AbstractPtr task, GroupManager&);

private slots:
    void leaveGroup();

private:
    AbstractPtr abstractItem;
    GroupManager *groupingStrategy;
};

/** Remove Group
class TASKMANAGER_EXPORT RemoveGroupAction : public QAction
{
    Q_OBJECT
public:
    RemoveGroupAction(QObject *parent, AbstractPtr task);
};
*/



/** The ToDesktop menu */
class TASKMANAGER_EXPORT DesktopsMenu : public QMenu
{
    Q_OBJECT
public:
    DesktopsMenu(QWidget *parent, AbstractPtr task);
};

/** Menu with the actions that the groupingStrategy provides*/
class TASKMANAGER_EXPORT GroupingStrategyMenu : public QMenu
{
    Q_OBJECT
public:
    GroupingStrategyMenu(QWidget *parent, AbstractGroupableItem* task, GroupManager &strategy);
};

/** The Advanced menu */
class TASKMANAGER_EXPORT AdvancedMenu : public QMenu
{
    Q_OBJECT
public:
    AdvancedMenu(QWidget *parent, AbstractPtr task);
};

/** The standard menu*/
class TASKMANAGER_EXPORT BasicMenu : public QMenu
{
    Q_OBJECT
public:
    BasicMenu(QWidget *parent, GroupPtr task, GroupManager &strategy, bool showAll);
    BasicMenu(QWidget *parent, TaskItem* task, GroupManager &strategy, bool showAll);
};


} // TaskManager namespace


#endif
