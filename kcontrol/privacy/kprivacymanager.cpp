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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "kprivacymanager.h"
#include <kapplication.h>
#include <dcopclient.h>
#include <kconfig.h>
#include <kprocess.h>
#include <kglobal.h>
#include <kapplication.h>
#include <kdebug.h>
#include <qstringlist.h>

KPrivacyManager::KPrivacyManager()
{
  if (!kapp->dcopClient()->isAttached())
    kapp->dcopClient()->attach();
}


KPrivacyManager::~KPrivacyManager()
{
}

bool KPrivacyManager::clearRunCommandHistory()
{
  KConfig *c = new KConfig("kdesktoprc", false, false);

  {
    KConfigGroupSaver saver(c, "MiniCli");
    c->deleteEntry("CompletionItems");
    c->deleteEntry("History");
    c->sync();
  }

  QByteArray dummyData;

  return kapp->dcopClient()->send( "kdesktop", "KDesktopIface", "reloadConfiguration()", dummyData );
}

bool KPrivacyManager::clearAllCookies()
{

  QByteArray dummyData;

  return kapp->dcopClient()->send( "kded", "kcookiejar", "deleteAllCookies()", dummyData );
}

bool KPrivacyManager::clearSavedClipboardContents()
{
  // TODO
  return true;
}

bool KPrivacyManager::clearStoredPasswords()
{
  // TODO
  return true;
}

bool KPrivacyManager::clearWebCache()
{
    KProcess process;
    process << "kio_http_cache_cleaner" << "--clear-all";
    return process.start(KProcess::DontCare);
}

bool KPrivacyManager::clearWebHistory()
{
  QByteArray dummyData;
  QStringList args("--preload");

  // preload Konqueror if it is not running
  if(!isApplicationRegistered("konqueror"))
  {
    kdDebug() << "couldn't find Konqueror instance, preloading." << endl;
    kapp->kdeinitExec("konqueror", args, 0,0);
  }

  return kapp->dcopClient()->send( "konqueror*", "KonqHistoryManager",
                                   "notifyClear(QCString)",dummyData );
}

bool KPrivacyManager::isApplicationRegistered(QString appName)
{

  QCStringList regApps = kapp->dcopClient()->registeredApplications();

  for ( QCStringList::Iterator it = regApps.begin(); it != regApps.end(); ++it )
  {
    if((*it).find(appName.latin1()) != -1) return true;
  }

  return false;
}
