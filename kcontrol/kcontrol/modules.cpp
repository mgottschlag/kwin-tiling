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

#include <qwidget.h>

#include <kapp.h>
#include <kstddirs.h>
#include <kglobal.h>
#include <kcmodule.h>

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

  KCModule *modWidget = ModuleLoader::module(*this);

  if (modWidget)
    {
      _module = new ProxyWidget(modWidget, name());
      connect(_module, SIGNAL(changed(bool)), this, SLOT(clientChanged(bool)));
      connect(_module, SIGNAL(closed()), this, SLOT(clientClosed()));
      connect(_module, SIGNAL(helpRequest()), this, SLOT(helpRequest()));
      return _module;
    }

  return 0;
}

void ConfigModule::deleteClient()
{
  clientClosed();
}

void ConfigModule::clientClosed()
{
  delete _module;
  _module = 0;

  _changed = false;
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
  kapp->invokeHTMLHelp(docPath(), "");
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
  QStringList files;
 
  files = KGlobal::dirs()->findAllResources("apps", "Settings/*.desktop", TRUE);

  QStringList::Iterator it;
  for (it = files.begin(); it != files.end(); ++it)
    {
      ConfigModule *module = new ConfigModule(*it);
      append(module);
    }
}
