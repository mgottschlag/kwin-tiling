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

#include "hotkeys_model.h"

#include "actions/actions.h"
#include "action_data/action_data_group.h"
#include "action_data/simple_action_data.h"

#include "KDE/KLocale"
#include "KDE/KDebug"

#include <QtGui/QContextMenuEvent>


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
    resizeColumnToContents(KHotkeysModel::NameColumn);
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

    resizeColumnToContents(KHotkeysModel::EnabledColumn);
    resizeColumnToContents(KHotkeysModel::NameColumn);
    }

#include "moc_hotkeys_tree_view.cpp"
