/*
   Copyright (C) 2008 Michael Jansen <kde@michael-jansen.biz>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
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

#include "hotkeys_model.h"

#include "simple_action_data.h"
#include "action_data_group.h"

#include <typeinfo>

#include <KDE/KDebug>
#include <KDE/KLocale>


KHotkeysModel::KHotkeysModel( QObject *parent )
    : QAbstractItemModel(parent)
     ,_settings()
     ,_actions(0)
    {}


KHotkeysModel::~KHotkeysModel()
    {
    }


// Add a group
QModelIndex KHotkeysModel::addGroup( const QModelIndex & parent )
    {
    KHotKeys::ActionDataGroup *list;
    if (parent.isValid())
        {
        list = indexToActionDataGroup(parent);
        }
    else
        {
        list = _actions;
        }
    Q_ASSERT(list);

    beginInsertRows( parent, list->child_count(), list->child_count() );

    /* KHotKeys:: ActionDataGroup *action = */
    new KHotKeys::ActionDataGroup( list, i18n("New Group"), i18n("Comment"));

    endInsertRows();
    return index( list->child_count()-1, NameColumn, parent );
    }


// Add a group
QModelIndex KHotkeysModel::insertActionData(  KHotKeys::ActionDataBase *data, const QModelIndex & parent )
    {
    KHotKeys::ActionDataGroup *list;
    if (parent.isValid())
        {
        list = indexToActionDataGroup(parent);
        }
    else
        {
        list = _actions;
        }
    Q_ASSERT(list);

    beginInsertRows( parent, list->child_count(), list->child_count() );

    list->add_child( data );

    endInsertRows();
    return index( list->child_count()-1, NameColumn, parent );
    }


int KHotkeysModel::columnCount( const QModelIndex & ) const
    {
    return 2;
    }


QVariant KHotkeysModel::data( const QModelIndex &index, int role ) const
    {
    // Check that the index is valid
    if (!index.isValid())
        {
        return QVariant();
        }

    // Get the item behind the index
    KHotKeys::ActionDataBase *action = indexToActionDataBase(index);
    Q_ASSERT(action);

    // Handle CheckStateRole
    if (role==Qt::CheckStateRole)
        {
        switch(index.column())
            {
            case 1:
                // If the parent is enabled we display the state of the object.
                // If the parent is disabled this object is disabled too.
                if (action->parent() && !action->parent()->enabled())
                    {
                    return Qt::Unchecked;
                    }
                return action->enabled()
                    ? Qt::Checked
                    : Qt::Unchecked;

            default:
                return QVariant();
            }
        }

    // Display and Tooltip. Tooltip displays the complete name. That's nice if
    // there is not enough space 
    if (role==Qt::DisplayRole || role==Qt::ToolTipRole)
        {
        switch (index.column())
            {
            case 0:
                return action->name();

            case 1:
                return QVariant();

            case 2:
                return indexToActionDataGroup(index)!=0;

            case 3:
                {
                const std::type_info &ti = typeid(*action);
                if (ti==typeid(KHotKeys::SimpleActionData))
                    return KHotkeysModel::SimpleActionData;
                else if (ti==typeid(KHotKeys::Action_data_group))
                    return KHotkeysModel::ActionDataGroup;
                else
                    return KHotkeysModel::Other;
                }

            default:
                return QVariant();
            }
        }

        // For everything else
        return QVariant();
    }


void KHotkeysModel::emitChanged(KHotKeys::ActionDataBase *item)
    {
    Q_ASSERT( item );

    KHotKeys::ActionDataGroup *parent = item->parent();
    QModelIndex topLeft;
    QModelIndex bottomRight;
    if (!parent)
        {
        topLeft = createIndex( 0, 0, _actions );
        bottomRight = createIndex( 0, 0, _actions );
        }
    else
        {
        int row = parent->list.indexOf(item);
        topLeft = createIndex( row, 0, parent );
        bottomRight = createIndex( row, columnCount(topLeft), parent );
        }

    emit dataChanged( topLeft, bottomRight );
    }


