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

#ifndef ABSTRACTSORTINGSTRATEGY_H
#define ABSTRACTSORTINGSTRATEGY_H

#include <QtCore/QObject>

#include <taskmanager/abstractgroupableitem.h>
#include <taskmanager/groupmanager.h>
#include <taskmanager/taskmanager_export.h>

namespace TaskManager
{

//class GroupManager;

/**
 * Base class for strategies which can be used to
 * automatically sort tasks.
 */
class TASKMANAGER_EXPORT AbstractSortingStrategy : public QObject
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
    virtual void desktopChanged(int newDesktop);

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

typedef QHash <AbstractGroupableItem*, int> itemHashTable;
typedef QHash <int, itemHashTable*> desktopHashTable;

} // TaskManager namespace
#endif
