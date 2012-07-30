/*
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

#ifndef menuinfo_h
#define menuinfo_h


#include <QList>

#include <KShortcut>
#include <KService>

class MenuFile;
class MenuEntryInfo;

class MenuInfo
{
public:
    MenuInfo() {}
    virtual ~MenuInfo() {}
};

class MenuSeparatorInfo : public MenuInfo
{
public:
    MenuSeparatorInfo() {}
};

class MenuFolderInfo : public MenuInfo
{
public:
    MenuFolderInfo() : dirty(false), hidden(false) { }
    ~MenuFolderInfo() { qDeleteAll(subFolders); subFolders.clear(); }
    // Add separator
    void add(MenuSeparatorInfo *, bool initial=false);

    // Add sub menu
    void add(MenuFolderInfo *, bool initial=false);

    // Remove sub menu (without deleting it)
    void take(MenuFolderInfo *);

    // Remove sub menu (without deleting it)
    // @return true if found
    bool takeRecursive(MenuFolderInfo *info);

    // Add entry
    void add(MenuEntryInfo *, bool initial = false);

    // Remove entry (without deleting it)
    void take(MenuEntryInfo *);

    // Return a unique sub-menu caption inspired by @p caption
    QString uniqueMenuCaption(const QString &caption);

    // Return a unique item caption inspired by @p caption but different
    // from @p exclude
    QString uniqueItemCaption(const QString &caption, const QString &exclude = QString());

    // Update full id's for this item and all submenus
    void updateFullId(const QString &parentId);

    // Return a list of existing submenu ids
    QStringList existingMenuIds();

    void setCaption(const QString &_caption)
    {
       if (_caption == caption) return;
       caption = _caption;
       setDirty();
    }

    void setIcon(const QString &_icon)
    {
       if (_icon == icon) return;
       icon = _icon;
       setDirty();
    }

    void setGenericName(const QString &_description)
    {
       if (_description == genericname) return;
       genericname = _description;
       setDirty();
    }

    void setComment(const QString &_comment)
    {
       if (_comment == comment) return;
       comment = _comment;
       setDirty();
    }

    // Mark menu as dirty
    void setDirty();

    // Return whether this menu or any entry or submenu contained in it is dirty.
    bool hasDirt();

    // Return whether this menu should be explicitly added to its parent menu
    bool needInsertion();

    // Save menu and all its entries and submenus
    void save(MenuFile *);

    // Search service by shortcut
    KService::Ptr findServiceShortcut(const KShortcut&);

    // Set whether the entry is in active use (as opposed to in the clipboard/deleted)
    void setInUse(bool inUse);

public:
    QString id; // Relative to parent
    QString fullId; // Name in tree
    QString caption; // Visible name
    QString genericname; // Generic description
    QString comment; // Comment
    QString directoryFile; // File describing this folder.
    QString icon; // Icon
    QList<MenuFolderInfo *> subFolders; // Sub menus in this folder
    QList<MenuEntryInfo *> entries; // Menu entries in this folder
    QList<MenuInfo *> initialLayout; // Layout of menu entries according to sycoca
    bool dirty;
    bool hidden;
};

class MenuEntryInfo : public MenuInfo
{
public:
    explicit MenuEntryInfo(const KService::Ptr &_service, KDesktopFile *_df = 0)
     : service(_service), m_desktopFile(_df),
       shortcutLoaded(false), shortcutDirty(false), dirty(_df != 0), hidden(false)
    {
       caption = service->name();
       description = service->genericName();
       icon = service->icon();
    }
    ~MenuEntryInfo();

    void setCaption(const QString &_caption);
    void setDescription(const QString &_description);
    void setIcon(const QString &_icon);

    QString menuId() const { return service->menuId(); }

    QString file() const { return service->entryPath(); }

    KShortcut shortcut();
    void setShortcut(const KShortcut &_shortcut);
    bool isShortcutAvailable(const KShortcut &_shortcut);

    void setDirty();

    // Set whether the entry is in active use (as opposed to in the clipboard/deleted)
    void setInUse(bool inUse);

    // Return whether this menu should be explicitly added to its parent menu
    bool needInsertion();

    void save();

    KDesktopFile *desktopFile();

public:
    QString caption;
    QString description;
    QString icon;
    KService::Ptr service;
    KDesktopFile *m_desktopFile;
    KShortcut shortCut;
    bool shortcutLoaded;
    bool shortcutDirty;
    bool dirty;
    bool hidden;
};

#endif
