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
#ifndef HOTKEYS_TREE_VIEW_H
#define HOTKEYS_TREE_VIEW_H

#include "libkhotkeysfwd.h"


#include <QtGui/QMenu>
#include <QtGui/QTreeView>


class KHotkeysModel;

/**
 * @author Michael Jansen <kde@michael-jansen.biz>
 */
class HotkeysTreeView : public QTreeView
    {
    Q_OBJECT

public:

    /**
     * Default constructor
     */
    HotkeysTreeView( QWidget *parent = 0 );

    /**
     * Destructor
     */
    virtual ~HotkeysTreeView();


    /**
     * The user requested a context menu
     */
    void contextMenuEvent( QContextMenuEvent *event );

    /**
     * Set a new model
     */
    void setModel( QAbstractItemModel *model );
    KHotkeysModel *model();

};


class HotkeysTreeViewContextMenu : public QMenu
    {
    Q_OBJECT

public:

    HotkeysTreeViewContextMenu( const QModelIndex &index, HotkeysTreeView *parent = 0 );
    virtual ~HotkeysTreeViewContextMenu();


private Q_SLOTS:

    void deleteAction();
    void newGlobalShortcutActionAction(int);
    void newGroupAction();

private:

    QModelIndex _index;
    HotkeysTreeView *_view;
    };



#endif /* #ifndef HOTKEYS_TREE_VIEW_H */
