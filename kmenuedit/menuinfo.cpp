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
 *   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#include "menuinfo.h"

#include <qregexp.h>

#include <kstandarddirs.h>
#include <kdesktopfile.h>
#include <khotkeys.h>

//
// MenuFolderInfo
//

// Add sub menu
void MenuFolderInfo::add(MenuFolderInfo *info)
{
   subFolders.append(info);
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

// Add entry
void MenuFolderInfo::add(MenuEntryInfo *entry)
{
   entries.append(entry);
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

void MenuFolderInfo::save()
{
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
      df->writeEntry("Comment", comment);
      df->writeEntry("Icon", icon);
      df->sync();
   }

   // Save sub-menus
   for(MenuFolderInfo *subFolderInfo = subFolders.first();
       subFolderInfo; subFolderInfo = subFolders.next())
   {
      subFolderInfo->save();
   }

   // Save entries
   MenuEntryInfo *entryInfo;
   for(QPtrListIterator<MenuEntryInfo> it(entries);
       (entryInfo = it.current()); ++it)
   {
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
   }
   return false;
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

void MenuEntryInfo::save()
{
   if (!dirty) return;

   df->sync();

   if (shortcutDirty)
   {
      if( KHotKeys::present())
      {
         KHotKeys::changeMenuEntryShortcut( service->storageId(), shortCut.toStringInternal() );
      }
      shortcutDirty = false;
   }

   dirty = false;
}

void MenuEntryInfo::setCaption(const QString &_caption)
{
   if (caption == _caption)
      return;
   caption = _caption;
   setDirty();
   desktopFile()->writeEntry("Name", caption);
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

void MenuEntryInfo::setShortcut(const KShortcut &_shortcut)
{
   if (shortCut == _shortcut)
      return;

   shortCut = _shortcut;
   shortcutLoaded = true;
   shortcutDirty = true;
   dirty = true;
}
