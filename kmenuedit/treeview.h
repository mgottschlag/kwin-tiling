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
 *   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#ifndef __treeview_h__
#define __treeview_h__

#include <qstring.h>
#include <klistview.h>
#include <kservice.h>
#include <kservicegroup.h>

class QPopupMenu;
class KActionCollection;
class KDesktopFile;
class MenuFile;

class FolderInfo 
{
public:
    FolderInfo() : hidden(false) { subFolders.setAutoDelete(true); }

    // Add sub menu
    void add(FolderInfo *);

    // Remove sub menu (without deleting it)
    void take(FolderInfo *);

    // Remove sub menu (without deleting it)
    // @return true if found
    bool takeRecursive(FolderInfo *info);
    
    // Add entry
    void add(KService *);
    
    // Remove entry
    void take(const QString &file);

    // Return a unique sub-menu caption inspired by @p caption
    QString uniqueMenuCaption(const QString &caption);

    // Return a unique item caption inspired by @p caption but different
    // from @p exclude
    QString uniqueItemCaption(const QString &caption, const QString &exclude = QString::null);

    // Return a list of existing submenu ids
    QStringList existingMenuIds();
    
public:
    QString id; // Relative to parent
    QString fullId; // Name in tree
    QString caption; // Visible name
    QString directoryFile; // File describing this folder.
    QString icon; // Icon
    QPtrList<FolderInfo> subFolders; // Sub menus in this folder
    KService::List entries; // Menu entries in this folder
    bool hidden;
};

class TreeItem : public QListViewItem
{
public:
    TreeItem(QListViewItem *parent, const QString& file);
    TreeItem(QListViewItem *parent, QListViewItem *after, const QString& file);
    TreeItem(QListView *parent, const QString& file);
    TreeItem(QListView *parent, QListViewItem* after, const QString& file);

    QString file() const { return _file; }
    void setFile(const QString& file) { _file = file; }
    QString directory() const { return _directoryPath; }
    void setDirectoryPath(const QString& path) { _directoryPath = path; }

    FolderInfo *folderInfo() { return m_folderInfo; }
    void setFolderInfo(FolderInfo *folderInfo) { m_folderInfo = folderInfo; }

    QString name() const { return _name; }
    void setName(const QString &name);
    
    bool isDirectory() const { return m_folderInfo; }
    
    bool isHidden() const { return _hidden; }
    void setHidden(bool b);

    virtual void setOpen(bool o);
    void load();

private:
    void update();

    bool _hidden : 1;
    bool _init : 1;
    QString _file;
    QString _name;
    QString _directoryPath;
    FolderInfo *m_folderInfo;
};

class TreeView : public KListView
{
    friend class TreeItem;
    Q_OBJECT
public:
    TreeView(KActionCollection *ac, QWidget *parent=0, const char *name=0);
    ~TreeView();

    void readFolderInfo(FolderInfo *folderInfo=0, KServiceGroup::Ptr folder=0, const QString &prefix=QString::null);
    void setViewMode(bool showHidden);

public slots:
    void currentChanged(const QString& desktopFile);

signals:
    void entrySelected(const QString&, const QString &, bool);

protected slots:
    void itemSelected(QListViewItem *);
    void slotDropped(QDropEvent *, QListViewItem *, QListViewItem *);
    void slotRMBPressed(QListViewItem*, const QPoint&);

    void newsubmenu();
    void newitem();

    void cut();
    void copy();
    void paste();
    void del();
    void undel();

protected:
    TreeItem *createTreeItem(TreeItem *parent, QListViewItem *after, FolderInfo *folderInfo);
    TreeItem *createTreeItem(TreeItem *parent, QListViewItem *after, KService *s);

    void del(TreeItem *, bool deleteInfo);
    void undel(TreeItem *);
    void fill();
    void fillBranch(FolderInfo *folderInfo, TreeItem *parent);
    QString findName(KDesktopFile *df, bool deleted);

    // moving = src will be removed later
    void copy( bool moving );

    void cleanupClipboard();

    QStringList fileList(const QString& relativePath);
    QStringList dirList(const QString& relativePath);

    virtual bool acceptDrag(QDropEvent* event) const;
    virtual QDragObject *dragObject();
    virtual void startDrag();

private:
    KActionCollection *m_ac;
    QPopupMenu        *m_rmb;
    QString            m_clipboard;
    FolderInfo        *m_clipboardInfo;
    QString            m_drag;
    FolderInfo        *m_dragInfo;
    TreeItem          *m_dragItem;
    bool               m_showHidden;
    MenuFile          *m_menuFile;
    FolderInfo         m_rootFolder;
};


#endif
