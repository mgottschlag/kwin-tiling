/*****************************************************************

Copyright 2010 Aaron Seigo <aseigo@kde.org

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

*****************************************************************/

#include "tasksmodel.h"

#include "groupmanager.h"
#include "taskgroup.h"

#include <KDebug>

namespace TaskManager
{

class TasksModelPrivate
{
public:
    TasksModelPrivate(TasksModel *model, GroupManager *gm);

    void populateModel();
    void populate(const QModelIndex &parent, TaskGroup *group);
    void itemAdded(AbstractGroupableItem *item);
    void itemAboutToBeRemoved(AbstractGroupableItem *item);
    void itemRemoved(AbstractGroupableItem *item);
    void itemMoved(AbstractGroupableItem *item);
    void groupChanged(::TaskManager::TaskChanges changes);

    int indexOf(AbstractGroupableItem *item);
    void addDelta(TaskGroup *group, int amount);

    TasksModel *q;
    QWeakPointer<GroupManager> groupManager;
    TaskGroup *rootGroup;
    QHash<TaskGroup *, int> deltas;
};

TasksModel::TasksModel(GroupManager *groupManager, QObject *parent)
    : QAbstractItemModel(parent),
      d(new TasksModelPrivate(this, groupManager))
{
    if (groupManager) {
        connect(groupManager, SIGNAL(reload()), this, SLOT(populateModel()));
    }

    d->populateModel();
}

TasksModel::~TasksModel()
{
    delete d;
}

QVariant TasksModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    AbstractGroupableItem *item = static_cast<AbstractGroupableItem *>(index.internalPointer());
    if (!item) {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        return item->name();
    } else if (role == Qt::DecorationRole) {
        return item->icon();
    }

    return QVariant();
}

QModelIndex TasksModel::index(int row, int column, const QModelIndex &parent) const
{
    GroupManager *gm = d->groupManager.data();
    if (!gm || row < 0 || column < 0) {
        return QModelIndex();
    }

    //kDebug() << "asking for" << row << column;

    TaskGroup *group = 0;
    if (parent.isValid()) {
        AbstractGroupableItem *item = static_cast<AbstractGroupableItem *>(parent.internalPointer());
        if (item && item->itemType() == GroupItemType) {
            group = static_cast<TaskGroup *>(item);
        }
    } else {
        group = gm->rootGroup();
    }

    if (!group || row >= group->members().count() - d->deltas.value(group, 0)) {
        return QModelIndex();
    }

    return createIndex(row, column, group->members().at(row));
}

QModelIndex TasksModel::parent(const QModelIndex &idx) const
{
    GroupManager *gm = d->groupManager.data();
    if (!gm) {
        return QModelIndex();
    }

    AbstractGroupableItem *item = static_cast<AbstractGroupableItem *>(idx.internalPointer());
    if (!item) {
        return QModelIndex();
    }

    TaskGroup *group = item->parentGroup();
    if (!group || group == gm->rootGroup()) {
        return QModelIndex();
    }

    int row = 0;
    bool found = false;
    TaskGroup *grandparent = group->parentGroup();
    while (row < grandparent->members().count()) {
        if (group == grandparent->members().at(row)) {
            found = true;
            break;
        }

        ++row;
    }

    if (!found) {
        return QModelIndex();
    }

    return createIndex(row, idx.column(), group);
}

int TasksModel::columnCount(const QModelIndex &) const
{
    return 1;
}

int TasksModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return d->rootGroup->members().count() - d->deltas.value(d->rootGroup, 0);
    }

    AbstractGroupableItem *item = static_cast<AbstractGroupableItem *>(parent.internalPointer());
    if (item->itemType() == GroupItemType) {
        TaskGroup *group = static_cast<TaskGroup *>(item);
        const int rowCount = group->members().count() - d->deltas.value(group, 0);
        kDebug() << "row count for" << group << "with real count of" << group->members().count() <<
            "is" << rowCount;
    }

    return 0;
}

TasksModelPrivate::TasksModelPrivate(TasksModel *model, GroupManager *gm)
    : q(model),
      groupManager(gm),
      rootGroup(0)
{
}