Qt::ItemFlags KHotkeysModel::flags( const QModelIndex &index ) const
    {
    if (!index.isValid())
        {
        return QAbstractItemModel::flags(index);
        }

    // Get the item behind the index
    KHotKeys::ActionDataBase *action = indexToActionDataBase(index);
    Q_ASSERT(action);

    switch (index.column())
        {
        case 1:
            return QAbstractItemModel::flags(index) | Qt::ItemIsUserCheckable;

        default:
            return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
        }
    }


// Get header data for section
QVariant KHotkeysModel::headerData( int section, Qt::Orientation, int role ) const
    {
    if (role!=Qt::DisplayRole)
        {
        return QVariant();
        }

    switch (section)
        {
        case 0:
            return QVariant(i18n("Name"));

        case 1:
            return QVariant(i18n("Enabled"));

        case 2:
            return QVariant(i18n("Type"));

        default:
            return QVariant();
        }
    }


QModelIndex KHotkeysModel::index( int row, int column, const QModelIndex &parent ) const
    {
    KHotKeys::ActionDataGroup *actionGroup = indexToActionDataGroup(parent);
    if (!actionGroup || row>=actionGroup->list.size() )
        {
        return QModelIndex();
        }

    KHotKeys::ActionDataBase *action =  actionGroup->list.at(row);
    Q_ASSERT( action );
    return createIndex( row, column, action );
    }


// Convert index to ActionDataBase
KHotKeys::ActionDataBase *KHotkeysModel::indexToActionDataBase( const QModelIndex &index ) const
    {
    if (!index.isValid())
        {
        return _actions;
        }
    return static_cast<KHotKeys::ActionDataBase*>( index.internalPointer() );
    }


// Convert index to ActionDataGroup
KHotKeys::ActionDataGroup *KHotkeysModel::indexToActionDataGroup( const QModelIndex &index ) const
    {
    if (!index.isValid())
        {
        return _actions;
        }
    return dynamic_cast<KHotKeys::ActionDataGroup*>( indexToActionDataBase(index) );
    }


void KHotkeysModel::load()
    {
    _settings.read_settings(true);
    _actions = _settings.actions();
    reset();
    }


// Get parent object for index
QModelIndex KHotkeysModel::parent( const QModelIndex &index ) const
    {
    KHotKeys::ActionDataBase *action = indexToActionDataBase(index);
    if (!action)
        {
        return QModelIndex();
        }

    KHotKeys::ActionDataGroup *parent = action->parent();
    if (!parent)
        {
        return QModelIndex();
        }

    KHotKeys::ActionDataGroup *grandparent = parent->parent();
    if (!grandparent)
        {
        return QModelIndex();
        }

    int row = grandparent->list.indexOf(parent);
    return createIndex( row, index.column(), parent );
    }


// Remove rows ( items )
bool KHotkeysModel::removeRows( int row, int count, const QModelIndex &parent )
    {
    Q_ASSERT( count == 1 );

    beginRemoveRows( parent, row, row+count-1 );

    KHotKeys::ActionDataGroup *list;
    if (parent.isValid())
        {
        list = indexToActionDataGroup(parent);
        }
    else
        {
        list = _actions;
        }
    Q_ASSERT(list);

    KHotKeys::ActionDataBase *action = indexToActionDataBase(index(row,0,parent));

    list->remove_child(action);
    delete action;

    endRemoveRows();
    return true;
    }


// Number of rows for index
int KHotkeysModel::rowCount( const QModelIndex &index ) const
    {
    KHotKeys::ActionDataGroup *group = indexToActionDataGroup(index);
    if (!group)
        {
        return 0;
        }

    return group->list.count();
    }


void KHotkeysModel::save()
    {
    _settings.write_settings();
    }


// Set data
bool KHotkeysModel::setData( const QModelIndex &index, const QVariant &value, int role )
    {

    if ( !index.isValid() || role != Qt::EditRole )
        {
        return false;
        }

    KHotKeys::ActionDataBase *action = indexToActionDataBase(index);
    Q_ASSERT( action );

    switch ( index.column() )
        {
        case NameColumn:
            {
            action->set_name( value.toString() );
            }
            break;

        default:
            return false;
        }

    emit dataChanged( index, index );
    return true;
    }

#include "moc_hotkeys_model.cpp"
