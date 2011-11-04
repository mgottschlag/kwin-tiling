/*****************************************************************

Copyright 2008 Christian Mollekopf <chrigi_1@hotmail.com>
Copyright 2009 CasperBoemann <cbr@boemann.dk>

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
#include <KService>

#include "abstractgroupingstrategy.h"
#include "groupmanager.h"

namespace TaskManager
{

class KustodianGroupingStrategy::Private
{
public:
    Private()
        : editableGroupProperties(AbstractGroupingStrategy::None) {
    }
    AbstractGroupingStrategy::EditableGroupProperties editableGroupProperties;
};


KustodianGroupingStrategy::KustodianGroupingStrategy(GroupManager *groupManager)
    : AbstractGroupingStrategy(groupManager),
      d(new Private)
{
    setType(GroupManager::KustodianGrouping);

    QStringList defaultApps;
    defaultApps << "dolphin" << "krita" << "konqueror" << "kwrite" << "konsole" << "gwenview" << "kontact" << "konversation" << "amarok" << "kword";
    foreach (const QString & name, defaultApps) {
        QList <AbstractGroupableItem *> list;
        TaskGroup* group = createGroup(list);
        group->setName(name);
        group->setPersistentWithLauncher(true);
        KService::Ptr service = KService::serviceByDesktopName(name);
        if (service && service->isValid()) {
            QIcon icon = KIcon(service->icon());
            group->setIcon(icon);
        }
    }
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

void KustodianGroupingStrategy::handleItem(AbstractGroupableItem *item)
{
    if (!rootGroup()) {
        return;
    }

    if (item->itemType() == GroupItemType) {
        rootGroup()->add(item);
        return;
    }

    TaskItem *task = dynamic_cast<TaskItem*>(item);
    if (task && !programGrouping(task, rootGroup())) {
        QString name = desktopNameFromClassName(task->task()->classClass());
        //kDebug() << "create new subgroup in root as this classname doesn't have a group " << name;

        QList <AbstractGroupableItem *> list;
        list.append(task);
        TaskGroup* group = createGroup(list);
        group->setName(name);
        group->setColor(Qt::red);
        KService::Ptr service = KService::serviceByDesktopName(name);
        if (service && service->isValid()) {
            QIcon icon = KIcon(service->icon());
            group->setIcon(icon);
        }
    }
}

bool KustodianGroupingStrategy::programGrouping(TaskItem* taskItem, TaskGroup* groupItem)
{
    //kDebug();
    QHash <QString, AbstractGroupableItem *> itemMap;

    foreach (AbstractGroupableItem * item, groupItem->members()) { //search for an existing group
        if (item->itemType() == GroupItemType && programGrouping(taskItem, static_cast<TaskGroup*>(item))) {
            //maybe add the condition that the subgroup was created by programGrouping
            //kDebug() << "joined subGroup";
            return true;
        }
    }

    QString name = desktopNameFromClassName(taskItem->task()->classClass());
    if (groupItem->name() == name) { //join this group
        //kDebug() << "joined this Group";
        groupItem->add(taskItem);
        return true;
    } else {
        return false;
    }
}

QString KustodianGroupingStrategy::desktopNameFromClassName(const QString & name)
{
    KService::Ptr service = KService::serviceByDesktopName(name.toLower());
    if (service && service->isValid()) {
        return name.toLower();
    }
    service = KService::serviceByDesktopName(name);
    if (service && service->isValid()) {
        return name;
    }
    service = KService::serviceByDesktopName(name.toLower().remove(QChar(' ')));
    if (service && service->isValid()) {
        return name.toLower().remove(' ');
    }

    return "???";
}

void KustodianGroupingStrategy::checkGroup()
{
    TaskGroup *group = qobject_cast<TaskGroup*>(sender());
    if (!group) {
        return;
    }

    if (group->members().isEmpty() && !group->isPersistentWithLauncher()) {
        closeGroup(group);
    }
}

}//namespace

#include "kustodiangroupingstrategy.moc"

