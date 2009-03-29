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

#ifndef POLICY_ITEM_H
#define POLICY_ITEM_H

#include <QList>
#include <QHash>
#include <QVariant>

#include <polkit/polkit.h>

namespace PolkitKde
{

class PolicyItem
{
public:
    explicit PolicyItem(bool isGroup, PolicyItem *parent = 0);
    ~PolicyItem();

    void appendChild(PolicyItem *child);
    void removeChild(PolicyItem *item);

    PolicyItem *child(int row);
    int childCount() const;
    QVariant data(int role) const;
    void setData(int role, const QVariant &data);
    int row() const;
    PolicyItem *parent();

    bool isGroup() const;
    void setPolkitEntry(PolKitPolicyFileEntry *entry);

private:
    QList<PolicyItem*> childItems;
    QHash<int, QVariant> itemData;
    PolicyItem *parentItem;
};


}

#endif
