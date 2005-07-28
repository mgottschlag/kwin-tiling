/**
 * kprivacymanager.cpp
 *
 * Copyright (c) 2003 Ralf Hoelzer <ralf@well.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published
 *  by the Free Software Foundation; either version 2.1 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "kprivacymanager.h"
#include <kapplication.h>
#include <dcopclient.h>
#include <kconfig.h>
#include <ksimpleconfig.h>
#include <kprocess.h>
#include <kglobal.h>
#include <kapplication.h>
#include <kdebug.h>
#include <krecentdocument.h>
#include <kstandarddirs.h>

#include <qstringlist.h>
#include <qfile.h>
#include <qdir.h>

KPrivacyManager::KPrivacyManager()
{
  if (!kapp->dcopClient()->isAttached())
    kapp->dcopClient()->attach();

  m_error = false;
}


KPrivacyManager::~KPrivacyManager()
{
}

bool KPrivacyManager::clearThumbnails()
{
  // http://freedesktop.org/Standards/Home
  // http://triq.net/~jens/thumbnail-spec/index.html

  QDir thumbnailDir( QDir::homeDirPath() + "/.thumbnails/normal");
  thumbnailDir.setFilter( QDir::Files );
  QStringList entries = thumbnailDir.entryList();
  for( QStringList::Iterator it = entries.begin() ; it != entries.end() ; ++it)
    if(!thumbnailDir.remove(*it)) m_error = true;
  if(m_error) return m_error;

  thumbnailDir.setPath(QDir::homeDirPath() + "/.thumbnails/large");
  entries = thumbnailDir.entryList();
  for( QStringList::Iterator it = entries.begin() ; it != entries.end() ; ++it)
    if(!thumbnailDir.remove(*it)) m_error = true;
  if(m_error) return m_error;

  thumbnailDir.setPath(QDir::homeDirPath() + "/.thumbnails/fail");
  entries = thumbnailDir.entryList();
  for( QStringList::Iterator it = entries.begin() ; it != entries.end() ; ++it)
    if(!thumbnailDir.remove(*it)) m_error = true;
  
  return m_error;
}

bool KPrivacyManager::clearRunCommandHistory() const
{
  return kapp->dcopClient()->send( "kdesktop", "KDesktopIface", "clearCommandHistory()", QString("") );
}

bool KPrivacyManager::clearAllCookies() const
{
  return kapp->dcopClient()->send( "kded", "kcookiejar", "deleteAllCookies()", QString("") );
}

bool KPrivacyManager::clearSavedClipboardContents()
{
  if(!isApplicationRegistered("klipper"))
  {
    KConfig *c = new KConfig("klipperrc", false, false);

    {
      KConfigGroupSaver saver(c, "General");
      c->deleteEntry("ClipboardData");
      c->sync();
    }
    delete c;
    return true;
  }

  return kapp->dcopClient()->send( "klipper", "klipper", "clearClipboardHistory()", QString ("") );
}

bool KPrivacyManager::clearFormCompletion() const
{
  QFile completionFile(locateLocal("data", "khtml/formcompletions"));

  return completionFile.remove();
}

bool KPrivacyManager::clearWebCache() const
{
    KProcess process;
    process << "kio_http_cache_cleaner" << "--clear-all";
    return process.start(KProcess::DontCare);
}

bool KPrivacyManager::clearRecentDocuments() const
{
  KRecentDocument::clear();
  return KRecentDocument::recentDocuments().isEmpty();
}

bool KPrivacyManager::clearQuickStartMenu() const
{
  return kapp->dcopClient()->send( "kicker", "kicker", "clearQuickStartMenu()", QString ("") );
}

bool KPrivacyManager::clearWebHistory()
{
  QStringList args("--preload");

  // preload Konqueror if it is not running
  if(!isApplicationRegistered("konqueror"))
  {
    kdDebug() << "couldn't find Konqueror instance, preloading." << endl;
    kapp->kdeinitExec("konqueror", args, 0,0);
  }

  return kapp->dcopClient()->send( "konqueror*", "KonqHistoryManager",
                                   "notifyClear(QCString)", QString ("") );
}

bool KPrivacyManager::clearFavIcons()
{
  QDir favIconDir(KGlobal::dirs()->saveLocation( "cache", "favicons/" ));
  favIconDir.setFilter( QDir::Files );
  
  QStringList entries = favIconDir.entryList();

  // erase all files in favicon directory
  for( QStringList::Iterator it = entries.begin() ; it != entries.end() ; ++it)
    if(!favIconDir.remove(*it)) m_error = true;
  return m_error;
}


bool KPrivacyManager::isApplicationRegistered(const QString &appName)
{
  DCOPCStringList regApps = kapp->dcopClient()->registeredApplications();

  for ( DCOPCStringList::Iterator it = regApps.begin(); it != regApps.end(); ++it )
    if((*it).find(appName.latin1()) != -1) return true;

  return false;
}

#include "kprivacymanager.moc"
