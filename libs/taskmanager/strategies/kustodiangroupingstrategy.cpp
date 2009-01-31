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

#include "kustodiangroupingstrategy.h"

#include <QAction>

#include <KDebug>
#include <KLocale>
#include <KIcon>

#include "abstractgroupingstrategy.h"
#include "groupmanager.h"

namespace TaskManager
{

class KustodianGroupingStrategy::Private
{
public:
    Private()
        :editableGroupProperties(AbstractGroupingStrategy::None)
    {
    }
    GroupManager *groupManager;
    AbstractGroupingStrategy::EditableGroupProperties editableGroupProperties;
};


KustodianGroupingStrategy::KustodianGroupingStrategy(GroupManager *groupManager)
    :AbstractGroupingStrategy(groupManager),
     d(new Private)
{
    d->groupManager = groupManager;
    setType(GroupManager::ProgramGrouping);
}

KustodianGroupingStrategy::~KustodianGroupingStrategy()
{
    delete d;
}

QList<QAction*> KustodianGroupingStrategy::strategyActions(QObject *parent, AbstractGroupableItem *item)
{
    Q_UNUSED(parent);
    Q_UNUSED(item);
    QList<QAction*> actionList;
    return actionList;
}

void KustodianGroupingStrategy::handleItem(AbstractItemPtr item)
{
    if (item->isGroupItem()) {
        d->groupManager->rootGroup()->add(item);
        return;
    }

    TaskItem *task = dynamic_cast<TaskItem*>(item);
    if (task && !programGrouping(task, d->groupManager->rootGroup())) {
        //kDebug() << "joined rootGroup ";
        d->groupManager->rootGroup()->add(item);
    }
}

bool KustodianGroupingStrategy::programGrouping(TaskItem* taskItem, TaskGroup* groupItem)
{
    //kDebug();
    QHash <QString,AbstractItemPtr> itemMap;

    foreach (AbstractItemPtr item, groupItem->members()) { //search for an existing group
        if (item->isGroupItem()) { //maybe add the condition that the subgroup was created by programGrouping
            if (programGrouping(taskItem, static_cast<TaskGroup*>(item))) {
                //kDebug() << "joined subGroup";
                return true;
            }
        }
    }

    QString name = taskItem->task()->classClass();
    if (groupItem->name() == name) { //join this group
        //kDebug() << "joined this Group";
        groupItem->add(taskItem);
        return true;
    } else { //create new subgroup as this task has not been pinned
        //kDebug() << "create Group";
        QIcon icon = KIcon(name.toLower());
        QList <AbstractItemPtr> list;
        list.append(taskItem);
        TaskGroup* group = createGroup(list);
        group->setName(name);
        group->setColor(Qt::red);
        group->setIcon(icon);
        return true;
    }
}

void KustodianGroupingStrategy::checkGroup()
{
    TaskGroup *group = qobject_cast<TaskGroup*>(sender()); 
    if (!group) {
        return;
    }

    if (group->members().size()==0 && true) {
        closeGroup(group);
    }
}

}//namespace

#include "kustodiangroupingstrategy.moc"

