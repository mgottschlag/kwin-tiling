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

#ifndef ABSTRACTSORTINGSTRATEGY_H
#define ABSTRACTSORTINGSTRATEGY_H

#include "abstractgroupableitem.h"
#include "groupmanager.h"
#include <QObject>




namespace TaskManager
{

//class GroupManager;

/**
 * Base class for strategies which can be used to
 * automatically sort tasks.
 */
class KDE_EXPORT AbstractSortingStrategy : public QObject
{
    Q_OBJECT
public:
    AbstractSortingStrategy(QObject *parent);
    virtual ~AbstractSortingStrategy();

    /** Adds group under control of sorting strategy. all added supgroups are automatically added to this sortingStrategy*/
    void handleGroup(TaskGroup *);

     /** Returns the strategy type */
    virtual GroupManager::TaskSortingStrategy type() const = 0;

    /** DesktopChanges time to backup any needed data */
    virtual void desktopChanged(int newDesktop){};

    /** Moves Item to new index*/
    bool moveItem(AbstractPtr, int);

protected slots:
     /** Handles a new item*/
    virtual void handleItem(AbstractPtr);
     /** Checks if the order has to be updated must be connect to AbstractGroupableItem* */
    void check(AbstractPtr item = 0);
//void removeGroup();

private:
    /** Sorts list of items according to startegy. Has to be reimlemented by every sortingStrategy*/
    virtual void sortItems(ItemList&) = 0;

    class Private;
    Private * const d;
};

/** Sorts the tasks alphabetically by programname found in Task::classClass()*/
class KDE_EXPORT AlphaSortingStrategy : public AbstractSortingStrategy
{
    Q_OBJECT
public:
    AlphaSortingStrategy(QObject *parent);
    ~AlphaSortingStrategy(){};
     
     /** Returns the strategy type */
    GroupManager::TaskSortingStrategy type() const;

private:
    /** Sorts list of items*/
    void sortItems(ItemList&);
};

typedef QHash <AbstractGroupableItem*, int> itemHashTable;
typedef QHash <int, itemHashTable*> desktopHashTable;

/**
* Manual Sorting
* If showAllDesktops is enabled the position of the tasks logically changes on all desktops
* If showAllDesktops is disabled the position only changes per virtual desktop even
* if the task is on all desktops
*/

class KDE_EXPORT ManualSortingStrategy : public AbstractSortingStrategy
{
    Q_OBJECT
public:
    ManualSortingStrategy(GroupManager *parent);
    ~ManualSortingStrategy();

    /*/** Adds group under control of sorting strategy*/
    //void handleGroup(TaskGroup *);

     /** Returns the strategy type */
    GroupManager::TaskSortingStrategy type() const;

    /** DesktopChanges, time to backup any needed data */
    void desktopChanged(int newDesktop);

protected slots:
     /** Handles a new item*/
    virtual void handleItem(AbstractPtr);

private:
    /** Sorts list of items*/
    void sortItems(ItemList&);
   
    void storePositions(TaskGroup *group);

    class Private;
    Private * const d;
};


} // TaskManager namespace
#endif
