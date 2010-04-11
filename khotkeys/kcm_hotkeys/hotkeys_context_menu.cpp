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
#include "hotkeys_context_menu.h"
#include "hotkeys_export_widget.h"

#include "hotkeys_model.h"

#include "actions/actions.h"
#include "action_data/action_data_group.h"
#include "action_data/simple_action_data.h"

#include <KDE/KDebug>
#include <KDE/KFileDialog>
#include <KDE/KLocale>
#include <KDE/KUrl>

#include <QtCore/QSignalMapper>
#include <QtGui/QContextMenuEvent>


HotkeysTreeViewContextMenu::HotkeysTreeViewContextMenu( const QModelIndex &index, HotkeysTreeView *parent )
        : QMenu(parent)
         ,_index(index)
         ,_view(parent)
    {
    setTitle( i18n("Test") );

    connect(this, SIGNAL(aboutToShow()),
            SLOT(slotAboutToShow()));
    }


HotkeysTreeViewContextMenu::HotkeysTreeViewContextMenu( HotkeysTreeView *parent )
        : QMenu(parent)
         ,_index()
         ,_view(parent)
    {
    setTitle( i18n("Test") );

    connect(this, SIGNAL(aboutToShow()),
            SLOT(slotAboutToShowForCurrent()));
    }


HotkeysTreeViewContextMenu::~HotkeysTreeViewContextMenu()
    {}


KHotKeys::Action *
HotkeysTreeViewContextMenu::createActionFromType(
        int actionType,
        KHotKeys::SimpleActionData* data
        ) const
    {
    KHotKeys::Action *action = NULL;
    switch (actionType)
        {
        case KHotKeys::Action::CommandUrlActionType:
            action = new KHotKeys::CommandUrlAction( data );
            break;

        case KHotKeys::Action::DBusActionType:
            action = new KHotKeys::DBusAction( data );
            break;

        case KHotKeys::Action::KeyboardInputActionType:
            action = new KHotKeys::KeyboardInputAction( data );
            break;

        case KHotKeys::Action::MenuEntryActionType:
            action = new KHotKeys::MenuEntryAction( data );
            break;

        default:
            Q_ASSERT(false);
            return NULL;
        }

    data->set_action(action);
    return action;
    }


void HotkeysTreeViewContextMenu::slotAboutToShowForCurrent()
    {
    _index = _view->currentIndex();

    slotAboutToShow();
    }


void HotkeysTreeViewContextMenu::slotAboutToShow()
    {
    clear();

    if (_index.isValid())
        {
        KHotKeys::ActionDataBase *element = _view->model()->indexToActionDataBase(_index);
        KHotKeys::ActionDataGroup *group =  _view->model()->indexToActionDataGroup(_index);
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
            // Item related actions
            addAction( i18n("Delete"), this, SLOT(deleteAction()) );
            }
        }
    else
        {
        createTriggerMenus(KHotKeys::Trigger::AllTypes, KHotKeys::Action::AllTypes);
        addAction( i18n("New Group") , this, SLOT(newGroupAction()) );
        }

    addSeparator();
    addAction( i18n("Export Group..."), this, SLOT(exportAction()) );
    addAction( i18n("Import..."), this, SLOT(importAction()) );
    }


void HotkeysTreeViewContextMenu::createTriggerMenus(
        KHotKeys::Trigger::TriggerTypes triggerTypes,
        KHotKeys::Action::ActionTypes actionTypes)
    {
    QMenu *newMenu = new QMenu(i18n("New"));

    if (triggerTypes & KHotKeys::Trigger::ShortcutTriggerType)
        {
        QSignalMapper *mapper = new QSignalMapper(this);

        QMenu *menu = new QMenu( i18n("Global Shortcut") );
        populateTriggerMenu(menu, mapper, actionTypes);
        newMenu->addMenu(menu);

        connect(
            mapper, SIGNAL(mapped(int)),
            this, SLOT(newGlobalShortcutActionAction(int)) );
        }

    if (triggerTypes & KHotKeys::Trigger::WindowTriggerType)
        {
        QSignalMapper *mapper = new QSignalMapper(this);

        QMenu *menu = new QMenu( i18n("Window Action") );
        populateTriggerMenu(menu, mapper, actionTypes);
        newMenu->addMenu(menu);

        connect(
            mapper, SIGNAL(mapped(int)),
            this, SLOT(newWindowTriggerActionAction(int)) );
        }

    if (triggerTypes & KHotKeys::Trigger::GestureTriggerType)
        {
        QSignalMapper *mapper = new QSignalMapper(this);

        QMenu *menu = new QMenu( i18n("Mouse Gesture Action") );
        populateTriggerMenu(menu, mapper, actionTypes);
        newMenu->addMenu(menu);

        connect(
            mapper, SIGNAL(mapped(int)),
            this, SLOT(newMouseGestureTriggerActionAction(int)) );
        }

    addMenu(newMenu);
    }


