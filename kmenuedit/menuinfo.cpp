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

#include "menuinfo.h"
#include "menufile.h"

#include <QRegExp>

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
   QString cap = (r.indexIn(caption) > -1) ? r.cap(1) : caption;

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
   return QString(); // Never reached
}

// Return a unique item caption inspired by @p caption
QString MenuFolderInfo::uniqueItemCaption(const QString &caption, const QString &exclude)
{
   QRegExp r("(.*)(?=-\\d+)");
   QString cap = (r.indexIn(caption) > -1) ? r.cap(1) : caption;

   QString result = caption;

   for(int n = 1; ++n; )
   {
      bool ok = true;
      if (result == exclude)
         ok = false;
      MenuEntryInfo *entryInfo;
      for(Q3PtrListIterator<MenuEntryInfo> it(entries);
          ok && (entryInfo = it.current()); ++it)
      {
         if (entryInfo->caption == result)
            ok = false;
      }
      if (ok)
         return result;

      result = cap + QString("-%1").arg(n);
   }
   return QString(); // Never reached
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

      KDesktopFile *df = 0;
      if (directoryFile != local)
      {
         KDesktopFile orig("apps", directoryFile);
         df = orig.copyTo(local);
      }
      else
      {
         df = new KDesktopFile("apps", directoryFile);
      }

      KConfigGroup dg( df->desktopGroup() );
      dg.writeEntry("Name", caption);
      dg.writeEntry("GenericName", genericname);
      dg.writeEntry("Comment", comment);
      dg.writeEntry("Icon", icon);
      dg.sync();
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
   for(Q3PtrListIterator<MenuEntryInfo> it(entries);
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
   for(Q3PtrListIterator<MenuEntryInfo> it(entries);
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
   for(Q3PtrListIterator<MenuEntryInfo> it(entries);
       (entryInfo = it.current()); ++it)
   {
      if (entryInfo->shortCut == cut)
         return entryInfo->service;
   }
   return KService::Ptr();
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
   for(Q3PtrListIterator<MenuEntryInfo> it(entries);
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
   m_desktopFile->rollback(false);
   delete m_desktopFile;
}

KDesktopFile *MenuEntryInfo::desktopFile()
{
   if (!m_desktopFile)
   {
      m_desktopFile = new KDesktopFile(service->desktopEntryPath());
   }
   return m_desktopFile;
}

void MenuEntryInfo::setDirty()
{
   if (dirty) return;

   dirty = true;

   QString local = KStandardDirs::locateLocal("xdgdata-apps", service->menuId());
   if (local != service->desktopEntryPath())
   {
      KDesktopFile *oldDf = desktopFile();
      m_desktopFile = oldDf->copyTo(local);
      delete oldDf;
   }
}

bool MenuEntryInfo::needInsertion()
{
   // If entry is dirty and previously stored under applnk, then we need to be added explicitly
   return dirty && !service->desktopEntryPath().startsWith("/");
}

void MenuEntryInfo::save()
{
   if (dirty)
   {
      m_desktopFile->sync();
      dirty = false;
   }

   if (shortcutDirty)
   {
      if( KHotKeys::present())
      {
         KHotKeys::changeMenuEntryShortcut( service->storageId(), shortCut.toString() );
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
   desktopFile()->desktopGroup().writeEntry("Name", caption);
}

void MenuEntryInfo::setDescription(const QString &_description)
{
    if (description == _description)
        return;
    description = _description;
    setDirty();
    desktopFile()->desktopGroup().writeEntry("GenericName", description);
}

void MenuEntryInfo::setIcon(const QString &_icon)
{
   if (icon == _icon)
      return;

   icon = _icon;
   setDirty();
   desktopFile()->desktopGroup().writeEntry("Icon", icon);
}

KShortcut MenuEntryInfo::shortcut()
{
   if (!shortcutLoaded)
   {
      shortcutLoaded = true;
      if( KHotKeys::present())
      {
         shortCut = KShortcut(KHotKeys::getMenuEntryShortcut( service->storageId() ));
      }
   }
   return shortCut;
}

static void freeShortcut(const KShortcut &shortCut)
{
   if (!shortCut.isEmpty())
   {
      QString shortcutKey = shortCut.toString();
      if (s_newShortcuts)
         s_newShortcuts->removeAll(shortcutKey);

      if (!s_freeShortcuts)
         s_freeShortcuts = new QStringList;

      s_freeShortcuts->append(shortcutKey);
   }
}

static void allocateShortcut(const KShortcut &shortCut)
{
   if (!shortCut.isEmpty())
   {
      QString shortcutKey = shortCut.toString();
      if (s_freeShortcuts)
          s_freeShortcuts->removeAll(shortcutKey);

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
   if (shortCut.isEmpty())
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
         s_deletedApps->removeAll(service->storageId());
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
