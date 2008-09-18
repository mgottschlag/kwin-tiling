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




class ManualSortingStrategy::Private
{
public:
    Private()
    {
    }
    GroupManager *groupingStrategy;
    
    itemHashTable *managedItems;
    desktopHashTable *desktops;
    int oldDesktop;
};


ManualSortingStrategy::ManualSortingStrategy(GroupManager *parent)
    :AbstractSortingStrategy(parent),
    d(new Private)
{
    d->groupingStrategy = parent;

    d->desktops = new desktopHashTable();
    //TODO add a screenHashTable
    d->oldDesktop = TaskManager::TaskManager::self()->currentDesktop();

    if (d->groupingStrategy->showOnlyCurrentDesktop()) {
        d->desktops->insert(TaskManager::TaskManager::self()->currentDesktop(), new itemHashTable());
        d->managedItems = d->desktops->value(TaskManager::TaskManager::self()->currentDesktop());
    } else {
        d->desktops->insert(0,new itemHashTable());
        d->managedItems = d->desktops->value(0);
    }
}

ManualSortingStrategy::~ManualSortingStrategy()
{
   // d->managedItems = 0;
    if (d->desktops) {
        foreach(itemHashTable *table, d->desktops->values()) {
            if (table) {
                delete table;       
            }
        }
        delete d->desktops;
    }
    delete d;
}

void ManualSortingStrategy::storePositions(TaskGroup *group)
{
    Q_ASSERT(group);
    foreach (AbstractGroupableItem *item, group->members()) {
        Q_ASSERT(item);
        if (item->isGroupItem()) {
            d->managedItems->insert(item, group->members().indexOf(item));
            storePositions(dynamic_cast<TaskGroup*>(item));
        } else {
            d->managedItems->insert(item, group->members().indexOf(item));
        }
    }
}

//Here we should store all infos about the grouping
void ManualSortingStrategy::desktopChanged(int newDesktop)
{
    //store positions of old desktop
    d->managedItems->clear();
    storePositions(d->groupingStrategy->rootGroup());
    d->desktops->insert(d->oldDesktop, d->managedItems);

    if (d->desktops->contains(newDesktop)) {
        d->managedItems = d->desktops->value(newDesktop);
    } else {
        d->managedItems = new itemHashTable();
    }

    d->oldDesktop = newDesktop;
}

void ManualSortingStrategy::sortItems(ItemList &items)
{
    //TODO get the manually created layout here
    foreach (AbstractPtr item, items) {
       // d->managedItems.insert(item, items.indexOf(item));
  //      handleItem(item);
    }

    kDebug();
    QMap<int, AbstractGroupableItem*> map;
    int i = 1000;
    foreach (AbstractGroupableItem *item, items) {
       // if (item->isGroupItem()) {
        if (d->managedItems->contains(item)) {
            map.insertMulti(d->managedItems->value(item), item);
        } else {//make sure unkwown items are appended
            kDebug() << "item not found in managedItems";
            map.insertMulti(i, item);
            i++;
        }
       /* } else {
            map.insertMulti((dynamic_cast<TaskItem*>(item))->taskPointer()->visibleName(), item);
        }*/
        //handleItem(item);
    }
    items.clear();
    items = map.values();

    /*while (!map.empty()) {
        items.append(map.take(keyList.takeFirst()));
    }*/
}

void ManualSortingStrategy::handleItem(AbstractPtr item)
{
    if (d->managedItems->contains(item)) {
        if (item->isGroupItem()) {
            handleGroup(qobject_cast<TaskGroup*>(item));
        }
        check(item);
    } else {
        Q_ASSERT(item->parentGroup());
        d->managedItems->insert(item, item->parentGroup()->members().indexOf(item));
    }
}


GroupManager::TaskSortingStrategy ManualSortingStrategy::type() const
{
    return GroupManager::ManualSorting;
}



} //namespace