void HotkeysTreeViewContextMenu::importAction()
    {
    KUrl url = KFileDialog::getOpenFileName(KUrl(), "*.khotkeys", this);
    if (!url.isEmpty())
        {
        KConfig config(url.path(), KConfig::SimpleConfig);
        _view->model()->importInputActions(_index, config);
        }
    }


void HotkeysTreeViewContextMenu::exportAction()
    {
    KHotkeysExportDialog *widget = new KHotkeysExportDialog(this);

    KHotKeys::ActionDataGroup *group =  _view->model()->indexToActionDataGroup(_index);
    if (!group)
        group = _view->model()->indexToActionDataBase(_index)->parent();

    widget->setImportId(group->importId());
    widget->setAllowMerging(group->allowMerging());

    if (widget->exec() == QDialog::Accepted)
        {
        KHotKeys::ActionState state;
        switch (widget->state())
            {
            case 0:
                state = KHotKeys::Retain;
                break;

            case 1:
                state = KHotKeys::Enabled;
                break;

            case 2:
                state = KHotKeys::Disabled;
                break;

            default:
                // Unknown value alled to our ui file. Use disabled as a
                // default.
                Q_ASSERT(false);
                state = KHotKeys::Disabled;
                break;
            }

        QString id = widget->importId();
        KUrl url   = widget->url();
        bool allowMerging = widget->allowMerging();
        if (!url.isEmpty())
            {
            KConfig config(url.path(), KConfig::SimpleConfig);
            _view->model()->exportInputActions(_index, config, id, state, allowMerging);
            }
        }
    delete widget;
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

    if (types & KHotKeys::Action::KeyboardInputActionType)
        {
        mapper->setMapping(
            menu->addAction( i18n("Send Keyboard Input"), mapper, SLOT(map()) ),
            KHotKeys::Action::KeyboardInputActionType );
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
        new KHotKeys::SimpleActionData( 0, i18n("New Action"), i18n("Comment"));
    data->set_trigger( new KHotKeys::ShortcutTrigger( data, KShortcut() ) );
    data->enable();

    createActionFromType(actionType, data);

    QModelIndex newAct = _view->model()->insertActionData(data, parent);
    _view->setCurrentIndex(newAct);
    _view->edit(newAct);
    _view->resizeColumnToContents(KHotkeysModel::NameColumn);
    }


void HotkeysTreeViewContextMenu::newMouseGestureTriggerActionAction( int actionType )
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
        new KHotKeys::SimpleActionData( 0, i18n("New Action"), i18n("Comment"));
    data->set_trigger( new KHotKeys::GestureTrigger(data) );
    data->enable();

    createActionFromType(actionType, data);

    QModelIndex newAct = _view->model()->insertActionData(data, parent);
    _view->setCurrentIndex(newAct);
    _view->edit(newAct);
    _view->resizeColumnToContents(KHotkeysModel::NameColumn);
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
        new KHotKeys::SimpleActionData( 0, i18n("New Action"), i18n("Comment"));
    data->set_trigger( new KHotKeys::WindowTrigger(data) );
    data->enable();

    createActionFromType(actionType, data);

    QModelIndex newAct = _view->model()->insertActionData(data, parent);
    _view->setCurrentIndex(newAct);
    _view->edit(newAct);
    _view->resizeColumnToContents(KHotkeysModel::NameColumn);
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
    _view->resizeColumnToContents(KHotkeysModel::NameColumn);
    }


void HotkeysTreeViewContextMenu::deleteAction()
    {
    if (!_index.isValid())
        {
        Q_ASSERT( _index.isValid() );
        return;
        }

    bool deletionSuccess;
    deletionSuccess = _view->model()->removeRow(_index.row(), _index.parent());
    Q_ASSERT(deletionSuccess == true);

    _view->setCurrentIndex(QModelIndex());
    }


#include "moc_hotkeys_context_menu.cpp"

