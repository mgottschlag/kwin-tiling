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

#ifndef GROUPMANAGER_H
#define GROUPMANAGER_H

#include "abstractgroupableitem.h"
#include "task.h"
#include "taskitem.h"
#include <QObject>

namespace TaskManager
{

class AbstractSortingStrategy;
class AbstractGroupingStrategy;

/**
 * Manages the grouping stuff. It doesn't know anything about grouping and sorting itself, this is done in the grouping and sorting strategies.
 */
class KDE_EXPORT GroupManager: public QObject
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
