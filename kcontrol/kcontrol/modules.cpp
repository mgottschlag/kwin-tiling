/*
  Copyright (c) 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
 
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
 
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 
*/                                                                            

#include <unistd.h>
#include <sys/types.h>


#include <qwidget.h>

#include <kapp.h>
#include <kglobal.h>
#include <kservicegroup.h>
#include <kcmodule.h>
#include <krun.h>

#include "modules.h"
#include "modules.moc"
#include "utils.h"
#include "proxywidget.h"
#include "modloader.h"

template class QList<ConfigModule>;

ConfigModule::ConfigModule(QString desktopFile)
  : ModuleInfo(desktopFile)
  ,_changed(false)
  ,_module(0)
{
}

ConfigModule::~ConfigModule()
{
  delete _module;
}

ProxyWidget *ConfigModule::module()
{
  if (_module)
    return _module;

  KCModule *modWidget = ModuleLoader::loadModule(*this);

  if (modWidget)
    {
      bool run_as_root = needsRootPrivileges() && (getuid() != 0);

      _module = new ProxyWidget(modWidget, name(), "", run_as_root);
      connect(_module, SIGNAL(changed(bool)), this, SLOT(clientChanged(bool)));
      connect(_module, SIGNAL(closed()), this, SLOT(clientClosed()));
      connect(_module, SIGNAL(helpRequest()), this, SLOT(helpRequest()));
      connect(_module, SIGNAL(runAsRoot()), this, SLOT(runAsRoot()));

      return _module;
    }

  return 0;
}

void ConfigModule::deleteClient()
{
  delete _module;
  _module = 0;

  ModuleLoader::unloadModule(*this);
  _changed = false;
}

void ConfigModule::clientClosed()
{
  deleteClient();

  emit changed(this);
  emit childClosed();
}

void ConfigModule::clientChanged(bool state)
{
  setChanged(state);
  emit changed(this);
}

void ConfigModule::helpRequest()
{
  kapp->invokeHelp(docPath(), "");
}


void ConfigModule::runAsRoot()
{
  QStringList urls;
  KRun::run(*service(), urls);
}


ConfigModuleList::ConfigModuleList()
{
  setAutoDelete(true);
}

const KAboutData *ConfigModule::aboutData() const
{
  return _module->aboutData();
}

void ConfigModuleList::readDesktopEntries()
{
  readDesktopEntriesRecursive("Settings/");
}

void ConfigModuleList::readDesktopEntriesRecursive(const QString &path)
{
  KServiceGroup::Ptr group = KServiceGroup::group(path);
  
  if (!group->isValid()) return;
 
  KServiceGroup::List list = group->entries();

  for( KServiceGroup::List::ConstIterator it = list.begin();
       it != list.end(); it++)
  {
     KSycocaEntry *p = (*it);
     if (p->isType(KST_KService))
     {
        ConfigModule *module = new ConfigModule(p->entryPath());
        append(module);
     }
     else if (p->isType(KST_KServiceGroup))
     {
        readDesktopEntriesRecursive(p->entryPath());
     }
  }
}
