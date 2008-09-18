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

#include "abstractsortingstrategy.h"

#include "taskitem.h"
#include "taskgroup.h"
#include "taskmanager.h"

#include <QMap>
#include <QString>
#include <QtAlgorithms>
#include <QList>

#include <KDebug>


namespace TaskManager
{

class AbstractSortingStrategy::Private
{
public:
    Private()
    {
    }

    QList <TaskGroup*> managedGroups;

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

void AbstractSortingStrategy::handleGroup(TaskGroup *group)
{
    if (d->managedGroups.contains(group) || !group) {
        return;
    }
    d->managedGroups.append(group);
    disconnect(group, 0, this, 0); //To avoid duplicate connections
   // connect(group, SIGNAL(changed()), this, SLOT(check()));
    connect(group, SIGNAL(itemAdded(AbstractPtr)), this, SLOT(handleItem(AbstractPtr)));
    connect(group, SIGNAL(destroyed()), this, SLOT(removeGroup()));
    foreach (AbstractPtr item, group->members()) {
        handleItem(item);
    }

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
    if (newIndex < 0) {
        newIndex = list.size() - 1;
    }

    if ((list.size() <= newIndex) || (newIndex < 0)) {
        kDebug() << "index out of bounds";
        return false;
    }
    if (newIndex >= list.indexOf(item)) {
        newIndex--; //the index has to be adjusted if we move the item from right to left because the item onthe left is removed first
    }
    list.move(list.indexOf(item), newIndex);
    //d->managedItems->insert(item, newIndex);
    kDebug() << "new index " << item->parentGroup()->members().indexOf(item); 
    TaskGroup* group = item->parentGroup();
    group->itemMoved(item);
    return true;
}


AlphaSortingStrategy::AlphaSortingStrategy(QObject *parent)
:AbstractSortingStrategy(parent)
{

}
//Returns true if s1 is less than s2
bool lessThan(const QString &s1, const QString &s2)
{
   // return s1.toLower() < s2.toLower();
    if (s1.localeAwareCompare(s2) < 0) {
        return true;
    }
    return false;
}

void AlphaSortingStrategy::sortItems(ItemList &items)
{   
    kDebug();
    QMap<QString, AbstractGroupableItem*> map;

    foreach (AbstractGroupableItem *item, items) {
        if (item->isGroupItem()) {
            map.insertMulti((dynamic_cast<TaskGroup*>(item))->name(), item);
        } else {
            //map.insertMulti((dynamic_cast<TaskItem*>(item))->taskPointer()->visibleName(), item);
            map.insertMulti((dynamic_cast<TaskItem*>(item))->taskPointer()->classClass(), item); //sort by programname not windowname
        }
    }
    items.clear();

    QList <QString> keyList = map.keys();
    qSort(keyList.begin(), keyList.end(), lessThan);

    while (!map.empty()) {
        items.append(map.take(keyList.takeFirst()));
    }

    //For debug
   /* foreach (AbstractGroupableItem *item, items) {
        if (item->isGroupItem()) {
            kDebug() << (dynamic_cast<TaskGroup*>(item))->name();
        } else {
            kDebug() << (dynamic_cast<TaskItem*>(item))->taskPointer()->visibleName();
        }
    }*/

}


GroupManager::TaskSortingStrategy AlphaSortingStrategy::type() const
{
    return GroupManager::AlphaSorting;
}

} //namespace

#include "abstractsortingstrategy.moc"

