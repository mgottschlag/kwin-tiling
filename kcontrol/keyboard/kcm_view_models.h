/*
 *  Copyright (C) 2010 Andriy Rysin (rysin@kde.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */


#ifndef KCM_VIEW_MODELS_H_
#define KCM_VIEW_MODELS_H_

#include <QtCore/QAbstractItemModel>
#include <QtCore/QAbstractTableModel>

class QTreeView;
class KeyboardConfig;
class Rules;
class Flags;

class LayoutsTableModel : public QAbstractTableModel
{
     Q_OBJECT

 public:
     LayoutsTableModel(Rules* rules, Flags *flags, KeyboardConfig* keyboardConfig, QObject *parent = 0);

     int columnCount(const QModelIndex&) const;
     Qt::ItemFlags flags(const QModelIndex &index) const;
     QVariant headerData(int section, Qt::Orientation orientation,
                         int role = Qt::DisplayRole) const;

     int rowCount(const QModelIndex &parent = QModelIndex()) const;
     QVariant data(const QModelIndex &index, int role) const;
     bool setData(const QModelIndex &index, const QVariant &value, int role);

     void refresh();

 private:
     KeyboardConfig* keyboardConfig;
     Rules *rules;
     Flags *countryFlags;
};

class XkbOptionsTreeModel: public QAbstractItemModel
{
public:
    XkbOptionsTreeModel(Rules* rules_, KeyboardConfig* keyboardConfig_, QObject *parent)
		: QAbstractItemModel(parent),
		  keyboardConfig(keyboardConfig_),
		  rules(rules_) { }

    int columnCount(const QModelIndex& /*parent*/) const { return 1; }
    int rowCount(const QModelIndex& parent) const;
    QModelIndex parent(const QModelIndex& index) const {
        if (!index.isValid() )
            return QModelIndex();
        if( index.internalId() < 100 )
            return QModelIndex();
        return createIndex(((index.internalId() - index.row())/100) - 1, index.column());
    }
    QModelIndex index(int row, int column, const QModelIndex& parent) const {
        if(!parent.isValid()) return createIndex(row, column);
        return createIndex(row, column, (100 * (parent.row()+1)) + row);
    }
    Qt::ItemFlags flags ( const QModelIndex & index ) const {
        if( ! index.isValid() )
            return 0;

        if( !index.parent().isValid() )
            return Qt::ItemIsEnabled;

        return Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
    }

    bool setData ( const QModelIndex & index, const QVariant & value, int role = Qt::EditRole );
    QVariant data(const QModelIndex& index, int role) const;
    void reset() { QAbstractItemModel::reset(); }
    void gotoGroup(const QString& group, QTreeView* view);

private:
    KeyboardConfig* keyboardConfig;
    Rules *rules;
};

#endif /* KCM_VIEW_MODELS_H_ */
