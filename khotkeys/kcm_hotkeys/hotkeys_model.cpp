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

#include "action_data/simple_action_data.h"
#include "action_data/menuentry_shortcut_action_data.h"
#include "action_data/action_data_group.h"

#include <typeinfo>

#include <QMimeData>

#include <KDE/KDebug>
#include <KDE/KLocale>
#include <KDE/KIcon>


static KHotKeys::ActionDataBase *findElement(
        void *ptr
        ,KHotKeys::ActionDataGroup *root)
    {
    Q_ASSERT(root);
    if (!root) return NULL;

    KHotKeys::ActionDataBase *match = NULL;

    Q_FOREACH( KHotKeys::ActionDataBase *element, root->children())
        {
        if (ptr == element)
            {
            match = element;
            break;
            }

        if (KHotKeys::ActionDataGroup *subGroup = dynamic_cast<KHotKeys::ActionDataGroup*>(element))
            {
            match = findElement(ptr, subGroup);
            if (match) break;
            }
        }

    return match;
    }



KHotkeysModel::KHotkeysModel( QObject *parent )
    : QAbstractItemModel(parent)
     ,_settings()
     ,_actions(0)
    {}


KHotkeysModel::~KHotkeysModel()
    {
    }


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

    beginInsertRows( parent, list->size(), list->size() );

    /* KHotKeys:: ActionDataGroup *action = */
    new KHotKeys::ActionDataGroup( list, i18n("New Group"), i18n("Comment"));

    endInsertRows();
    return index( list->size()-1, NameColumn, parent );
    }


// Add a group
QModelIndex KHotkeysModel::insertActionData(  KHotKeys::ActionDataBase *data, const QModelIndex & parent )
    {
    Q_ASSERT(data);

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

    beginInsertRows( parent, list->size(), list->size() );

    list->add_child(data);

    endInsertRows();
    return index( list->size()-1, NameColumn, parent );
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
            case EnabledColumn:
                // If the parent is enabled we display the state of the object.
                // If the parent is disabled this object is disabled too.
                if (action->parent() && !action->parent()->isEnabled())
                    {
                    return Qt::Unchecked;
                    }
                return action->isEnabled()
                    ? Qt::Checked
                    : Qt::Unchecked;

            default:
                return QVariant();
            }
        }

    // Display and Tooltip. Tooltip displays the complete name. That's nice if
    // there is not enough space
    else if (role==Qt::DisplayRole || role==Qt::ToolTipRole)
        {
        switch (index.column())
            {
            case NameColumn:
                return action->name();

            case EnabledColumn:
                return QVariant();

            case IsGroupColumn:
                return indexToActionDataGroup(index)!=0;

            case TypeColumn:
                {
                const std::type_info &ti = typeid(*action);
                if (ti==typeid(KHotKeys::SimpleActionData))
                    return KHotkeysModel::SimpleActionData;
                else if (ti==typeid(KHotKeys::MenuEntryShortcutActionData))
                    return KHotkeysModel::SimpleActionData;
                else if (ti==typeid(KHotKeys::ActionDataGroup))
                    return KHotkeysModel::ActionDataGroup;
                else
                    return KHotkeysModel::Other;
                }

            default:
                return QVariant();
            }
        }

    // Decoration role
    else if (role==Qt::DecorationRole)
        {
        switch (index.column())
            {
            // The 0 is correct here. We want to decorate that column
            // regardless of the content it has
            case 0:
                return dynamic_cast<KHotKeys::ActionDataGroup*>(action)
                    ? KIcon("folder")
                    : QVariant();

            default:
                return QVariant();
            }
        }

    //Providing the current action name on edit
    else if (role==Qt::EditRole)
        {
        switch (index.column())
            {
            case NameColumn:
                return action->name();

            default:
                return QVariant();
            }
        }

    else if (role==Qt::ForegroundRole)
        {
        QPalette pal;
        switch (index.column())
            {
            case NameColumn:
                if (!action->isEnabled())
                    {
                    return pal.color(QPalette::Disabled, QPalette::Foreground);
                    }

            default:
                return QVariant();
            }
        }

    // For everything else
    return QVariant();
    }


