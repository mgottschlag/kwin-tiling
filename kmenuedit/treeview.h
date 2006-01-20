/*
 *   Copyright (C) 2000 Matthias Elter <elter@kde.org>
 *   Copyright (C) 2001-2002 Raffaele Sandrini <sandrini@kde.org>
 *   Copyright (C) 2003 Waldo Bastian <bastian@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef __treeview_h__
#define __treeview_h__

#include <qstring.h>
//Added by qt3to4:
#include <QMenu>
#include <QDropEvent>
#include <klistview.h>
#include <kservice.h>
#include <kservicegroup.h>

class QMenu;
class KActionCollection;
class KDesktopFile;
class MenuFile;
class MenuFolderInfo;
class MenuEntryInfo;
class MenuSeparatorInfo;
class KShortcut;

class TreeItem : public Q3ListViewItem
{
public:
  TreeItem(Q3ListViewItem *parent, Q3ListViewItem *after, const QString &menuIdn, bool __init = false);
    TreeItem(Q3ListView *parent, Q3ListViewItem* after, const QString &menuId, bool __init = false);

    QString menuId() const { return _menuId; }

    QString directory() const { return _directoryPath; }
    void setDirectoryPath(const QString& path) { _directoryPath = path; }

    MenuFolderInfo *folderInfo() { return m_folderInfo; }
    void setMenuFolderInfo(MenuFolderInfo *folderInfo) { m_folderInfo = folderInfo; }

    MenuEntryInfo *entryInfo() { return m_entryInfo; }
    void setMenuEntryInfo(MenuEntryInfo *entryInfo) { m_entryInfo = entryInfo; }

    QString name() const { return _name; }
    void setName(const QString &name);

    bool isDirectory() const { return m_folderInfo; }
    bool isEntry() const { return m_entryInfo; }

    bool isHidden() const { return _hidden; }
    void setHidden(bool b);

    bool isLayoutDirty() { return _layoutDirty; }
    void setLayoutDirty() { _layoutDirty = true; }
    QStringList layout();

    virtual void setOpen(bool o);
    void load();

    virtual void paintCell(QPainter * p, const QColorGroup & cg, int column, int width, int align);
    virtual void setup();

private:
    void update();

    bool _hidden : 1;
    bool _init : 1;
    bool _layoutDirty : 1;
    QString _menuId;
    QString _name;
    QString _directoryPath;
    MenuFolderInfo *m_folderInfo;
    MenuEntryInfo *m_entryInfo;
};

class TreeView : public KListView
{
    friend class TreeItem;
    Q_OBJECT
public:
    TreeView(bool controlCenter, KActionCollection *ac, QWidget *parent=0, const char *name=0);
    ~TreeView();

    void readMenuFolderInfo(MenuFolderInfo *folderInfo=0, KServiceGroup::Ptr folder=0, const QString &prefix=QString());
    void setViewMode(bool showHidden);
    bool save();

    bool dirty();

    void selectMenu(const QString &menu);
    void selectMenuEntry(const QString &menuEntry);
    
public Q_SLOTS:
    void currentChanged(MenuFolderInfo *folderInfo);
    void currentChanged(MenuEntryInfo *entryInfo);
    void findServiceShortcut(const KShortcut&, KService::Ptr &);

Q_SIGNALS:
    void entrySelected(MenuFolderInfo *folderInfo);
    void entrySelected(MenuEntryInfo *entryInfo);
    void disableAction();
protected Q_SLOTS:
    void itemSelected(Q3ListViewItem *);
    void slotDropped(QDropEvent *, Q3ListViewItem *, Q3ListViewItem *);
    void slotRMBPressed(Q3ListViewItem*, const QPoint&);

    void newsubmenu();
    void newitem();
    void newsep();

    void cut();
    void copy();
    void paste();
    void del();

protected:
    TreeItem *createTreeItem(TreeItem *parent, Q3ListViewItem *after, MenuFolderInfo *folderInfo, bool _init = false);
    TreeItem *createTreeItem(TreeItem *parent, Q3ListViewItem *after, MenuEntryInfo *entryInfo, bool _init = false);
    TreeItem *createTreeItem(TreeItem *parent, Q3ListViewItem *after, MenuSeparatorInfo *sepInfo, bool _init = false);

    void del(TreeItem *, bool deleteInfo);
    void fill();
    void fillBranch(MenuFolderInfo *folderInfo, TreeItem *parent);
    QString findName(KDesktopFile *df, bool deleted);

    void closeAllItems(Q3ListViewItem *item);

    // moving = src will be removed later
    void copy( bool moving );

    void cleanupClipboard();
    
    bool isLayoutDirty();
    void setLayoutDirty(TreeItem *);
    void saveLayout();

    QStringList fileList(const QString& relativePath);
    QStringList dirList(const QString& relativePath);

    virtual bool acceptDrag(QDropEvent* event) const;
    virtual Q3DragObject *dragObject();
    virtual void startDrag();

private:
    KActionCollection *m_ac;
    QMenu        *m_rmb;
    int                m_clipboard;
    MenuFolderInfo    *m_clipboardFolderInfo;
    MenuEntryInfo     *m_clipboardEntryInfo;
    int                m_drag;
    MenuFolderInfo    *m_dragInfo;
    TreeItem          *m_dragItem;
    QString            m_dragPath;
    bool               m_showHidden;
    bool               m_controlCenter;
    MenuFile          *m_menuFile;
    MenuFolderInfo    *m_rootFolder;
    MenuSeparatorInfo *m_separator;
    QStringList        m_newMenuIds;
    QStringList        m_newDirectoryList;
    bool               m_detailedMenuEntries;
    bool               m_detailedEntriesNamesFirst;
    bool               m_layoutDirty;
};


#endif
