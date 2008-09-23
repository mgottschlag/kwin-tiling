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

#include "groupmanager.h"

#include <QList>
#include <KDebug>
#include <QTimer>


#include "abstractsortingstrategy.h"
#include "startup.h"
#include "task.h"
#include "taskitem.h"
#include "taskgroup.h"
#include "taskmanager.h"
#include "strategies/alphasortingstrategy.h"
#include "strategies/programgroupingstrategy.h"
#include "strategies/manualgroupingstrategy.h"
#include "strategies/manualsortingstrategy.h"

namespace TaskManager
{

class GroupManager::Private
{
public:
    Private()
        :rootGroup(0),
         sortingStrategy(NoSorting),
         groupingStrategy(NoGrouping),
         abstractGroupingStrategy(0),
         abstractSortingStrategy(0),
         currentScreen(-1),
         showOnlyCurrentDesktop(false),
         showOnlyCurrentScreen(false)
    {
    }

    QHash<TaskPtr, TaskItem*> itemList; //holds all tasks of the Taskmanager
    QHash<StartupPtr, TaskItem*> startupList;
    TaskGroup *rootGroup; //the current layout
    TaskSortingStrategy sortingStrategy;
    TaskGroupingStrategy groupingStrategy;
    AbstractGroupingStrategy *abstractGroupingStrategy;
    AbstractSortingStrategy *abstractSortingStrategy;
    int currentScreen;
    QTimer screenTimer;
    QList<TaskPtr> geometryTasks;
    bool showOnlyCurrentDesktop : 1;
    bool showOnlyCurrentScreen : 1;
};




GroupManager::GroupManager(QObject *parent)
    :QObject(parent),
     d(new Private)
{
    connect(TaskManager::self(), SIGNAL(taskAdded(TaskPtr)), this, SLOT(add(TaskPtr)));
    connect(TaskManager::self(), SIGNAL(taskRemoved(TaskPtr)), this, SLOT(remove(TaskPtr)));
    connect(TaskManager::self(), SIGNAL(startupAdded(StartupPtr)), this, SLOT(add(StartupPtr)));
    connect(TaskManager::self(), SIGNAL(startupRemoved(StartupPtr)), this, SLOT(remove(StartupPtr)));
    d->rootGroup = new TaskGroup(this, "RootGroup", Qt::transparent);
    //reloadTasks();
    d->screenTimer.setSingleShot(true);
    d->screenTimer.setInterval(300);
    connect(&d->screenTimer, SIGNAL(timeout()), this, SLOT(checkScreenChange()));

    kDebug();
}

GroupManager::~GroupManager()
{
    delete d->abstractSortingStrategy;
    delete d->abstractGroupingStrategy;
    delete d->rootGroup;
    delete d;
}

void GroupManager::reloadTasks() //TODO optimize: only remove what really is removed readd the rest to update grouping. if (!add(task)){remove(task);} an
{
    kDebug() << "number of tasks available " << TaskManager::self()->tasks().size();
   /* foreach(TaskPtr task, d->itemList.keys()) { //Remove all tasks
        remove(task);
    }*/
    //d->rootGroup->clear();
    QList <TaskPtr> taskList = TaskManager::self()->tasks().values();
    foreach(TaskPtr task, taskList) { //Add all existing tasks
        if ((!task->isOnCurrentDesktop() && showOnlyCurrentDesktop()) ||
            (!task->isOnScreen(d->currentScreen) && showOnlyCurrentScreen())) {
            connect(task.data(),SIGNAL(changed()),
                            this, SLOT(addAttentionTask()));
        }
        if (!add(task)) {
	    remove(task); //remove what isn't needed anymore
	}
	taskList.removeAll(task);
    }
    foreach(TaskPtr task, taskList) { //Remove the remaining
        remove(task);
    }

    emit reload();
}

void GroupManager::add(StartupPtr task)
{
    kDebug();
    TaskItem *item;
    if (!d->startupList.contains(task)) {
        item = new TaskItem(this,task);
        d->startupList.insert(task, item); 
    }
    d->rootGroup->add(item);
}

void GroupManager::remove(StartupPtr task)
{
    kDebug() << "startup";
    AbstractPtr item = d->startupList.take(task);
    if (!item) {
        kDebug() << "invalid item";
        return;
    }

    if (item->parentGroup()) {
        item->parentGroup()->remove(item);
    }
    emit itemRemoved(item);

}

bool GroupManager::add(TaskPtr task)
{
    kDebug();
    /*kDebug() << task->visibleName();
    kDebug() <<  task->visibleNameWithState();
    kDebug() <<  task->name();
    kDebug() <<  task->className();
    kDebug() <<  task->classClass();*/

    //Go through all filters wether the task should be displayed or not

    if (!task->showInTaskbar()) {
        kDebug() << "Dont show in taskbar";
        return false;
    }

    if (showOnlyCurrentDesktop() && !task->isOnCurrentDesktop() && !task->demandsAttention()) {
        kDebug() << "Not on this desktop and showOnlyCurrentDesktop";
        return false;
    }

    if (showOnlyCurrentScreen() && !task->isOnScreen(d->currentScreen) && !task->demandsAttention()) {//FIXME implement
        kDebug() << "Not on this screen and showOnlyCurrentScreen";
        return false;
    }

    NET::WindowType type = task->info().windowType(NET::NormalMask | NET::DialogMask |
                                                   NET::OverrideMask | NET::UtilityMask);
    if (type == NET::Utility) {
        kDebug() << "skipping utility window" << task->name();
        return false;
    }

        //TODO: should we check for transiency? if so the following code can detect it.
    /*
        QHash <TaskPtr, TaskItem*>::iterator it = d->itemList.begin();

        while (it != d->itemList.end()) {
            TaskItem *item = it.value();
            if (item->taskPointer()->hasTransient(task->window())) {
                kDebug() << "TRANSIENT TRANSIENT TRANSIENT!";
            }
            ++it;
        }
    */


    //Ok the Task should be displayed
    TaskItem *item = 0;
    if (!d->itemList.contains(task)) {
        foreach (StartupPtr startup, d->startupList.keys()) { //Lookout for existing startuptask of this task
            if (startup->matchesWindow(task->window())) {
                kDebug() << "startup task";
                item = d->startupList.take(startup);
                item->setTaskPointer(task);
                break;
            }
        }
        if (!item) {
            item = new TaskItem(this,task);
        }
        connect(item, SIGNAL(destroyed()), this, SLOT(itemDestroyed()));
        d->itemList.insert(task, item); 
    } else {
        item = d->itemList.value(task); //we add it again so the group is evaluated again
    }

    //Find a fitting group for the task with GroupingStrategies
    if (d->abstractGroupingStrategy && !task->demandsAttention()) { //do not group attetion tasks
        d->abstractGroupingStrategy->handleItem(item);
    } else {
        d->rootGroup->add(item);
    }
    return true;
}

