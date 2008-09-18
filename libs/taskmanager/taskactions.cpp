/*****************************************************************

Copyright (c) 2008 Christian Mollekopf <chrigi_1@hotmail.com>

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
#include "taskactions.h"
// Own

#include "taskgroup.h"
#include "task.h"
#include "taskitem.h"
#include "taskmanager.h"
#include "abstractgroupingstrategy.h"

// KDE
#include <kicon.h>
#include <klocale.h>
#include <KDebug>

namespace TaskManager
{

MinimizeAction::MinimizeAction(QObject *parent, AbstractPtr item)
:QAction(parent)      
{
    connect(this, SIGNAL(triggered()), item, SLOT(toggleMinimized()));
    setText(i18n("Mi&nimize"));
    setCheckable(true);
    setChecked(item->isMinimized());
    setEnabled(item->isActionSupported(NET::ActionMinimize));
}


MaximizeAction::MaximizeAction(QObject *parent, AbstractPtr item)
:QAction(parent)      
{
    connect(this, SIGNAL(triggered()), item, SLOT(toggleMaximized()));
    setText(i18n("Ma&ximize"));
    setCheckable(true);
    setChecked(item->isMaximized());
    setEnabled(item->isActionSupported(NET::ActionMax));
}

ShadeAction::ShadeAction(QObject *parent, AbstractPtr item)
:QAction(parent)      
{
    connect(this, SIGNAL(triggered()), item, SLOT(toggleShaded()));
    setText(i18n("&Shade"));
    setCheckable(true);
    setChecked(item->isShaded());
    setEnabled(item->isActionSupported(NET::ActionShade));
}

ResizeAction::ResizeAction(QObject *parent, TaskItem* item)
:QAction(parent)      
{
    TaskPtr task = item->taskPointer();
    connect(this, SIGNAL(triggered()), task.data(), SLOT(resize()));
    setText(i18n("Re&size"));
    setEnabled(item->isActionSupported(NET::ActionResize));
}

MoveAction::MoveAction(QObject *parent, TaskItem* item)
:QAction(parent)      
{   
    TaskPtr task = item->taskPointer();
    connect(this, SIGNAL(triggered()), task.data(), SLOT(move()));
    setText(i18n("&Move"));
    setIcon(KIcon("transform-move"));
    setEnabled(item->isActionSupported(NET::ActionMove));
}

CloseAction::CloseAction(QObject *parent, AbstractPtr item)
:QAction(parent)      
{
    connect(this, SIGNAL(triggered()), item, SLOT(close()));
    setText(i18n("&Close"));
    setIcon(KIcon("window-close"));
    setEnabled(item->isActionSupported(NET::ActionClose));
}


ToCurrentDesktopAction::ToCurrentDesktopAction(QObject *parent, AbstractPtr task)
:QAction(parent),
m_task(task)      
{
    connect(this, SIGNAL(triggered()), this, SLOT(slotToCurrentDesktop()));
    setText(i18n("&To Current Desktop"));
    setEnabled(!task->isOnCurrentDesktop());
}

void ToCurrentDesktopAction::slotToCurrentDesktop() 
{
    m_task->toDesktop(KWindowSystem::currentDesktop());
}



ToDesktopAction::ToDesktopAction(QObject *parent, AbstractPtr task, int desktop)
    :QAction(parent),
     m_desktop(desktop),
     m_task(task)
{
    connect(this, SIGNAL(triggered()), this, SLOT(slotToDesktop()));
    setCheckable(true);
    if (!desktop) { //to All Desktops
        setText(i18n("&All Desktops"));
        setChecked( task->isOnAllDesktops() );
    } else {
        QString name = QString("&%1 %2").arg(desktop).arg(TaskManager::self()->desktopName(desktop).replace('&', "&&"));
        setText(name);
        setChecked( !task->isOnAllDesktops() && task->desktop() == desktop );
    }

}

void ToDesktopAction::slotToDesktop() 
{
    m_task->toDesktop(m_desktop);
}



DesktopsMenu::DesktopsMenu(QWidget *parent, AbstractPtr item)
    :QMenu(parent)
{
    setTitle( i18n("To &Desktop") );
    addAction( new ToDesktopAction(this,item,0) );      //0 means all desktops
    addSeparator();
    for (int i = 1; i <= TaskManager::self()->numberOfDesktops(); i++) {
        addAction( new ToDesktopAction(this,item,i) );
    }
    setEnabled(item->isActionSupported(NET::ActionChangeDesktop));
}

KeepAboveAction::KeepAboveAction(QObject *parent, AbstractPtr item)
:QAction(parent)      
{
    connect(this, SIGNAL(triggered()), item, SLOT(toggleAlwaysOnTop()));
    setText(i18n("Keep &Above Others"));
    setIcon(KIcon("go-up"));
    setCheckable(true);
    setChecked(item->isAlwaysOnTop());
}

KeepBelowAction::KeepBelowAction(QObject *parent, AbstractPtr item)
:QAction(parent)      
{
    connect(this, SIGNAL(triggered()), item, SLOT(toggleKeptBelowOthers()));
    setText(i18n("Keep &Below Others"));
    setIcon(KIcon("go-down"));
    setCheckable(true);
    setChecked(item->isKeptBelowOthers());
}

ViewFullscreenAction::ViewFullscreenAction(QObject *parent, AbstractPtr item)
:QAction(parent)      
{
    connect(this, SIGNAL(triggered()), item, SLOT(toggleFullScreen()));
    setText(i18n("&Fullscreen"));
    setIcon(KIcon("view-fullscreen"));
    setCheckable(true);
    setChecked(item->isFullScreen());
    setEnabled(item->isActionSupported(NET::ActionFullScreen));
}

AdvancedMenu::AdvancedMenu(QWidget *parent, AbstractPtr task)
    :QMenu(parent)
{
    setTitle(i18n("Ad&vanced"));
    addAction(new KeepAboveAction(this, task));
    addAction(new KeepBelowAction(this, task));
    addAction(new ViewFullscreenAction(this, task));
}



LeaveGroupAction::LeaveGroupAction(QObject *parent, AbstractPtr item, GroupManager &strategy)
:QAction(parent), abstractItem(item), groupingStrategy(&strategy)
{
    connect(this, SIGNAL(triggered()), this, SLOT(leaveGroup()));
    setText(i18n("&Leave Group"));
    setIcon(KIcon("window-close"));
    setEnabled(item->grouped());
}

void LeaveGroupAction::leaveGroup()
{
    groupingStrategy->manualGroupingRequest(abstractItem,abstractItem->parentGroup()->parentGroup());
}

GroupingStrategyMenu::GroupingStrategyMenu(QWidget *parent, AbstractGroupableItem* task, GroupManager &strategy)
    :QMenu(parent)
{
    setTitle("Grouping strategy actions");
    if (strategy.groupingStrategyPointer()) {
        QList <QAction*> *groupingStrategyActions = strategy.groupingStrategyPointer()->strategyActions(this, task);
        if (groupingStrategyActions && !groupingStrategyActions->empty()) {
            addSeparator();
            foreach (QAction *action, *groupingStrategyActions) {
                addAction(action);
            }
            // delete groupingStrategyActions;
        }
    }

}


BasicMenu::BasicMenu(QWidget *parent, TaskItem* task, GroupManager &strategy, bool showAll)
    :QMenu(parent)
{
    addMenu(new AdvancedMenu(this, task));

    if (TaskManager::self()->numberOfDesktops() > 1)
    {
        addMenu(new DesktopsMenu(this, task));
        if (showAll)
        {
            addAction(new ToCurrentDesktopAction(this, task));
        }
    }

    addAction(new MoveAction(this, task));
    addAction(new ResizeAction(this, task));
    addAction(new MinimizeAction(this, task));
    addAction(new MaximizeAction(this, task));
    addAction(new ShadeAction(this, task));
 
    if (strategy.groupingStrategyPointer()) {
        QList <QAction*> *groupingStrategyActions = strategy.groupingStrategyPointer()->strategyActions(this, task);
        if (groupingStrategyActions && !groupingStrategyActions->empty()) {
            addSeparator();
            foreach (QAction *action, *groupingStrategyActions) {
                addAction(action);
            }
            // delete groupingStrategyActions;
        }
    }

    if (task->grouped()) {
        addSeparator();
        addMenu(new BasicMenu(this, task->parentGroup(), strategy, showAll));
    }
 
    addSeparator();
    addAction(new CloseAction(this, task));

}

BasicMenu::BasicMenu(QWidget *parent, GroupPtr task, GroupManager &strategy, bool showAll)
    :QMenu(parent)
{
    setTitle(task->name());
    addMenu(new AdvancedMenu(this, task));

    if (TaskManager::self()->numberOfDesktops() > 1)
    {
        addMenu(new DesktopsMenu(this, task));
        if (showAll)
        {
            addAction(new ToCurrentDesktopAction(this, task));
        }
    }

//    addAction(new MoveAction(this, task));
//    addAction(new ResizeAction(this, task));
    addAction(new MinimizeAction(this, task));
    addAction(new MaximizeAction(this, task));
    addAction(new ShadeAction(this, task));
    
    if (strategy.groupingStrategyPointer()) {
        QList <QAction*> *groupingStrategyActions = strategy.groupingStrategyPointer()->strategyActions(this, task);
        if (groupingStrategyActions && !groupingStrategyActions->empty()) {
            addSeparator();
            foreach (QAction *action, *groupingStrategyActions) {
                addAction(action);
            }
            delete groupingStrategyActions;
        }
    }

    if (task->grouped()) {
        addSeparator();
        addMenu(new BasicMenu(parent, task->parentGroup(), strategy, showAll));
    }

    addSeparator();
    addAction(new CloseAction(this, task));

}

} // TaskManager namespace

#include "taskactions.moc"