void TasksModelPrivate::populateModel()
{
    GroupManager *gm = groupManager.data();
    kDebug() << gm;
    if (!gm) {
        rootGroup = 0;
        return;
    }


    if (rootGroup != gm->rootGroup()) {
        if (rootGroup) {
            QObject::disconnect(rootGroup, 0, q, 0);
        }

        rootGroup = gm->rootGroup();
        kDebug() << "root group connection" << rootGroup;
    }

    q->beginResetModel();
    const QModelIndex idx;
    populate(idx, rootGroup);
    q->endResetModel();
}

void TasksModelPrivate::populate(const QModelIndex &parent, TaskGroup *group)
{
    if (group->members().isEmpty()) {
        return;
    }

    QObject::connect(group, SIGNAL(itemAdded(AbstractGroupableItem*)), q, SLOT(itemAdded(AbstractGroupableItem*)), Qt::UniqueConnection);
    QObject::connect(group, SIGNAL(itemAboutToBeRemoved(AbstractGroupableItem*)), q, SLOT(itemAboutToBeRemoved(AbstractGroupableItem*)), Qt::UniqueConnection);
    QObject::connect(group, SIGNAL(itemRemoved(AbstractGroupableItem*)), q, SLOT(itemRemoved(AbstractGroupableItem*)), Qt::UniqueConnection);
    QObject::connect(group, SIGNAL(itemPositionChanged(AbstractGroupableItem*)), q, SLOT(itemMoved(AbstractGroupableItem*)), Qt::UniqueConnection);
    QObject::connect(group, SIGNAL(changed(::TaskManager::TaskChanges)), q, SLOT(groupChanged(::TaskManager::TaskChanges)), Qt::UniqueConnection);

    typedef QPair<QModelIndex, TaskGroup *> idxGroupPair;
    QList<idxGroupPair> childGroups;

    int i = 0;
    foreach (AbstractGroupableItem *item,  group->members()) {
        if (item->itemType() == GroupItemType) {
            QModelIndex idx(q->index(i, 0, parent));
            childGroups << idxGroupPair(idx, static_cast<TaskGroup *>(item));
        }
        ++i;
    }

    foreach (const idxGroupPair &pair, childGroups) {
        populate(pair.first, pair.second);
    }
}

int TasksModelPrivate::indexOf(AbstractGroupableItem *item)
{
    Q_ASSERT(item != rootGroup);
    int row = 0;
    //kDebug() << item << item->parentGroup();

    foreach (const AbstractGroupableItem *child, item->parentGroup()->members()) {
        if (child == item) {
            break;
        }

        ++row;
    }

    return row;
}

void TasksModelPrivate::addDelta(TaskGroup *group, int amount)
{
    const int delta = deltas.value(group, 0) + amount;
    kDebug() << "adding" << delta << "to" << group;
    if (delta == 0) {
        deltas.remove(group);
    } else {
        deltas.insert(group, delta);
    }
}

void TasksModelPrivate::itemAdded(AbstractGroupableItem *item)
{
    //kDebug() << item << item->parentGroup();
    TaskGroup *parent = item->parentGroup();
    QModelIndex parentIdx;
    if (parent->parentGroup()) {
        parentIdx = q->createIndex(indexOf(parent), 0, parent);
    }

    const int index = indexOf(item);
    addDelta(parent, 1);
    q->beginInsertRows(parentIdx, index, index);
    addDelta(parent, -1);
    q->endInsertRows();
}

void TasksModelPrivate::itemAboutToBeRemoved(AbstractGroupableItem *item)
{
    kDebug() << item;
    TaskGroup *parent = static_cast<TaskGroup *>(q->sender());//item->parentGroup();
    QModelIndex parentIdx;
    if (parent && parent->parentGroup()) {
        parentIdx = q->createIndex(indexOf(parent), 0, parent);
    }

    const int index = indexOf(item);
    q->beginRemoveRows(parentIdx, index, index);
}

void TasksModelPrivate::itemRemoved(AbstractGroupableItem *item)
{
    q->endRemoveRows();
}

void TasksModelPrivate::itemMoved(AbstractGroupableItem *item)
{
    kDebug() << item;
}

void TasksModelPrivate::groupChanged(::TaskManager::TaskChanges changes)
{
    kDebug() << changes;
}

}

#include "tasksmodel.moc"

