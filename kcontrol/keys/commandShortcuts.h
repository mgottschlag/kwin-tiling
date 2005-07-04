/*
 * commandShortcuts.h
 *
 * Copyright (c) 2003 Aaron J. Seigo
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
 *  Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef __COMMAND_SHORTCUTS_MODULE_H
#define __COMMAND_SHORTCUTS_MODULE_H

#include <qtabwidget.h>
#include <kshortcut.h>
#include <qptrlist.h>

class AppTreeView;
class AppTreeItem;
class QButtonGroup;
class QRadioButton;
class KKeyButton;
class QListViewItem;

typedef QPtrList<AppTreeItem> treeItemList;
typedef QPtrListIterator<AppTreeItem> treeItemListIterator;

class CommandShortcutsModule : public QWidget
{
    Q_OBJECT
    public:
        CommandShortcutsModule( QWidget *parent = 0, const char *name = 0 );
        ~CommandShortcutsModule();

        void load();
        void save();
        void defaults();
        QString quickHelp() const;

    signals:
        void changed( bool );

    public slots:
        void showing(QWidget*);

    protected slots:
        void commandSelected(const QString&, const QString &, bool);
        void shortcutChanged(const KShortcut& shortcut);
        void shortcutRadioToggled(bool remove);
        void commandDoubleClicked(QListViewItem *item, const QPoint &, int);
        void launchMenuEditor();

    protected:
        void initGUI();

    private:
        AppTreeView* m_tree;
        QButtonGroup* m_shortcutBox;
        QRadioButton* m_noneRadio;
        QRadioButton* m_customRadio;
        KKeyButton* m_shortcutButton;
        treeItemList m_changedItems;
};

#endif // __COMMAND_SHORTCUTS_MODULE_H
