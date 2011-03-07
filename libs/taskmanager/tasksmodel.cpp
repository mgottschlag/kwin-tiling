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

    TasksModel *q;
    QWeakPointer<GroupManager> groupManager;
    int rows;
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
    }

    return QVariant();
}

QModelIndex TasksModel::index(int row, int column, const QModelIndex &parent) const
{
    GroupManager *gm = d->groupManager.data();
    if (!gm || row < 0 || column < 0) {
        return QModelIndex();
    }

    kDebug() << "asking for" << row << column;

    TaskGroup *group = 0;
    if (parent.isValid()) {
        AbstractGroupableItem *item = static_cast<AbstractGroupableItem *>(parent.internalPointer());
        if (item && item->itemType() == GroupItemType) {
            group = static_cast<TaskGroup *>(item);
        }
    } else {
        group = gm->rootGroup();
    }

    if (!group || row >= group->members().count()) {
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

int TasksModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

int TasksModel::rowCount(const QModelIndex &parent) const
{
    TaskGroup *group = 0;
    if (!parent.isValid()) {
        if (d->groupManager) {
            group = d->groupManager.data()->rootGroup();
        }
    } else {
        AbstractGroupableItem *item = static_cast<AbstractGroupableItem *>(parent.internalPointer());
        if (item->itemType() == GroupItemType) {
            group = static_cast<TaskGroup *>(item);
        }
    }

    if (group) {
        return group->members().count();
    }

    return 0;
}

TasksModelPrivate::TasksModelPrivate(TasksModel *model, GroupManager *gm)
    : q(model),
      groupManager(gm),
      rows(0)
{
}

void TasksModelPrivate::populateModel()
{
    q->beginRemoveRows(QModelIndex(), 0, q->rowCount());

    GroupManager *gm = groupManager.data();
    kDebug() << gm;
    if (!gm) {
        rows = 0;
        return;
    }

    TaskGroup *root = gm->rootGroup();
    rows = root->members().count();
    kDebug() << "we gots ourselves rows #:" << rows;
    QModelIndex idx;
    populate(idx, root);
}

void TasksModelPrivate::populate(const QModelIndex &parent, TaskGroup *group)
{
    typedef QPair<QModelIndex, TaskGroup *> idxGroupPair;
    QList<idxGroupPair> childGroups;
    q->beginInsertRows(parent, 0, group->members().count());
    int i = 0;
    foreach (AbstractGroupableItem *item,  group->members()) {
        q->insertRow(i, parent);
        QModelIndex idx(q->index(i, 0, parent));
        QMap<int, QVariant> data;
        data.insert(Qt::DisplayRole, item->name());
        data.insert(Qt::DecorationRole, item->icon());
        q->setItemData(idx, data);
        if (item->itemType() == GroupItemType) {
            childGroups << idxGroupPair(idx, static_cast<TaskGroup *>(item));
        }
    }
    q->endInsertRows();

    foreach (const idxGroupPair &pair, childGroups) {
        populate(pair.first, pair.second);
    }
}

}

#include "tasksmodel.moc"

