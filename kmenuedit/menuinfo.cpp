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
 *   Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "menuinfo.h"
#include "menufile.h"

#include <qregexp.h>

#include <kdesktopfile.h>
#include <khotkeys.h>
#include <kstandarddirs.h>

//
// MenuFolderInfo
//

static QStringList *s_allShortcuts = 0;
static QStringList *s_newShortcuts = 0;
static QStringList *s_freeShortcuts = 0;
static QStringList *s_deletedApps = 0;

// Add separator
void MenuFolderInfo::add(MenuSeparatorInfo *info, bool initial)
{
   if (initial)
      initialLayout.append(info);
}

// Add sub menu
void MenuFolderInfo::add(MenuFolderInfo *info, bool initial)
{
   subFolders.append(info);
   if (initial)
      initialLayout.append(info);
}

// Remove sub menu (without deleting it)
void MenuFolderInfo::take(MenuFolderInfo *info)
{
   subFolders.take(subFolders.findRef(info));
}

// Remove sub menu (without deleting it)
bool MenuFolderInfo::takeRecursive(MenuFolderInfo *info)
{
   int i = subFolders.findRef(info);
   if (i >= 0)
   {
      subFolders.take(i);
      return true;
   }

   for(MenuFolderInfo *subFolderInfo = subFolders.first();
       subFolderInfo; subFolderInfo = subFolders.next())
   {
      if (subFolderInfo->takeRecursive(info))
         return true;
   }
   return false;
}

// Recursively update all fullIds
void MenuFolderInfo::updateFullId(const QString &parentId)
{
   fullId = parentId + id;

   for(MenuFolderInfo *subFolderInfo = subFolders.first();
       subFolderInfo; subFolderInfo = subFolders.next())
   {
      subFolderInfo->updateFullId(fullId);
   }
}

// Add entry
void MenuFolderInfo::add(MenuEntryInfo *entry, bool initial)
{
   entries.append(entry);
   if (initial)
      initialLayout.append(entry);
}

// Remove entry
void MenuFolderInfo::take(MenuEntryInfo *entry)
{
   entries.removeRef(entry);
}


// Return a unique sub-menu caption inspired by @p caption
QString MenuFolderInfo::uniqueMenuCaption(const QString &caption)
{
   QRegExp r("(.*)(?=-\\d+)");
   QString cap = (r.search(caption) > -1) ? r.cap(1) : caption;

   QString result = caption;

   for(int n = 1; ++n; )
   {
      bool ok = true;
      for(MenuFolderInfo *subFolderInfo = subFolders.first();
          subFolderInfo; subFolderInfo = subFolders.next())
      {
         if (subFolderInfo->caption == result)
         {
            ok = false;
            break;
         }
      }
      if (ok)
         return result;

      result = cap + QString("-%1").arg(n);
   }
   return QString::null; // Never reached
}

// Return a unique item caption inspired by @p caption
QString MenuFolderInfo::uniqueItemCaption(const QString &caption, const QString &exclude)
{
   QRegExp r("(.*)(?=-\\d+)");
   QString cap = (r.search(caption) > -1) ? r.cap(1) : caption;

   QString result = caption;

   for(int n = 1; ++n; )
   {
      bool ok = true;
      if (result == exclude)
         ok = false;
      MenuEntryInfo *entryInfo;
      for(QPtrListIterator<MenuEntryInfo> it(entries);
          ok && (entryInfo = it.current()); ++it)
      {
         if (entryInfo->caption == result)
            ok = false;
      }
      if (ok)
         return result;

      result = cap + QString("-%1").arg(n);
   }
   return QString::null; // Never reached
}

// Return a list of existing submenu ids
QStringList MenuFolderInfo::existingMenuIds()
{
   QStringList result;
   for(MenuFolderInfo *subFolderInfo = subFolders.first();
       subFolderInfo; subFolderInfo = subFolders.next())
   {
       result.append(subFolderInfo->id);
   }
   return result;
}

void MenuFolderInfo::setDirty()
{
   dirty = true;
}

