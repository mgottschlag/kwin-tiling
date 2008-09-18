/*****************************************************************

Copyright 2008 Christian Mollekopf <robertknight@gmail.com>

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

#ifndef GROUPMANAGER_H
#define GROUPMANAGER_H

#include <QObject>

#include "abstractgroupableitem.h"
#include "task.h"
#include "taskitem.h"
#include <taskmanager/taskmanager_export.h>

namespace TaskManager
{

class AbstractSortingStrategy;
class AbstractGroupingStrategy;

/**
 * Manages the grouping stuff. It doesn't know anything about grouping and sorting itself, this is done in the grouping and sorting strategies.
 */
class TASKMANAGER_EXPORT GroupManager: public QObject
{

Q_OBJECT
public:
    GroupManager(QObject *parent);
    ~GroupManager();


    /**
    * Returns a group which contains all items and subgroups.
    * Visualisations should be based on this.
    */
    GroupPtr rootGroup();

    /**
    * Strategy used to Group new items
    */
    enum TaskGroupingStrategy
    {
        NoGrouping = 0,
        ManualGrouping = 1, //Allow manual grouping
        ProgramGrouping = 2 //Group automatically  same programs
    };

    TaskGroupingStrategy groupingStrategy() const;
    void setGroupingStrategy(TaskGroupingStrategy);
    AbstractGroupingStrategy* groupingStrategyPointer() const;


    /**
    * How the task are ordered
    */
    enum TaskSortingStrategy
    {
        NoSorting = 0,
        ManualSorting = 1,
        AlphaSorting = 2,
        DesktopSorting = 3
    };

    TaskSortingStrategy sortingStrategy() const;
    void setSortingStrategy(TaskSortingStrategy);
    AbstractSortingStrategy* sortingStrategyPointer() const;

    bool showOnlyCurrentScreen() const;
    void setShowOnlyCurrentScreen(bool);

    bool showOnlyCurrentDesktop() const;
    void setShowOnlyCurrentDesktop(bool);


    /** 
     * Functions to call if the user wants to do something manually, the strategy allows or refuses the request
     */
    bool manualGroupingRequest(AbstractGroupableItem* taskItem, TaskGroup* groupItem);
    bool manualGroupingRequest(ItemList items);

    bool manualSortingRequest(AbstractGroupableItem* taskItem, int newIndex);

    /**
     * The Visualization is responsible to update the screen number the visualization is currently on, 
     * when the getCurrentScreen() signal is emitted
     */
    void setScreen(int screen);

signals:
    /** Signal that the rootGroup has to be reloaded in the visualization */
    void reload();
    /** Signal that the item is no longer available */
    void itemRemoved(AbstractGroupableItem*);
    /** Signal that the item is no longer available */
   // void startupRemoved(AbstractGroupableItem*);
    /** If this signal is emitted, the visualization is expected to update the current screen with setScreen(int);*/
    void getCurrentScreen();

public slots:
    /**
    * Slots for newly added tasks from TaskManager
    */
    bool add(TaskPtr);
    void remove(TaskPtr);

    void add(StartupPtr);
    void remove(StartupPtr);

    /**
    *   listen to the relevant signals of taskmanager
    */
    void reconnect();

private slots:

    /**
    * Keep track of changes in Taskmanager
    */
    void currentDesktopChanged(int);
    void taskChangedDesktop(TaskPtr);
    void addAttentionTask();
    void windowChangedGeometry(TaskPtr task);

    void checkScreenChange();

    void itemDestroyed();

private:
    /** reload all tasks from TaskManager */
    void reloadTasks();

    class Private;
    Private * const d;

};
}
#endif
