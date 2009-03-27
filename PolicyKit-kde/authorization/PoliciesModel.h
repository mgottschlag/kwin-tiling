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

#ifndef POLICIES_MODEL_H
#define POLICIES_MODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

#include <polkit/polkit.h>

namespace PolkitKde
{

class PolicyItem;

class PoliciesModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    enum DataRoles {
        PathRole = 41,
        IsGroupRole = 42,
        PolkitEntryRole = 43
    };

    PoliciesModel(QObject *parent = 0);
    ~PoliciesModel();

    QVariant data(const QModelIndex &index, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    void setCurrentEntries(const QList<PolKitPolicyFileEntry *> &entries);
    QModelIndex indexFromId(const QString &id) const;

private:
    void insertOrUpdate(const QStringList &actionPath, PolKitPolicyFileEntry *entry,
                        PolicyItem *parent, int level = 0);
    bool removeEntries(const QStringList &entries, PolicyItem *parent);
    QModelIndex indexFromId(const QString &id, PolicyItem *parent) const;

    PolicyItem *rootItem;
};


}

Q_DECLARE_METATYPE(PolKitPolicyFileEntry*)

#endif