bool KHotkeysModel::dropMimeData(
        const QMimeData *data
        ,Qt::DropAction action
        ,int row
        ,int column
        ,const QModelIndex &parent)
    {
    Q_UNUSED(column);

    // We only support move actions and our own mime type
    if ( (action!=Qt::CopyAction)
            || !data->hasFormat("application/x-pointer"))
        {
        kDebug() << "Drop not supported " << data->formats();
        return false;
        }

    // Decode the stream
    QByteArray encodedData = data->data("application/x-pointer");
    QDataStream stream(&encodedData, QIODevice::ReadOnly);
    QList<quintptr> ptrs;
    while (!stream.atEnd())
        {
        quintptr ptr;
        stream >> ptr;
        ptrs << ptr;
        }

    // No pointers, nothing to do
    if (ptrs.empty()) return false;

    // Get the group we have to drop into. If the drop target is no group get
    // it's parent and drop behind it
    int position = row;
    QModelIndex dropIndex = parent;
    KHotKeys::ActionDataGroup *dropToGroup = indexToActionDataGroup(dropIndex);
    if (!dropToGroup)
        {
        dropIndex = parent.parent();
        dropToGroup = indexToActionDataGroup(dropIndex);
        position = dropToGroup->children().indexOf(indexToActionDataBase(parent));
        }

    if (position==-1)
        {
        position = dropToGroup->size();
        }

    // Do the moves
    Q_FOREACH(quintptr ptr, ptrs)
        {
        KHotKeys::ActionDataBase *element = findElement(
                reinterpret_cast<void*>(ptr),
                _actions);

        if (element) moveElement(element, dropToGroup, position);
        }

    return true;
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
        int row = parent->children().indexOf(item);
        topLeft = createIndex( row, 0, parent );
        bottomRight = createIndex( row, columnCount(topLeft), parent );
        }

    emit dataChanged( topLeft, bottomRight );
    }


void KHotkeysModel::exportInputActions(
        const QModelIndex &index,
        KConfigBase &config,
        const QString& id,
        const KHotKeys::ActionState state,
        bool mergingAllowed)
    {
    KHotKeys::ActionDataBase  *element = indexToActionDataBase(index);
    KHotKeys::ActionDataGroup *group   = indexToActionDataGroup(index);

    settings()->exportTo(
            group ? group : element->parent(),
            config,
            id,
            state,
            mergingAllowed);
    }


Qt::ItemFlags KHotkeysModel::flags( const QModelIndex &index ) const
    {
    Qt::ItemFlags flags = QAbstractItemModel::flags(index);

    Q_ASSERT(!(flags & Qt::ItemIsDropEnabled));
    Q_ASSERT(!(flags & Qt::ItemIsDragEnabled));

    if (!index.isValid())
        {
            return flags | Qt::ItemIsDropEnabled;
        }

    KHotKeys::ActionDataBase  *element = indexToActionDataBase(index);
    KHotKeys::ActionDataGroup *actionGroup = indexToActionDataGroup(index);
    if (!actionGroup) actionGroup = element->parent();

    Q_ASSERT(element);
    Q_ASSERT(actionGroup);

    // We do not allow dragging for system groups and their elements
    // We do not allow dropping into systemgroups
    if (!actionGroup->is_system_group())
        {
        flags |= Qt::ItemIsDragEnabled;
        flags |= Qt::ItemIsDropEnabled;
        }

    // Show a checkbox in column 1 whatever is shown there.
    switch (index.column())
        {
        case 1:
            return flags
                | Qt::ItemIsUserCheckable;

        default:
            return flags
                | Qt::ItemIsEditable;
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
        case NameColumn:
            return QVariant(i18nc("action name", "Name"));

        case EnabledColumn:
            return QVariant();
            return QVariant(i18nc("action enabled", "Enabled"));

        case IsGroupColumn:
            return QVariant(i18n("Type"));

        default:
            return QVariant();
        }
    }


void KHotkeysModel::importInputActions(const QModelIndex &index, KConfigBase const &config)
    {
    KHotKeys::ActionDataGroup *group = indexToActionDataGroup(index);
    QModelIndex groupIndex = index;
    if (!group)
        {
        group = indexToActionDataBase(index)->parent();
        groupIndex = index.parent();
        }

    if (settings()->importFrom(group, config, KHotKeys::ImportAsk, KHotKeys::Retain))
        {
        kDebug();
        reset();
        save();
        }
    }


