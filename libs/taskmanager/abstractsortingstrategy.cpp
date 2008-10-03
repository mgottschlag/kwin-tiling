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

#include "abstractsortingstrategy.h"

#include "taskitem.h"
#include "taskgroup.h"
#include "taskmanager.h"

#include <QtAlgorithms>
#include <QList>

#include <KDebug>


namespace TaskManager
{

class AbstractSortingStrategy::Private
{
public:
    Private()
        : type(GroupManager::NoSorting)
    {
    }

    QList<TaskGroup*> managedGroups;
    GroupManager::TaskSortingStrategy type;
};


AbstractSortingStrategy::AbstractSortingStrategy(QObject *parent)
    :QObject(parent),
    d(new Private)
{

}

AbstractSortingStrategy::~AbstractSortingStrategy()
{
    delete d;
}

GroupManager::TaskSortingStrategy AbstractSortingStrategy::type() const
{
    return d->type;
}

void AbstractSortingStrategy::setType(GroupManager::TaskSortingStrategy type)
{
    d->type = type;
}

void AbstractSortingStrategy::handleGroup(TaskGroup *group)
{
    if (d->managedGroups.contains(group) || !group) {
        return;
    }
    d->managedGroups.append(group);
    disconnect(group, 0, this, 0); //To avoid duplicate connections
   // connect(group, SIGNAL(changed()), this, SLOT(check()));
    connect(group, SIGNAL(itemAdded(AbstractPtr)), this, SLOT(handleItem(AbstractPtr)));
    connect(group, SIGNAL(destroyed()), this, SLOT(removeGroup())); //FIXME necessary?
    foreach (AbstractPtr item, group->members()) {
        handleItem(item);
    }

}

void AbstractSortingStrategy::removeGroup()
{
    TaskGroup *group = dynamic_cast<TaskGroup*>(sender());
    if (!group) {
	return;
    }
    d->managedGroups.removeAll(group);
}

void AbstractSortingStrategy::handleItem(AbstractPtr item)
{
    if (item->isGroupItem()) {
        handleGroup(qobject_cast<TaskGroup*>(item));
    } else if (!(qobject_cast<TaskItem*>(item))->taskPointer()) { //ignore startup tasks
        return;
    }
    disconnect(item, 0, this, 0); //To avoid duplicate connections
    connect(item, SIGNAL(changed()), this, SLOT(check()));
    check(item);
}

void AbstractSortingStrategy::check(AbstractPtr itemToCheck)
{
    kDebug();
    AbstractPtr item;
    if (!itemToCheck) {
        //return;
        item = dynamic_cast<AbstractPtr>(sender());
    } else {
        item = itemToCheck;
    }

    if (!item) {
        kDebug() << "invalid item";
        return;
    }

    if (!item->isGroupItem()) {
        if (!(qobject_cast<TaskItem*>(item))->taskPointer()) { //ignore startup tasks
            return;
        }
    }

    if (!item->parentGroup()) {
        kDebug() << "No parent group";
        return;
    }
    ItemList sortedList = item->parentGroup()->members();
    sortItems(sortedList);
    int oldIndex = item->parentGroup()->members().indexOf(item);
    int newIndex = sortedList.indexOf(item);
    if (oldIndex != newIndex) {
        TaskGroup* group = item->parentGroup();
        item->parentGroup()->members().move(oldIndex, newIndex);
        group->itemMoved(item);
    }
}

void AbstractSortingStrategy::desktopChanged(int newDesktop)
{
    Q_UNUSED(newDesktop)
}

bool AbstractSortingStrategy::moveItem(AbstractPtr item, int newIndex)
{
    kDebug() << "move to " << newIndex;
    if (!item->parentGroup()) {
        kDebug() << "error";
        return false;
    }
    ItemList &list = item->parentGroup()->members();
    if ((newIndex < 0) || (newIndex >= list.size())) {
        newIndex = list.size();
    }
    
    if (newIndex > list.indexOf(item)) {
        newIndex--; //the index has to be adjusted if we move the item from right to left because the item on the left is removed first
    }
    
    if ((list.size() <= newIndex) || (newIndex < 0)) {
        kDebug() << "index out of bounds";
        return false;
    }
    

    list.move(list.indexOf(item), newIndex);
    kDebug() << "new index " << item->parentGroup()->members().indexOf(item); 
    TaskGroup* group = item->parentGroup();
    group->itemMoved(item);
    return true;
}

} //namespace

#include "abstractsortingstrategy.moc"

