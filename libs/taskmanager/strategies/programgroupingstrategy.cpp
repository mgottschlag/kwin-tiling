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

#include "programgroupingstrategy.h"

#include <QAction>
#include <QPointer>

#include <KDebug>
#include <KLocale>
#include <KConfig>
#include <KConfigGroup>

#include "abstractgroupingstrategy.h"
#include "groupmanager.h"

namespace TaskManager
{

class ProgramGroupingStrategy::Private
{
public:
    Private()
        :editableGroupProperties(AbstractGroupingStrategy::None)
    {
    }

    AbstractGroupingStrategy::EditableGroupProperties editableGroupProperties;
    QPointer<AbstractGroupableItem> tempItem;
    QStringList blackList; //Programs in this list should not be grouped
};


ProgramGroupingStrategy::ProgramGroupingStrategy(GroupManager *groupManager)
    :AbstractGroupingStrategy(groupManager),
     d(new Private)
{
    setType(GroupManager::ProgramGrouping);

    KConfig groupBlacklist( "taskbargroupblacklistrc", KConfig::NoGlobals );
    KConfigGroup blackGroup( &groupBlacklist, "Blacklist" );
    d->blackList = blackGroup.readEntry( "Applications", QStringList() );
}

ProgramGroupingStrategy::~ProgramGroupingStrategy()
{
    KConfig groupBlacklist("taskbargroupblacklistrc", KConfig::NoGlobals);
    KConfigGroup blackGroup(&groupBlacklist, "Blacklist");
    blackGroup.writeEntry("Applications", d->blackList);
    blackGroup.config()->sync();

    delete d;
}

QList<QAction*> ProgramGroupingStrategy::strategyActions(QObject *parent, AbstractGroupableItem *item)
{
    QAction *a = new QAction(parent);
    QString name = className(item);
    if (d->blackList.contains(name)) {
        a->setText(i18n("Allow This Program to Be Grouped"));
    } else {
        a->setText(i18n("Do Not Allow This Program to Be Grouped"));
    }
    connect(a, SIGNAL(triggered()), this, SLOT(toggleGrouping()));

    QList<QAction*> actionList;
    actionList.append(a);
    d->tempItem = item;
    return actionList;
}

QString ProgramGroupingStrategy::className(AbstractGroupableItem *item)
{
    QString name;
    if (item->isGroupItem()) { //maybe add the condition that the subgroup was created by programGrouping
        TaskGroup *group = qobject_cast<TaskGroup*>(item);
        TaskItem *task = qobject_cast<TaskItem*>(group->members().first()); //There are only TaskItems in programGrouping groups
        return task->task()->classClass();
    }

    return (qobject_cast<TaskItem*>(item))->task()->classClass();
}

void ProgramGroupingStrategy::toggleGrouping()
{
    if (!d->tempItem) {
        return;
    }

    QString name = className(d->tempItem);

    if (d->blackList.contains(name)) {
        d->blackList.removeAll(name);
        if (d->tempItem->isGroupItem()) {
            foreach (AbstractGroupableItem *item, (qobject_cast<TaskGroup*>(d->tempItem))->members()) {
                handleItem(item);
            }
        } else {
            handleItem(d->tempItem);
        }
    } else {
        d->blackList.append(name);
        if (d->tempItem->isGroupItem()) {
            closeGroup(qobject_cast<TaskGroup*>(d->tempItem));
        } else if (rootGroup()) {
            rootGroup()->add(d->tempItem);
        }
    }

    d->tempItem = 0;
}

void ProgramGroupingStrategy::handleItem(AbstractGroupableItem *item)
{
    GroupPtr root = rootGroup();

    if (!root) {
        return;
    }

    if (item->isGroupItem()) {
        //kDebug() << "item is groupitem";
        root->add(item);
        return;
    } else if (d->blackList.contains((qobject_cast<TaskItem*>(item))->task()->classClass())) {
        //kDebug() << "item is in blacklist";
        root->add(item);
        return;
    }

    TaskItem *task = dynamic_cast<TaskItem*>(item);
    if (task && !programGrouping(task, root)) {
        //kDebug() << "joined rootGroup ";
        root->add(item);
    }
}

bool ProgramGroupingStrategy::programGrouping(TaskItem* taskItem, TaskGroup* groupItem)
{
    //kDebug();
    QHash <QString,AbstractGroupableItem *> itemMap;

    foreach (AbstractGroupableItem *item, groupItem->members()) { //search for an existing group
        if (item->isGroupItem()) { //maybe add the condition that the subgroup was created by programGrouping
            if (programGrouping(taskItem, static_cast<TaskGroup*>(item))) {
                //kDebug() << "joined subGroup";
                return true;
            }
        } else {
            TaskItem *task = static_cast<TaskItem*>(item);
            if (task->task()) { //omit startup tasks
                QString name = task->task()->classClass();
                itemMap.insertMulti(name,item);
            }
        }
    }

    if (!itemMap.values().contains(taskItem)) {
        itemMap.insertMulti(taskItem->task()->classClass(), taskItem);
    }

    QString name = taskItem->task()->classClass();
    if (itemMap.count(name) >= groupItem->members().count() && !groupItem->isRootGroup()) { //join this group if this is not the rootGroup, otherwise tasks may not be grouped if there arent tasks of any other type, typically on startup
        //kDebug() << "joined this Group";
        groupItem->add(taskItem);
        return true;
    } else if (itemMap.count(name) >= 2) { //create new subgroup with at least 2 other task
        //kDebug() << "create Group";
        QIcon icon = taskItem->task()->icon();
        QList <AbstractGroupableItem *> list(itemMap.values(name));
        TaskGroup* group = createGroup(list);
        group->setName(name);
        group->setColor(Qt::red);
        group->setIcon(icon);
        connect(group, SIGNAL(checkIcon(TaskGroup*)), this, SLOT(updateIcon(TaskGroup*)));
        return true;
    }
    return false;
}

void ProgramGroupingStrategy::checkGroup()
{
    TaskGroup *group = qobject_cast<TaskGroup*>(sender()); 
    if (group) {
        if (group->members().size() < 2) {
            closeGroup(group);
        } else {
            updateIcon(group);
        }
    }
}

void ProgramGroupingStrategy::updateIcon(TaskGroup *group)
{
    foreach (AbstractGroupableItem *taskItem, group->members()) {
        if (!taskItem->icon().isNull()) {
            QIcon icon = taskItem->icon();
            group->setIcon(icon);
            break;
        }
    }
}

}//namespace

#include "programgroupingstrategy.moc"

