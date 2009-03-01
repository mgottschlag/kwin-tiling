#ifndef HOTKEYS_CONTEXT_MENU_H
#define HOTKEYS_CONTEXT_MENU_H
/**
 * Copyright (C) 2009 Michael Jansen <kde@michael-jansen.biz>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB. If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "triggers/triggers.h"
#include "actions/actions.h"

#include "libkhotkeysfwd.h"

#include <QtGui/QMenu>
#include <QtCore/QModelIndex>


class HotkeysTreeView;

class QModelIndex;
class QSignalMapper;


class HotkeysTreeViewContextMenu : public QMenu
    {
    Q_OBJECT

public:

    HotkeysTreeViewContextMenu( const QModelIndex &index, HotkeysTreeView *parent = 0 );
    HotkeysTreeViewContextMenu( HotkeysTreeView *parent = 0 );

    virtual ~HotkeysTreeViewContextMenu();

    //! Create a submenu per allowed trigger type
    void createTriggerMenus(
        KHotKeys::Trigger::TriggerTypes triggerTypes,
        KHotKeys::Action::ActionTypes actionTypes);

    //! Populate a trigger menu
    void populateTriggerMenu(QMenu *menu, QSignalMapper *mapper, KHotKeys::Action::ActionTypes types);

private Q_SLOTS:

    void slotAboutToShow();
    void slotAboutToShowForCurrent();
    void deleteAction();

    void exportAction();
    void importAction();

    void newGlobalShortcutActionAction(int);
    void newWindowTriggerActionAction(int);
    void newMouseGestureTriggerActionAction(int);
    void newGroupAction();

private:

    KHotKeys::Action* createActionFromType(
            int type,
            KHotKeys::SimpleActionData *data) const;

    QModelIndex _index;
    HotkeysTreeView *_view;
    };



#endif /* HOTKEYS_CONTEXT_MENU_H */