void MenuFolderInfo::save(MenuFile *menuFile)
{
   if (s_deletedApps)
   {
      // Remove hotkeys for applications that have been deleted
      for(QStringList::ConstIterator it = s_deletedApps->begin();
          it != s_deletedApps->end(); ++it)
      {
         KHotKeys::menuEntryDeleted(*it);
      }
      delete s_deletedApps;
      s_deletedApps = 0;
   }

   if (dirty)
   {
      QString local = KDesktopFile::locateLocal(directoryFile);

      KConfig *df = 0;
      if (directoryFile != local)
      {
         KConfig orig(directoryFile, true, false, "apps");
         df = orig.copyTo(local);
      }
      else
      {
         df = new KConfig(directoryFile, false, false, "apps");
      }

      df->setDesktopGroup();
      df->writeEntry("Name", caption);
      df->writeEntry("GenericName", genericname);
      df->writeEntry("Comment", comment);
      df->writeEntry("Icon", icon);
      df->sync();
      delete df;
      dirty = false;
   }

   // Save sub-menus
   for(MenuFolderInfo *subFolderInfo = subFolders.first();
       subFolderInfo; subFolderInfo = subFolders.next())
   {
      subFolderInfo->save(menuFile);
   }

   // Save entries
   MenuEntryInfo *entryInfo;
   for(QPtrListIterator<MenuEntryInfo> it(entries);
       (entryInfo = it.current()); ++it)
   {
      if (entryInfo->needInsertion())
         menuFile->addEntry(fullId, entryInfo->menuId());
      entryInfo->save();
   }
}

bool MenuFolderInfo::hasDirt()
{
   if (dirty) return true;

   // Check sub-menus
   for(MenuFolderInfo *subFolderInfo = subFolders.first();
       subFolderInfo; subFolderInfo = subFolders.next())
   {
      if (subFolderInfo->hasDirt()) return true;
   }

   // Check entries
   MenuEntryInfo *entryInfo;
   for(QPtrListIterator<MenuEntryInfo> it(entries);
       (entryInfo = it.current()); ++it)
   {
      if (entryInfo->dirty) return true;
      if (entryInfo->shortcutDirty) return true;
   }
   return false;
}

KService::Ptr MenuFolderInfo::findServiceShortcut(const KShortcut&cut)
{
   KService::Ptr result;
   // Check sub-menus
   for(MenuFolderInfo *subFolderInfo = subFolders.first();
       subFolderInfo; subFolderInfo = subFolders.next())
   {
      result = subFolderInfo->findServiceShortcut(cut);
      if (result)
          return result;
   }

   // Check entries
   MenuEntryInfo *entryInfo;
   for(QPtrListIterator<MenuEntryInfo> it(entries);
       (entryInfo = it.current()); ++it)
   {
      if (entryInfo->shortCut == cut)
         return entryInfo->service;
   }
   return 0;
}

void MenuFolderInfo::setInUse(bool inUse)
{
   // Propagate to sub-menus
   for(MenuFolderInfo *subFolderInfo = subFolders.first();
       subFolderInfo; subFolderInfo = subFolders.next())
   {
      subFolderInfo->setInUse(inUse);
   }

   // Propagate to entries
   MenuEntryInfo *entryInfo;
   for(QPtrListIterator<MenuEntryInfo> it(entries);
       (entryInfo = it.current()); ++it)
   {
      entryInfo->setInUse(inUse);
   }
}

//
// MenuEntryInfo
//

MenuEntryInfo::~MenuEntryInfo()
{
   df->rollback(false);
   delete df;
}

KDesktopFile *MenuEntryInfo::desktopFile()
{
   if (!df)
   {
      df = new KDesktopFile(service->desktopEntryPath());
   }
   return df;
}

void MenuEntryInfo::setDirty()
{
   if (dirty) return;

   dirty = true;

   QString local = locateLocal("xdgdata-apps", service->menuId());
   if (local != service->desktopEntryPath())
   {
      KDesktopFile *oldDf = desktopFile();
      df = oldDf->copyTo(local);
      df->setDesktopGroup();
      delete oldDf;
   }
}

