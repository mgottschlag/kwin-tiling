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
};

TasksModel::TasksModel(GroupManager *groupManager, QObject *parent)
    : QAbstractItemModel(parent),
      d(new TasksModelPrivate(this, groupManager))
{
    if (groupManager) {
        connect(groupManager, SIGNAL(reload()), this, SLOT(populateModel()));
    }
}

TasksModel::~TasksModel()
{
    delete d;
}

QVariant TasksModel::data(const QModelIndex &index, int role) const
{
    return QVariant();
}

QModelIndex TasksModel::index(int row, int column, const QModelIndex &parent) const
{
    return QModelIndex();
}

QModelIndex TasksModel::parent(const QModelIndex &index) const
{
    return QModelIndex();
}

int TasksModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

int TasksModel::rowCount(const QModelIndex &parent) const
{
    return 0;
}


TasksModelPrivate::TasksModelPrivate(TasksModel *model, GroupManager *gm)
    : q(model),
      groupManager(gm)
{
    populateModel();
}

void TasksModelPrivate::populateModel()
{
    q->beginRemoveRows(QModelIndex(), 0, q->rowCount());

    GroupManager *gm = groupManager.data();
    if (!gm) {
        return;
    }

    TaskGroup *root = gm->rootGroup();
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

