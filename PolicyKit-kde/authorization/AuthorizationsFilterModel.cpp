/*  This file is part of the KDE project
    Copyright (C) 2008 Dario Freddi <drf54321@gmail.com>
    Copyright (C) 2009 Daniel Nicoletti <dantti85-pk@yahoo.com.br>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#include "AuthorizationsFilterModel.h"

#include "PoliciesModel.h"

#include <KDebug>

namespace PolkitKde
{

AuthorizationsFilterModel::AuthorizationsFilterModel(QObject *parent)
        : QSortFilterProxyModel(parent)
{
}

AuthorizationsFilterModel::~AuthorizationsFilterModel()
{
}

bool AuthorizationsFilterModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    QModelIndex index = sourceModel()->index(source_row, 0, source_parent);

    // if the filter is empty avoid checking since if the model isn't complete populated
    // some items might be hidden
    if (filterRegExp().isEmpty()) {
        return true;
    }

    if (index.data(PolkitKde::PoliciesModel::IsGroupRole).toBool()) {
        return groupHasMatchingItem(index);
    }

    return (index.data(PolkitKde::PoliciesModel::PathRole).toString().contains(filterRegExp()) ||
            index.data(Qt::DisplayRole).toString().contains(filterRegExp()));
}

bool AuthorizationsFilterModel::groupHasMatchingItem(const QModelIndex &parent) const
{
//     kDebug() << "group" << parent.data(Qt::DisplayRole);
    for (int i = 0; i < sourceModel()->rowCount(parent); i++) {
        QModelIndex index = sourceModel()->index(i, 0, parent);
        // we check to see if the item is a group
        // if so we call groupHasMatchingItem for that group
        if (index.data(PolkitKde::PoliciesModel::IsGroupRole).toBool()) {
            // we call to see if the subgroup has an matching item
            if (groupHasMatchingItem(sourceModel()->index(i, 0, parent))) {
                return true;
            }
        } else {
//             kDebug() << "item" << index.data(Qt::DisplayRole);
            // here we have an action let's see it this match
            if (index.data(PolkitKde::PoliciesModel::PathRole).toString().contains(filterRegExp()) ||
                index.data(Qt::DisplayRole).toString().contains(filterRegExp()))
            {
                return true;
            }
        }
    }
    // if we don't find any matching action return false
    return false;
}


}

#include "AuthorizationsFilterModel.moc"
