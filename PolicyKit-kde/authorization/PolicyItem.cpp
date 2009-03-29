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

#include "PolicyItem.h"

#include <QStringList>
#include <KIconLoader>
#include <KIcon>

#include "PoliciesModel.h"

using namespace PolkitKde;

PolicyItem::PolicyItem(bool isGroup, PolicyItem *parent)
    : parentItem(parent)
{
    if (isGroup) {
        itemData[Qt::DecorationRole] = KIcon("folder-locked");
    } else {
        itemData[Qt::DecorationRole] = KIcon("preferences-desktop-cryptography");
    }
    itemData[PoliciesModel::IsGroupRole] = isGroup;
}

PolicyItem::~PolicyItem()
{
    if (itemData.contains(PoliciesModel::PolkitEntryRole)) {
        polkit_policy_file_entry_unref(itemData[PoliciesModel::PolkitEntryRole].value<PolKitPolicyFileEntry *>());
    }
    qDeleteAll(childItems);
}

void PolicyItem::setPolkitEntry(PolKitPolicyFileEntry *entry)
{
    // yep, caching the icon DOES improve speed
    QString iconName = polkit_policy_file_entry_get_action_icon_name(entry);
    if (KIconLoader::global()->iconPath(iconName, KIconLoader::NoGroup, true).isEmpty()) {
        itemData[Qt::DecorationRole] = KIcon("preferences-desktop-cryptography");
    } else {
        itemData[Qt::DecorationRole] = KIcon(iconName);
    }

    itemData[Qt::DisplayRole] = QString::fromUtf8(polkit_policy_file_entry_get_action_description(entry));
    itemData[PoliciesModel::PathRole] = polkit_policy_file_entry_get_id(entry);
    if (itemData.contains(PoliciesModel::PolkitEntryRole)) {
        polkit_policy_file_entry_unref(itemData[PoliciesModel::PolkitEntryRole].value<PolKitPolicyFileEntry *>());
    }
    itemData[PoliciesModel::PolkitEntryRole] = QVariant::fromValue(entry);
    polkit_policy_file_entry_ref(entry);
}

void PolicyItem::appendChild(PolicyItem *item)
{
    childItems.append(item);
}

void PolicyItem::removeChild(PolicyItem *item)
{
    delete childItems.takeAt(childItems.indexOf(item));
}

PolicyItem *PolicyItem::child(int row)
{
    return childItems.value(row);
}

int PolicyItem::childCount() const
{
    return childItems.count();
}

QVariant PolicyItem::data(int role) const
{
    if (itemData.contains(role)) {
        return itemData[role];
    }
    return QVariant();
}

void PolicyItem::setData(int role, const QVariant &data)
{
    itemData[role] = data;
}

PolicyItem *PolicyItem::parent()
{
    return parentItem;
}

int PolicyItem::row() const
{
    if (parentItem)
        return parentItem->childItems.indexOf(const_cast<PolicyItem*>(this));

    return 0;
}

bool PolicyItem::isGroup() const
{
    return itemData[PoliciesModel::IsGroupRole].toBool();
}