bool MenuEntryInfo::needInsertion()
{
   // If entry is dirty and previously stored under applnk, then we need to be added explicity 
   return dirty && !service->desktopEntryPath().startsWith("/");
}

void MenuEntryInfo::save()
{
   if (dirty)
   {
      df->sync();
      dirty = false;
   }

   if (shortcutDirty)
   {
      if( KHotKeys::present())
      {
         KHotKeys::changeMenuEntryShortcut( service->storageId(), shortCut.toStringInternal() );
      }
      shortcutDirty = false;
   }
}

void MenuEntryInfo::setCaption(const QString &_caption)
{
   if (caption == _caption)
      return;
   caption = _caption;
   setDirty();
   desktopFile()->writeEntry("Name", caption);
}

void MenuEntryInfo::setDescription(const QString &_description)
{
    if (description == _description)
        return;
    description = _description;
    setDirty();
    desktopFile()->writeEntry("GenericName", description);
}

void MenuEntryInfo::setIcon(const QString &_icon)
{
   if (icon == _icon)
      return;

   icon = _icon;
   setDirty();
   desktopFile()->writeEntry("Icon", icon);
}

KShortcut MenuEntryInfo::shortcut()
{
   if (!shortcutLoaded)
   {
      shortcutLoaded = true;
      if( KHotKeys::present())
      {
         shortCut = KHotKeys::getMenuEntryShortcut( service->storageId() );
      }
   }
   return shortCut;
}

static bool isEmpty(const KShortcut &shortCut)
{
   for(int i = shortCut.count(); i--;)
   {
      if (!shortCut.seq(i).isNull())
         return false;
   }
   return true;
}

static void freeShortcut(const KShortcut &shortCut)
{
   if (!isEmpty(shortCut))
   {
      QString shortcutKey = shortCut.toString();
      if (s_newShortcuts)
         s_newShortcuts->remove(shortcutKey);
      
      if (!s_freeShortcuts)
         s_freeShortcuts = new QStringList;
      
      s_freeShortcuts->append(shortcutKey);
   }
}

static void allocateShortcut(const KShortcut &shortCut)
{
   if (!isEmpty(shortCut))
   {
      QString shortcutKey = shortCut.toString();
      if (s_freeShortcuts)
          s_freeShortcuts->remove(shortcutKey);

      if (!s_newShortcuts)
         s_newShortcuts = new QStringList;

      s_newShortcuts->append(shortcutKey);
   }
}

void MenuEntryInfo::setShortcut(const KShortcut &_shortcut)
{
   if (shortCut == _shortcut)
      return;

   freeShortcut(shortCut);
   allocateShortcut(_shortcut);

   shortCut = _shortcut;
   if (isEmpty(shortCut))
      shortCut = KShortcut(); // Normalize

   shortcutLoaded = true;
   shortcutDirty = true;
}

void MenuEntryInfo::setInUse(bool inUse)
{
   if (inUse)
   {
      KShortcut temp = shortcut();
      shortCut = KShortcut();
      if (isShortcutAvailable(temp))
         shortCut = temp;
      else
         shortcutDirty = true;
      allocateShortcut(shortCut);

      if (s_deletedApps)
         s_deletedApps->remove(service->storageId());
   }
   else
   {
      freeShortcut(shortcut());
      
      // Add to list of deleted apps
      if (!s_deletedApps)
         s_deletedApps = new QStringList;

      s_deletedApps->append(service->storageId());
   }
}

bool MenuEntryInfo::isShortcutAvailable(const KShortcut &_shortcut)
{
   if (shortCut == _shortcut)
      return true;
      
   QString shortcutKey = _shortcut.toString();
   bool available = true;
   if (!s_allShortcuts)
   {
      s_allShortcuts = new QStringList(KHotKeys::allShortCuts());
   }
   available = !s_allShortcuts->contains(shortcutKey);
   if (available && s_newShortcuts)
   {
      available = !s_newShortcuts->contains(shortcutKey);
   }
   if (!available && s_freeShortcuts)
   {
      available = s_freeShortcuts->contains(shortcutKey);
   }
   return available;
}
