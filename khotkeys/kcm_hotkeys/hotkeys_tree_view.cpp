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

#include "hotkeys_tree_view.h"

#include "hotkeys_model.h"

#include "actions/actions.h"
#include "action_data/action_data_group.h"
#include "action_data/simple_action_data.h"

#include "KDE/KLocale"
#include "KDE/KDebug"

#include <QtCore/QSignalMapper>
#include <QtGui/QContextMenuEvent>


HotkeysTreeViewContextMenu::HotkeysTreeViewContextMenu( const QModelIndex &index, HotkeysTreeView *parent )
        : QMenu(parent)
         ,_index(index)
         ,_view(parent)
    {
    setTitle( i18n("Test") );

    if (index.isValid())
        {

        KHotKeys::ActionDataBase *element = parent->model()->indexToActionDataBase(index);
        KHotKeys::ActionDataGroup *group =  parent->model()->indexToActionDataGroup(index);
        bool isGroup = group;   // Is the current element a group
        if (!isGroup)
            {
            group = element->parent();
            }

        // Create the create actions
        createTriggerMenus(group->allowedTriggerTypes(), group->allowedActionTypes());

        // It is not allowed to create a subgroup for a system group.
        if (!group->is_system_group())
            {
            addAction( i18n("New Group") , this, SLOT(newGroupAction()) );
            }

        // It is not allowed to delete a system group
        if (!(isGroup && group->is_system_group()))
            {
            // Global actions
            addSeparator();

            // Item related actions
            addAction( i18n("Delete"), this, SLOT(deleteAction()) );
            }
        }
    else
        {
        createTriggerMenus(KHotKeys::Trigger::AllTypes, KHotKeys::Action::AllTypes);
        addAction( i18n("New Group") , this, SLOT(newGroupAction()) );
        }
    }


HotkeysTreeViewContextMenu::~HotkeysTreeViewContextMenu()
    {}

void HotkeysTreeViewContextMenu::createTriggerMenus(
        KHotKeys::Trigger::TriggerTypes triggerTypes,
        KHotKeys::Action::ActionTypes actionTypes)
    {
    if (triggerTypes & KHotKeys::Trigger::ShortcutTriggerType)
        {
        QSignalMapper *mapper = new QSignalMapper(this);

        QMenu *menu = new QMenu( i18n("New Global Shortcut") );
        populateTriggerMenu(menu, mapper, actionTypes);
        addMenu(menu);

        connect(
            mapper, SIGNAL(mapped(int)),
            this, SLOT(newGlobalShortcutActionAction(int)) );
        }

    if (triggerTypes & KHotKeys::Trigger::WindowTriggerType)
        {
        QSignalMapper *mapper = new QSignalMapper(this);

        QMenu *menu = new QMenu( i18n("New Window Action") );
        populateTriggerMenu(menu, mapper, actionTypes);
        addMenu(menu);

        connect(
            mapper, SIGNAL(mapped(int)),
            this, SLOT(newWindowTriggerActionAction(int)) );
        }
    }


void HotkeysTreeViewContextMenu::populateTriggerMenu(
        QMenu *menu,
        QSignalMapper *mapper,
        KHotKeys::Action::ActionTypes types)
    {
    if (types & KHotKeys::Action::CommandUrlActionType)
        {
        mapper->setMapping(
            menu->addAction( i18n("Command/URL"), mapper, SLOT(map()) ),
            KHotKeys::Action::CommandUrlActionType );
        }

    if (types & KHotKeys::Action::DBusActionType)
        {
        mapper->setMapping(
            menu->addAction( i18n("D-Bus Command"), mapper, SLOT(map()) ),
            KHotKeys::Action::DBusActionType );
        }

    if (types & KHotKeys::Action::MenuEntryActionType)
        {
        mapper->setMapping(
            menu->addAction( i18n("K-Menu Entry"), mapper, SLOT(map()) ),
            KHotKeys::Action::MenuEntryActionType );
        }
    }

