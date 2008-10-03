/*****************************************************************

Copyright 2008 Christian Mollekopf <chrigi_1@hotmail.com>

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
#include "taskactions_p.h"

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


QAction *standardGroupableAction(GroupableAction action, AbstractPtr item, QObject *parent, int desktop)
{
    Q_ASSERT(item);

    switch (action) {
        case MaximizeAction:
            return new MaximizeActionImpl(parent, item);
            break;
        case MinimizeAction:
            return new MinimizeActionImpl(parent, item);
            break;
        case ToCurrentDesktopAction:
            return new ToCurrentDesktopActionImpl(parent, item);
            break;
        case ToDesktopAction:
            return new ToDesktopActionImpl(parent, item, desktop);
            break;
        case ShadeAction:
            return new ShadeActionImpl(parent, item);
            break;
        case CloseAction:
            return new CloseActionImpl(parent, item);
            break;
        case ViewFullscreenAction:
            return new ViewFullscreenActionImpl(parent, item);
            break;
        case KeepBelowAction:
            return new KeepBelowActionImpl(parent, item);
            break;
    }

    return 0;
}

QAction* standardTaskAction(TaskAction action, TaskItem *task, QObject *parent)
{
    Q_ASSERT(task);

    switch (action) {
        case ResizeAction:
            return new ResizeActionImpl(parent, task);
            break;
        case MoveAction:
            return new MoveActionImpl(parent, task);
            break;
    }

    return 0;
}

QAction* standardGroupingAction(GroupingAction action, AbstractPtr item, GroupManager *strategy, QObject *parent)
{
    Q_ASSERT(item);
    Q_ASSERT(strategy);

    switch (action) {
        case LeaveGroupAction:
            return new LeaveGroupActionImpl(parent, item, strategy);
            break;
    }

    return 0;
}

MinimizeActionImpl::MinimizeActionImpl(QObject *parent, AbstractPtr item)
    : QAction(parent)
{
    connect(this, SIGNAL(triggered()), item, SLOT(toggleMinimized()));
    setText(i18n("Mi&nimize"));
    setCheckable(true);
    setChecked(item->isMinimized());
    setEnabled(item->isActionSupported(NET::ActionMinimize));
}


MaximizeActionImpl::MaximizeActionImpl(QObject *parent, AbstractPtr item)
    : QAction(parent)
{
    connect(this, SIGNAL(triggered()), item, SLOT(toggleMaximized()));
    setText(i18n("Ma&ximize"));
    setCheckable(true);
    setChecked(item->isMaximized());
    setEnabled(item->isActionSupported(NET::ActionMax));
}

ShadeActionImpl::ShadeActionImpl(QObject *parent, AbstractPtr item)
    : QAction(parent)
{
    connect(this, SIGNAL(triggered()), item, SLOT(toggleShaded()));
    setText(i18n("&Shade"));
    setCheckable(true);
    setChecked(item->isShaded());
    setEnabled(item->isActionSupported(NET::ActionShade));
}

ResizeActionImpl::ResizeActionImpl(QObject *parent, TaskItem* item)
    : QAction(parent)
{
    TaskPtr task = item->taskPointer();
    connect(this, SIGNAL(triggered()), task.data(), SLOT(resize()));
    setText(i18n("Re&size"));
    setEnabled(item->isActionSupported(NET::ActionResize));
}

MoveActionImpl::MoveActionImpl(QObject *parent, TaskItem* item)
    : QAction(parent)
{   
    TaskPtr task = item->taskPointer();
    connect(this, SIGNAL(triggered()), task.data(), SLOT(move()));
    setText(i18n("&Move"));
    setIcon(KIcon("transform-move"));
    setEnabled(item->isActionSupported(NET::ActionMove));
}

CloseActionImpl::CloseActionImpl(QObject *parent, AbstractPtr item)
    : QAction(parent)
{
    connect(this, SIGNAL(triggered()), item, SLOT(close()));
    setText(i18n("&Close"));
    setIcon(KIcon("window-close"));
    setEnabled(item->isActionSupported(NET::ActionClose));
}


ToCurrentDesktopActionImpl::ToCurrentDesktopActionImpl(QObject *parent, AbstractPtr task)
    : QAction(parent),
m_task(task)      
{
    connect(this, SIGNAL(triggered()), this, SLOT(slotToCurrentDesktop()));
    setText(i18n("&To Current Desktop"));
    setEnabled(!task->isOnCurrentDesktop());
}

void ToCurrentDesktopActionImpl::slotToCurrentDesktop() 
{
    m_task->toDesktop(KWindowSystem::currentDesktop());
}



ToDesktopActionImpl::ToDesktopActionImpl(QObject *parent, AbstractPtr task, int desktop)
        : QAction(parent),
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

void ToDesktopActionImpl::slotToDesktop() 
{
    m_task->toDesktop(m_desktop);
}



DesktopsMenu::DesktopsMenu(QWidget *parent, AbstractPtr item)
    : QMenu(parent)
{
    setTitle( i18n("To &Desktop") );
    addAction( new ToDesktopActionImpl(this,item,0) );      //0 means all desktops
    addSeparator();
    for (int i = 1; i <= TaskManager::self()->numberOfDesktops(); i++) {
        addAction( new ToDesktopActionImpl(this,item,i) );
    }
    setEnabled(item->isActionSupported(NET::ActionChangeDesktop));
}

KeepAboveActionImpl::KeepAboveActionImpl(QObject *parent, AbstractPtr item)
    : QAction(parent)
{
    connect(this, SIGNAL(triggered()), item, SLOT(toggleAlwaysOnTop()));
    setText(i18n("Keep &Above Others"));
    setIcon(KIcon("go-up"));
    setCheckable(true);
    setChecked(item->isAlwaysOnTop());
}

KeepBelowActionImpl::KeepBelowActionImpl(QObject *parent, AbstractPtr item)
    : QAction(parent)
{
    connect(this, SIGNAL(triggered()), item, SLOT(toggleKeptBelowOthers()));
    setText(i18n("Keep &Below Others"));
    setIcon(KIcon("go-down"));
    setCheckable(true);
    setChecked(item->isKeptBelowOthers());
}

ViewFullscreenActionImpl::ViewFullscreenActionImpl(QObject *parent, AbstractPtr item)
    : QAction(parent)
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
    addAction(new KeepAboveActionImpl(this, task));
    addAction(new KeepBelowActionImpl(this, task));
    addAction(new ViewFullscreenActionImpl(this, task));
}

LeaveGroupActionImpl::LeaveGroupActionImpl(QObject *parent, AbstractPtr item, GroupManager *strategy)
    : QAction(parent), abstractItem(item), groupingStrategy(strategy)
{
    Q_ASSERT(strategy);
    connect(this, SIGNAL(triggered()), this, SLOT(leaveGroup()));
    setText(i18n("&Leave Group"));
    setIcon(KIcon("window-close"));
    setEnabled(item->isGrouped());
}

void LeaveGroupActionImpl::leaveGroup()
{
    groupingStrategy->manualGroupingRequest(abstractItem,abstractItem->parentGroup()->parentGroup());
}

GroupingStrategyMenu::GroupingStrategyMenu(QWidget *parent, AbstractGroupableItem* task, GroupManager *strategy)
    : QMenu(parent)
{
    Q_ASSERT(task);
    Q_ASSERT(strategy);

    setTitle("Grouping strategy actions");
    if (strategy->taskGrouper()) {
        QList<QAction*> groupingStrategyActions = strategy->taskGrouper()->strategyActions(this, task);
        if (!groupingStrategyActions.empty()) {
            addSeparator();
            foreach (QAction *action, groupingStrategyActions) {
                addAction(action);
            }
        }
    }

}


BasicMenu::BasicMenu(QWidget *parent, TaskItem* task, GroupManager *strategy, bool showAll)
    : QMenu(parent)
{
    Q_ASSERT(task);
    Q_ASSERT(strategy);

    setTitle(task->name());
    setIcon(task->icon());
    addMenu(new AdvancedMenu(this, task));

    if (TaskManager::self()->numberOfDesktops() > 1)
    {
        addMenu(new DesktopsMenu(this, task));
        if (showAll)
        {
            addAction(new ToCurrentDesktopActionImpl(this, task));
        }
    }

    addAction(new MoveActionImpl(this, task));
    addAction(new ResizeActionImpl(this, task));
    addAction(new MinimizeActionImpl(this, task));
    addAction(new MaximizeActionImpl(this, task));
    addAction(new ShadeActionImpl(this, task));

    if (strategy->taskGrouper()) {
        QList<QAction*> groupingStrategyActions = strategy->taskGrouper()->strategyActions(this, task);
        if (!groupingStrategyActions.isEmpty()) {
            addSeparator();
            foreach (QAction *action, groupingStrategyActions) {
                addAction(action);
            }
            // delete groupingStrategyActions;
        }
    }

    /*if (task->isGrouped()) {
        addSeparator();
        addMenu(new BasicMenu(this, task->parentGroup(), strategy, showAll));
    }*/
 
    addSeparator();
    addAction(new CloseActionImpl(this, task));

}

