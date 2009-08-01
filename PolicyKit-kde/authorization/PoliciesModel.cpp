/***************************************************************************
 *   Copyright (C) 2009 by Daniel Nicoletti                                *
 *   dantti85-pk@yahoo.com.br                                              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "PoliciesModel.h"

#include <QStringList>
#include <KDebug>

#include "PolicyItem.h"

using namespace PolkitKde;

PoliciesModel::PoliciesModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    rootItem = new PolicyItem(true);
}

PoliciesModel::~PoliciesModel()
{
    delete rootItem;
}

int PoliciesModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 1;
}

QVariant PoliciesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    PolicyItem *item = static_cast<PolicyItem*>(index.internalPointer());

    return item->data(role);
}

Qt::ItemFlags PoliciesModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QModelIndex PoliciesModel::index(int row, int column, const QModelIndex &parent)
            const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    PolicyItem *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<PolicyItem*>(parent.internalPointer());

    PolicyItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex PoliciesModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    PolicyItem *childItem = static_cast<PolicyItem*>(index.internalPointer());
    PolicyItem *parentItem = childItem->parent();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int PoliciesModel::rowCount(const QModelIndex &parent) const
{
    PolicyItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<PolicyItem*>(parent.internalPointer());

    return parentItem->childCount();
}

void PoliciesModel::setCurrentEntries(const QList<PolKitPolicyFileEntry *> &entries)
{
    // before inserting let's remove entries (policies)
    // that don't exist anymore
    QStringList stringEntries;
    foreach(PolKitPolicyFileEntry *entry, entries) {
        stringEntries.append(polkit_policy_file_entry_get_id(entry));
    }
    removeEntries(stringEntries, rootItem);

    // now we insert or update all the items
    foreach(PolKitPolicyFileEntry *entry, entries) {
        QStringList actionPath;
        actionPath = QString(polkit_policy_file_entry_get_id(entry)).split('.');
        if (actionPath.size() > 2) {
            // if we have an action id bigger than
            // "org.kde"
            QString rootString = actionPath.takeFirst();
            rootString        += '.' + actionPath.takeFirst();
            actionPath.prepend(rootString);
            insertOrUpdate(actionPath, entry, rootItem);
        } else if (actionPath.size() > 1) {
            // although pretty weird and probably
            // bad behavior, here we check if the action
            // id is bigger than "org"
            insertOrUpdate(actionPath, entry, rootItem);
        } else {
            kWarning() << "Bad Policy file entry" << polkit_policy_file_entry_get_id(entry) << entry;
        }
    }
}

QModelIndex PoliciesModel::indexFromId(const QString &id) const
{
    return indexFromId(id, rootItem);
}

QModelIndex PoliciesModel::indexFromId(const QString &id, PolicyItem *parent) const
{
    for (int i = 0; i < parent->childCount(); i++) {
        PolicyItem *item = parent->child(i);
//         kDebug() << "ITEM" << item->data(PathRole).toString() << parent->childCount();
        if (item->isGroup()) {
            const QModelIndex index = indexFromId(id, item);
            if (index != QModelIndex()) {
                return index;
            }
        } else if (id == item->data(PathRole).toString()) {
            if (parent == rootItem) {
                // i doubt this would ever happen :D
                return QModelIndex();
            } else{
                return createIndex(item->row(), 0, item);
            }
        }
    }
    return QModelIndex();
}

bool PoliciesModel::removeEntries(const QStringList &entries, PolicyItem *parent)
{
    for (int i = 0; i < parent->childCount(); i++) {
        PolicyItem *item = parent->child(i);
//         kDebug() << "ITEM" << item->data(PathRole).toString() << parent->childCount();
        if (item->isGroup()) {
            if (!removeEntries(entries, item)) {
                // removeEntries returns true if the action does
                // not have any children
                // if there are items let's continue to the next item
                continue;
            }
        } else {
            bool found = false;
            // lets see if this action still exists in the new list
            foreach (const QString &entry, entries) {
                if (entry == item->data(PathRole).toString()) {
                    found = true;
                    break;
                }
            }
            if (found) {
                // if we found it let's go to the next item
                continue;
            }
        }
        // if we got here we can remove
        QModelIndex index;
        if (parent != rootItem) {
            index = createIndex(parent->row(), 0, parent);
        }
//         kDebug() << "REMOVING" << item->data(PathRole).toString() << index;
        beginRemoveRows(index, item->row(), item->row());
        parent->removeChild(item);
        endRemoveRows();
        // we decrement the iterator as
        // the item was removed ;)
        i--;
    }
    return parent->childCount() == 0;
}

void PoliciesModel::insertOrUpdate(const QStringList &actionPath, PolKitPolicyFileEntry *entry, PolicyItem *parent, int level)
{
//     kDebug() << actionPath << actionPath.size() << entry << level << parent;
    if (actionPath.size() - 1 == level) {
        // if the actionPath size is equal as
        // the leve we are about to insert the
        // action itself
        QString path = actionPath.join(".");
        PolicyItem *action = 0;
        for (int i = 0; i < parent->childCount(); i++) {
            if (!parent->child(i)->isGroup() && path == parent->child(i)->data(PathRole)) {
                action = parent->child(i);
                break;
            }
        }
        QModelIndex index;
        if (parent != rootItem) {
            index = createIndex(parent->row(), 0, parent);
        }
        if (action) {
            // ok action found let's update it.
//             kDebug() << "Updating action:" << path << index;
            action->setPolkitEntry(entry);
            emit dataChanged(index, index);
        } else {
            // action not found lets add one
//             kDebug() << "Adding action:" << path << index;
            beginInsertRows(index, parent->childCount(), parent->childCount());
            parent->appendChild(action = new PolicyItem(false, parent));
            action->setPolkitEntry(entry);
            endInsertRows();
            // Update the parent
            emit dataChanged(index, index);
        }
    } else {
        // here we are adding or finding a group
        QString path = actionPath.at(level);
        PolicyItem *group = 0;
        for (int i = 0; i < parent->childCount(); i++) {
            if (parent->child(i)->isGroup() && path == parent->child(i)->data(PathRole)) {
                group = parent->child(i);
                break;
            }
        }
        if (group) {
//             kDebug() << "Group Found" << path;
            insertOrUpdate(actionPath, entry, group, level + 1);
        } else {
            // ok we didn't find the group
            // let's add the group
            QModelIndex index;
            if (parent != rootItem) {
                index = createIndex(parent->row(), 0, parent);
            }
//             kDebug() << "Group NOT Found, adding one: " << path << index;
            beginInsertRows(index, parent->childCount(), parent->childCount());
            parent->appendChild(group = new PolicyItem(true, parent));
            group->setData(PathRole, path);
            // if we are at the lowest level of a group
            // we try to get the vendorName
            if (actionPath.size() - 2 == level) {
                const QString vendorName = QString::fromLocal8Bit(polkit_policy_file_entry_get_action_vendor(entry));
                if (vendorName.isEmpty()) {
                    group->setData(Qt::DisplayRole, path);
                } else {
                    group->setData(Qt::DisplayRole, vendorName);
                }
            } else {
                group->setData(Qt::DisplayRole, path);
            }
            endInsertRows();
            insertOrUpdate(actionPath, entry, group, level + 1);
        }
    }
}
