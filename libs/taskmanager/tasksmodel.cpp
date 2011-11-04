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

#include <QMetaEnum>

#include <KDebug>

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
    void itemAboutToBeAdded(AbstractGroupableItem *item, int index);
    void itemAdded(AbstractGroupableItem *item);
    void itemAboutToBeRemoved(AbstractGroupableItem *item);
    void itemRemoved(AbstractGroupableItem *item);
    void itemAboutToMove(AbstractGroupableItem *item, int currentIndex, int newIndex);
    void itemMoved(AbstractGroupableItem *item);
    void itemChanged(::TaskManager::TaskChanges changes);

    int indexOf(AbstractGroupableItem *item);

    TasksModel *q;
    QWeakPointer<GroupManager> groupManager;
    TaskGroup *rootGroup;
};

TasksModel::TasksModel(GroupManager *groupManager, QObject *parent)
    : QAbstractItemModel(parent),
      d(new TasksModelPrivate(this, groupManager))
{
    // set the role names based on the values of the DisplayRoles enum for the sake of QML
    QHash<int, QByteArray> roles;
    roles.insert(Qt::DisplayRole, "DisplayRole");
    roles.insert(Qt::DecorationRole, "DecorationRole");
    QMetaEnum e = metaObject()->enumerator(metaObject()->indexOfEnumerator("DisplayRoles"));
    for (int i = 0; i < e.keyCount(); ++i) {
        roles.insert(e.value(i), e.key(i));
    }
    setRoleNames(roles);

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
    } else if (role == TasksModel::IsStartup) {
        return item->isStartupItem();
    } else if (role == TasksModel::OnAllDesktops) {
        return item->isOnAllDesktops();
    } else if (role == TasksModel::Desktop) {
        return item->desktop();
    } else if (role == TasksModel::Shaded) {
        return item->isShaded();
    } else if (role == TasksModel::Maximized) {
        return item->isMaximized();
    } else if (role == TasksModel::Minimized) {
        return item->isMinimized();
    } else if (role == TasksModel::FullScreen) {
        return item->isFullScreen();
    } else if (role == TasksModel::BelowOthers) {
        return item->isKeptBelowOthers();
    } else if (role == TasksModel::AlwaysOnTop) {
        return item->isAlwaysOnTop();
    } else if (role == TasksModel::Active) {
        return item->isActive();
    } else if (role == TasksModel::DemandsAttention) {
        return item->demandsAttention();
    } else if (role == TasksModel::LauncherUrl) {
        return item->launcherUrl();
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

int TasksModel::columnCount(const QModelIndex &) const
{
    return 1;
}

int TasksModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return d->rootGroup->members().count();
    }

    AbstractGroupableItem *item = static_cast<AbstractGroupableItem *>(parent.internalPointer());
    if (item->itemType() == GroupItemType) {
        TaskGroup *group = static_cast<TaskGroup *>(item);
        return group->members().count();
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

    QObject::connect(group, SIGNAL(itemAboutToBeAdded(AbstractGroupableItem*, int)),
                     q, SLOT(itemAboutToBeAdded(AbstractGroupableItem*, int)),
                     Qt::UniqueConnection);
    QObject::connect(group, SIGNAL(itemAdded(AbstractGroupableItem*)),
                     q, SLOT(itemAdded(AbstractGroupableItem*)),
                     Qt::UniqueConnection);
    QObject::connect(group, SIGNAL(itemAboutToBeRemoved(AbstractGroupableItem*)),
                     q, SLOT(itemAboutToBeRemoved(AbstractGroupableItem*)),
                     Qt::UniqueConnection);
    QObject::connect(group, SIGNAL(itemRemoved(AbstractGroupableItem*)),
                     q, SLOT(itemRemoved(AbstractGroupableItem*)),
                     Qt::UniqueConnection);
    QObject::connect(group, SIGNAL(itemAboutToMove(AbstractGroupableItem*, int, int)),
                     q, SLOT(itemAboutToMove(AbstractGroupableItem*, int, int)),
                     Qt::UniqueConnection);
    QObject::connect(group, SIGNAL(itemPositionChanged(AbstractGroupableItem*)),
                     q, SLOT(itemMoved(AbstractGroupableItem*)),
                     Qt::UniqueConnection);

    typedef QPair<QModelIndex, TaskGroup *> idxGroupPair;
    QList<idxGroupPair> childGroups;

    int i = 0;
    foreach (AbstractGroupableItem * item,  group->members()) {
        if (item->itemType() == GroupItemType) {
            QModelIndex idx(q->index(i, 0, parent));
            childGroups << idxGroupPair(idx, static_cast<TaskGroup *>(item));
        }

        QObject::connect(item, SIGNAL(changed(::TaskManager::TaskChanges)),
                         q, SLOT(itemChanged(::TaskManager::TaskChanges)),
                         Qt::UniqueConnection);
        ++i;
    }

    foreach (const idxGroupPair & pair, childGroups) {
        populate(pair.first, pair.second);
    }
}

int TasksModelPrivate::indexOf(AbstractGroupableItem *item)
{
    Q_ASSERT(item != rootGroup);
    int row = 0;
    //kDebug() << item << item->parentGroup();

    foreach (const AbstractGroupableItem * child, item->parentGroup()->members()) {
        if (child == item) {
            break;
        }

        ++row;
    }

    return row;
}

void TasksModelPrivate::itemAboutToBeAdded(AbstractGroupableItem *item, int index)
{
    TaskGroup *parent = item->parentGroup();
    QModelIndex parentIdx;
    if (parent->parentGroup()) {
        parentIdx = q->createIndex(indexOf(parent), 0, parent);
    }

    q->beginInsertRows(parentIdx, index, index);
}

void TasksModelPrivate::itemAdded(AbstractGroupableItem *item)
{
    Q_UNUSED(item)
    q->endInsertRows();
}

void TasksModelPrivate::itemAboutToBeRemoved(AbstractGroupableItem *item)
{
    TaskGroup *parent = static_cast<TaskGroup *>(q->sender());
    QModelIndex parentIdx;
    if (parent && parent->parentGroup()) {
        parentIdx = q->createIndex(indexOf(parent), 0, parent);
    }

    const int index = indexOf(item);
    q->beginRemoveRows(parentIdx, index, index);
}

void TasksModelPrivate::itemRemoved(AbstractGroupableItem *item)
{
    Q_UNUSED(item)
    q->endRemoveRows();
}

void TasksModelPrivate::itemAboutToMove(AbstractGroupableItem *item, int currentIndex, int newIndex)
{
    QModelIndex parentIdx;
    TaskGroup *parent = item->parentGroup();
    if (parent && parent->parentGroup()) {
        parentIdx = q->createIndex(indexOf(parent), 0, parent);
    }

    q->beginMoveRows(parentIdx, currentIndex, currentIndex, parentIdx, newIndex + 1);
}

void TasksModelPrivate::itemMoved(AbstractGroupableItem *item)
{
    Q_UNUSED(item)
    q->endMoveRows();
}

void TasksModelPrivate::itemChanged(::TaskManager::TaskChanges changes)
{
    Q_UNUSED(changes)
    AbstractGroupableItem *item = static_cast<AbstractGroupableItem *>(q->sender());
    const int index = indexOf(item);
    QModelIndex idx = q->createIndex(index, 0, item);
    emit q->dataChanged(idx, idx);
}

}

#include "tasksmodel.moc"