BasicMenu::BasicMenu(QWidget *parent, TaskGroup* group, GroupManager *strategy, bool showAll)
    :QMenu(parent)
{
    Q_ASSERT(group);
    Q_ASSERT(strategy);

    setTitle(group->name());
    setIcon(group->icon());
    addMenu(new AdvancedMenu(this, group));

    if (TaskManager::self()->numberOfDesktops() > 1)
    {
        addMenu(new DesktopsMenu(this, group));
        if (showAll)
        {
            addAction(new ToCurrentDesktopActionImpl(this, group));
        }
    }

//    addAction(new MoveActionImpl(this, group));
//    addAction(new ResizeActionImpl(this, group));
    addAction(new MinimizeActionImpl(this, group));
    addAction(new MaximizeActionImpl(this, group));
    addAction(new ShadeActionImpl(this, group));

    if (strategy->taskGrouper()) {
        QList<QAction*> groupingStrategyActions = strategy->taskGrouper()->strategyActions(this, group);
        if (!groupingStrategyActions.isEmpty()) {
            addSeparator();
            foreach (QAction *action, groupingStrategyActions) {
                addAction(action);
            }
        }
    }

    /*if (group->isGrouped()) {
        addSeparator();
        addMenu(new BasicMenu(parent, group->parentGroup(), strategy, showAll));
    }*/

    addSeparator();
    addAction(new CloseActionImpl(this, group));

}

GroupPopupMenu::GroupPopupMenu(QWidget *parent, TaskGroup *group, GroupManager *groupManager, bool showAll)
    :QMenu(parent)
{
    setTitle(group->name());
    setIcon(group->icon());
    foreach (AbstractGroupableItem *item, group->members()) {
	if (!item) {
	    kDebug() << "invalid Item";
	    continue;
	}

	if (item->isGroupItem()) {
	    QMenu* menu = new GroupPopupMenu (this, qobject_cast<TaskGroup*>(item), groupManager, false);
	    addMenu(menu);
	} else {
	    QAction* action = new QAction(item->icon(), item->name(), this);
	    connect(action, SIGNAL(triggered(bool)), (qobject_cast<TaskItem*>(item))->taskPointer().data() , SLOT(activateRaiseOrIconify()));
	    addAction(action);
	}
    }
}

} // TaskManager namespace

#include "taskactions.moc"
#include "taskactions_p.moc"