QModelIndex KHotkeysModel::index( int row, int column, const QModelIndex &parent ) const
    {
    KHotKeys::ActionDataGroup *actionGroup = indexToActionDataGroup(parent);
    if (!actionGroup || row>=actionGroup->children().size() )
        {
        return QModelIndex();
        }

    KHotKeys::ActionDataBase *action =  actionGroup->children().at(row);
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
    _settings.reread_settings(true);
    _actions = _settings.actions();
    reset();
    }


QMimeData *KHotkeysModel::mimeData(const QModelIndexList &indexes) const
    {
    QMimeData * mimeData = new QMimeData();
    QByteArray encodedData;

    QDataStream stream(&encodedData, QIODevice::WriteOnly);

    Q_FOREACH (const QModelIndex &index, indexes)
        {
        if (index.isValid() and index.column() == 0)
            {
            KHotKeys::ActionDataBase *element = indexToActionDataBase(index);
            // We use the pointer as id.
            stream << reinterpret_cast<quintptr>(element);
            }
        }

    mimeData->setData("application/x-pointer", encodedData);
    return mimeData;
    }


QStringList KHotkeysModel::mimeTypes() const
    {
    QStringList types;
    types << "application/x-pointer";
    return types;
    }


bool KHotkeysModel::moveElement(
        KHotKeys::ActionDataBase   *element
        ,KHotKeys::ActionDataGroup *newGroup
        ,int position)
    {
    Q_ASSERT(element && newGroup);
    if (!element || !newGroup) return false;

    // TODO: Make this logic more advanced
    // We do not allow moving into our systemgroup
    if (newGroup->is_system_group()) return false;

    // Make sure we don't move a group to one of it's children or
    // itself.
    KHotKeys::ActionDataGroup *tmp = newGroup;
    do  {
        if (tmp == element)
            {
            kDebug() << "Forbidden move" << tmp->name();
            return false;
            }
        }
        while((tmp = tmp->parent()));

    KHotKeys::ActionDataGroup *oldParent = element->parent();

    // TODO: Make this logic more advanced
    // We do not allow moving from our systemgroup
    if (oldParent->is_system_group()) return false;

    // Adjust position if oldParent and newGroup are identical
    if (oldParent == newGroup)
        {
        if (oldParent->children().indexOf(element) < position)
            {
            --position;
            }
        }

    emit layoutAboutToBeChanged();

    // Remove it from it's current place
    oldParent->remove_child(element);
    newGroup->add_child(element, position);

    emit layoutChanged();

    return true;
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

    int row = grandparent->children().indexOf(parent);
    return createIndex( row, 0, parent );
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

    action->aboutToBeErased();
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

    return group->children().count();
    }


void KHotkeysModel::save()
    {
    _settings.write();
    }


// Set data
bool KHotkeysModel::setData( const QModelIndex &index, const QVariant &value, int role )
    {

    if ( !index.isValid() )
        {
        return false;
        }

    KHotKeys::ActionDataBase *action = indexToActionDataBase(index);
    Q_ASSERT( action );

    // Handle CheckStateRole
    if ( role == Qt::CheckStateRole )
        {
        switch(index.column())
            {
            case EnabledColumn:
                {
                // If the parent is enabled we display the state of the object.
                // If the parent is disabled this object is disabled too.
                if (action->parent() && !action->parent()->isEnabled())
                    {
                    // TODO: Either show a message box or enhance the gui to
                    // show this item cannot be enabled
                    return false;
                    }

                value.toInt() == Qt::Checked
                    ? action->enable()
                    : action->disable();

                // If this is a group we have to inform the view that all our
                // childs have changed. They are all disabled now
                KHotKeys::ActionDataGroup *actionGroup = indexToActionDataGroup(index);
                if (actionGroup && actionGroup->size())
                    {
                    Q_EMIT dataChanged(
                            createIndex(0, 0, actionGroup),
                            createIndex(actionGroup->size(), columnCount(index), actionGroup));
                    }
                }
                break;

            default:
                return false;
            }
        }
    else if ( role == Qt::EditRole )
        {
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
        }
    else
        return false;

    emit dataChanged( index, index );
    return true;
    }


KHotKeys::Settings *KHotkeysModel::settings()
    {
    return &_settings;
    }


#include "moc_hotkeys_model.cpp"