 /** Adds a windowTaskItem that is demanding attention to the taskbar if it is not currently shown and is not on the current desktop.
*This funtion applies when the showOnlyCurrentDesktop or showOnlyCurrentScreen switch is set. 
*/
void GroupManager::addAttentionTask()
{
    TaskPtr task;
    task.attach(qobject_cast<Task*>(sender()));
    if (task->demandsAttention() && !d->itemList.keys().contains(task)) {
        add(task);
    }
}

void GroupManager::remove(TaskPtr task)
{
    kDebug() << "remove: " << task->visibleName();
    TaskItem *item = d->itemList.value(task);
    if (!item) {
        kDebug() << "invalid item";
        return;
    }
    //GroupPtr group = d->directory.take(item);
    if (item->parentGroup()) {
        item->parentGroup()->remove(item);
    }
    emit itemRemoved(item);
    //the item must exist as long as the TaskPtr does because of activate calls so don't delete the item here, it will delete itself. We keep it in the itemlist because it may return
    
}

void GroupManager::itemDestroyed()
{
    TaskItem *taskItem = qobject_cast<TaskItem*>(sender());
    TaskItem *item = d->itemList.take(d->itemList.key(taskItem));
    if (!item) {
        kDebug() << "invalid item";
        return;
    }
    disconnect(item, 0, this, 0);
}


bool GroupManager::manualGroupingRequest(AbstractGroupableItem* item, TaskGroup* groupItem)
{
    kDebug();
    if (d->abstractGroupingStrategy) {
        return d->abstractGroupingStrategy->addItemToGroup(item, groupItem);
    //    kDebug() << d->abstractGroupingStrategy->type() << ManualGrouping;
        /*if (d->abstractGroupingStrategy->type() == ManualGrouping) {
   //         kDebug();
            return (qobject_cast<ManualGroupingStrategy*>(d->abstractGroupingStrategy))->addItemToGroup(item,groupItem);
        }*/
    }
    return false;
}

bool GroupManager::manualGroupingRequest(ItemList items)
{
    kDebug();
    if (d->abstractGroupingStrategy) {
     //   kDebug() << d->abstractGroupingStrategy->type() << ManualGrouping;
        if (d->abstractGroupingStrategy->type() == ManualGrouping) {
      //      kDebug();
            return (qobject_cast<ManualGroupingStrategy*>(d->abstractGroupingStrategy))->groupItems(items);
        }
    }
    return false;
}

bool GroupManager::manualSortingRequest(AbstractGroupableItem* taskItem, int newIndex)
{
    kDebug();
    if (d->abstractSortingStrategy) {
        if (d->abstractSortingStrategy->type() == ManualSorting) {
            return (qobject_cast<ManualSortingStrategy*>(d->abstractSortingStrategy))->moveItem(taskItem, newIndex);
        }
    }
    return false;
}


GroupPtr GroupManager::rootGroup()
{
    return d->rootGroup;
}


void GroupManager::currentDesktopChanged(int newDesktop)
{
    kDebug();
    if (!showOnlyCurrentDesktop()) {
        return;
    }       
    if (d->abstractSortingStrategy) {
        d->abstractSortingStrategy->desktopChanged(newDesktop);
    }
    if (d->abstractGroupingStrategy) {
        d->abstractGroupingStrategy->desktopChanged(newDesktop);
    }
    reloadTasks();
}

/* On deskto change this signal is first emmitted for all tasks on this desktop and TaskManager::self()->currentDesktop() is also still the old desk, then currentDesktopChanged is emitted and in the end this signal is
emitted for every task on the new desktop. For tasks on all desktops this signal isn't emitted at all;
*/
void GroupManager::taskChangedDesktop(TaskPtr task)
{
    kDebug() << task->visibleName() << "on" << TaskManager::self()->currentDesktop(); 
    if (!task->isOnCurrentDesktop() && !task->demandsAttention()) {
        kDebug() << "remove(task);";
        remove(task);
    } else if (!d->itemList.contains(task)) {
        kDebug() << "add(task);";
        add(task);
    }
}

void GroupManager::setScreen(int screen)
{
    kDebug() << "new Screen: " << screen;
    d->currentScreen = screen;
}


void GroupManager::windowChangedGeometry(TaskPtr task)
{
    kDebug();
    if (!d->geometryTasks.contains(task)) {
        d->geometryTasks.append(task);
    }

    if (!d->screenTimer.isActive()) {
        emit getCurrentScreen();
        d->screenTimer.start();
    }
}


void GroupManager::checkScreenChange()
{
    kDebug();
    foreach (const TaskPtr &task, d->geometryTasks) {
        if (!task->isOnScreen(d->currentScreen) && !task->demandsAttention()) {
            remove(task);
        } else if (!d->itemList.contains(task)) {
            add(task);
        }
    }
    d->geometryTasks.clear();
}


void GroupManager::reconnect()
{
    kDebug();
    disconnect(TaskManager::self(), SIGNAL(desktopChanged(int)),
               this, SLOT(currentDesktopChanged(int)));
    disconnect(TaskManager::self(), SIGNAL(windowChanged(TaskPtr)),
               this, SLOT(taskChangedDesktop(TaskPtr)));

    if (showOnlyCurrentDesktop()) {
        // listen to the relevant task manager signals
        connect(TaskManager::TaskManager::self(), SIGNAL(desktopChanged(int)),
                this, SLOT(currentDesktopChanged(int)));

        connect(TaskManager::self(), SIGNAL(windowChanged(TaskPtr)),
                this, SLOT(taskChangedDesktop(TaskPtr)));
    }

    disconnect(TaskManager::TaskManager::self(), SIGNAL(windowChangedGeometry(TaskPtr)),
               this, SLOT(windowChangedGeometry(TaskPtr))); //FIXME: Needs to be implemented

    if (showOnlyCurrentScreen()) {
        // listen to the relevant task manager signals
        connect(TaskManager::TaskManager::self(), SIGNAL(windowChangedGeometry(TaskPtr)),
                this, SLOT(windowChangedGeometry(TaskPtr)));
        TaskManager::TaskManager::self()->trackGeometry();
    }

    reloadTasks();
}

bool GroupManager::showOnlyCurrentScreen() const
{
    return d->showOnlyCurrentScreen;
}

void GroupManager::setShowOnlyCurrentScreen(bool state)
{
    d->showOnlyCurrentScreen = state;
}

bool GroupManager::showOnlyCurrentDesktop() const
{
    return d->showOnlyCurrentDesktop;
}

void GroupManager::setShowOnlyCurrentDesktop(bool state)
{
    d->showOnlyCurrentDesktop = state;
}

GroupManager::TaskSortingStrategy GroupManager::sortingStrategy() const
{
    return d->sortingStrategy;
}

AbstractSortingStrategy* GroupManager::taskSorter() const
{
    return d->abstractSortingStrategy;
}

void GroupManager::setSortingStrategy(TaskSortingStrategy sortOrder)
{
    kDebug() << sortOrder;

    if (d->abstractSortingStrategy) {
        if (d->abstractSortingStrategy->type() == sortOrder){
            return;
        } else {
            d->abstractSortingStrategy->deleteLater();
        }
    }

    switch (sortOrder) {
        case NoSorting: //manual and no grouping result both in non automatic grouping
            d->abstractSortingStrategy = 0;
            break;
        case ManualSorting:
            d->abstractSortingStrategy = new ManualSortingStrategy(this);
            d->abstractSortingStrategy->handleGroup(d->rootGroup);
            break;

        case AlphaSorting:
            d->abstractSortingStrategy = new AlphaSortingStrategy(this);
            d->abstractSortingStrategy->handleGroup(d->rootGroup);
            break;

        case DesktopSorting:
            kDebug() << "Strategy not implemented";
            //d->abstractSortingStrategy = new DesktopSortingStrategy(this);
            break;

        default:
            kDebug() << "Invalid Strategy";
            d->abstractSortingStrategy = 0;
    }
    d->sortingStrategy = sortOrder;
    reloadTasks();
}

GroupManager::TaskGroupingStrategy GroupManager::groupingStrategy() const
{
    return d->groupingStrategy;
}

AbstractGroupingStrategy* GroupManager::taskGrouper() const
{
    return d->abstractGroupingStrategy;
}

void GroupManager::setGroupingStrategy(TaskGroupingStrategy strategy)
{
    kDebug() << strategy;

    if (d->abstractGroupingStrategy) {
        if (d->abstractGroupingStrategy->type() == strategy){
            return;
        } else {
            d->abstractGroupingStrategy->deleteLater();
        }
    }

    switch (strategy) {
        case NoGrouping:
            d->abstractGroupingStrategy = 0;
            break;
        case ManualGrouping:
            d->abstractGroupingStrategy = new ManualGroupingStrategy(this);
            break;

        case ProgramGrouping:
            d->abstractGroupingStrategy = new ProgramGroupingStrategy(this);
            break;

        default:
            kDebug() << "Strategy not implemented";
            d->abstractGroupingStrategy = 0;
    }
    d->groupingStrategy = strategy;
    reloadTasks();
}


} // TaskManager namespace

#include "groupmanager.moc"