void HotkeysTreeViewContextMenu::newGlobalShortcutActionAction( int actionType )
    {
    QModelIndex parent;      // == root element
    if (!_index.isValid()
        || _view->model()->data( _index.sibling( _index.row(), KHotkeysModel::IsGroupColumn)).toBool())
        {
        // if the index is invalid (root index) or represents an group use it.
        parent = _index;
        }
    else
        {
        // It is an action. Take the parent.
        parent = _index.parent();
        }

    KHotKeys::SimpleActionData *data =
        new KHotKeys::SimpleActionData( 0, i18n("New Group"), i18n("Comment"));
    data->set_trigger( new KHotKeys::ShortcutTrigger( data, KShortcut() ) );

    switch (actionType)
        {
        case KHotKeys::Action::MenuEntryActionType:
            data->set_action( new KHotKeys::MenuEntryAction( data ));
            break;

        case KHotKeys::Action::CommandUrlActionType:
            data->set_action( new KHotKeys::CommandUrlAction( data ));
            break;

        case KHotKeys::Action::DBusActionType:
            data->set_action( new KHotKeys::DBusAction( data ));
            break;

        default:
            Q_ASSERT(false);
            return;
        }

    QModelIndex newAct = _view->model()->insertActionData(data, parent);
    _view->setCurrentIndex(newAct);
    _view->edit(newAct);
    _view->resizeColumnToContents(0);
    }


void HotkeysTreeViewContextMenu::newWindowTriggerActionAction( int actionType )
    {
    QModelIndex parent;      // == root element
    if (!_index.isValid()
        || _view->model()->data( _index.sibling( _index.row(), KHotkeysModel::IsGroupColumn)).toBool())
        {
        // if the index is invalid (root index) or represents an group use it.
        parent = _index;
        }
    else
        {
        // It is an action. Take the parent.
        parent = _index.parent();
        }

    KHotKeys::SimpleActionData *data =
        new KHotKeys::SimpleActionData( 0, i18n("New Group"), i18n("Comment"));
    data->set_trigger( new KHotKeys::WindowTrigger(data) );

    switch (actionType)
        {
        case KHotKeys::Action::MenuEntryActionType:
            data->set_action( new KHotKeys::MenuEntryAction( data ));
            break;

        case KHotKeys::Action::CommandUrlActionType:
            data->set_action( new KHotKeys::CommandUrlAction( data ));
            break;

        case KHotKeys::Action::DBusActionType:
            data->set_action( new KHotKeys::DBusAction( data ));
            break;

        default:
            Q_ASSERT(false);
            return;
        }

    QModelIndex newAct = _view->model()->insertActionData(data, parent);
    _view->setCurrentIndex(newAct);
    _view->edit(newAct);
    _view->resizeColumnToContents(0);
    }


void HotkeysTreeViewContextMenu::newGroupAction()
    {
    QModelIndex parent;      // == root element
    if (!_index.isValid()
        || _view->model()->data( _index.sibling( _index.row(), KHotkeysModel::IsGroupColumn)).toBool())
        {
        // if the index is invalid (root index) or represents an group use it.
        parent = _index;
        }
    else
        {
        // It is an action. Take the parent.
        parent = _index.parent();
        }

    QModelIndex newGroup = _view->model()->addGroup(parent);
    _view->setCurrentIndex(newGroup);
    _view->edit(newGroup);
    _view->resizeColumnToContents(0);
    }


void HotkeysTreeViewContextMenu::deleteAction()
    {
    if (!_index.isValid())
        {
        Q_ASSERT( _index.isValid() );
        return;
        }
    Q_ASSERT( _view->model()->removeRow(_index.row(), _index.parent()) );

    _view->setCurrentIndex(QModelIndex());
    }


HotkeysTreeView::HotkeysTreeView( QWidget *parent )
    : QTreeView(parent)
    {
    setObjectName("khotkeys treeview");
    setAllColumnsShowFocus(true);
    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(true);
    }


HotkeysTreeView::~HotkeysTreeView()
    {}


void
HotkeysTreeView::contextMenuEvent( QContextMenuEvent *event )
    {
    QModelIndex index = indexAt(event->pos());
    // KHotKeys::ActionDataBase *item = model()->indexToActionDataBase(index);
    HotkeysTreeViewContextMenu menu( index, this );
    menu.exec(event->globalPos());
    }


void
HotkeysTreeView::modelReset()
    {
    kDebug();
    resizeColumnToContents(0);
    }


KHotkeysModel *HotkeysTreeView::model()
    {
    return dynamic_cast<KHotkeysModel*>( QTreeView::model() );
    }


void
HotkeysTreeView::setModel( QAbstractItemModel *model )
    {
    if (!dynamic_cast<KHotkeysModel*>( model ))
        {
        Q_ASSERT(dynamic_cast<KHotkeysModel*>( model ));
        return;
        }
    QTreeView::setModel(model);

    setAllColumnsShowFocus(true);
    setAlternatingRowColors(true);

    setSelectionBehavior( QAbstractItemView::SelectRows );
    setSelectionMode( QAbstractItemView::SingleSelection );

    connect(
        model, SIGNAL(modelReset()),
        this, SLOT(modelReset()));
    }

#include "moc_hotkeys_tree_view.cpp"
