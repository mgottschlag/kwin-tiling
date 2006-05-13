this file is currently not used.
this message breaks compilation.
that is intentional :-]

/*
  Copyright (c) 2000 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>

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
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/


#include <q3header.h>
#include <QString>
#include <q3ptrlist.h>
#include <QPoint>
#include <QCursor>

#include <klocale.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <kservicegroup.h>
#include <kdebug.h>

#include "modulemenu.h"
#include "modulemenu.moc"
#include "modules.h"
#include "global.h"


ModuleMenu::ModuleMenu(ConfigModuleList *list, QWidget * parent, const char * name)
  : KMenu(parent, name)
  , _modules(list)
{
  // use large id's to start with...
  id = 10000;

  fill(this, KCGlobal::baseGroup());

  connect(this, SIGNAL(activated(int)), this, SLOT(moduleSelected(int)));
}

void ModuleMenu::fill(KMenu *parentMenu, const QString &parentPath)
{
  QStringList subMenus = _modules->submenus(parentPath);
  for(QStringList::ConstIterator it = subMenus.begin();
      it != subMenus.end(); ++it)
  {
     QString path = *it;
     KServiceGroup::Ptr group = KServiceGroup::group(path);
     if (!group)
        continue;
     
     // create new menu
     KMenu *menu = new KMenu(parentMenu);
     connect(menu, SIGNAL(activated(int)), this, SLOT(moduleSelected(int)));

     // Item names may contain ampersands. To avoid them being converted to 
     // accelators, replace them with two ampersands.
     QString name = group->caption();
     name.replace("&", "&&");
  
     parentMenu->insertItem(KGlobal::iconLoader()->loadIcon(group->icon(), K3Icon::Desktop, K3Icon::SizeSmall)
                        , name, menu);

     fill(menu, path);
  }

  ConfigModule *module;
  Q3PtrList<ConfigModule> moduleList = _modules->modules(parentPath);
  for (module=moduleList.first(); module != 0; module=moduleList.next())
  {
     // Item names may contain ampersands. To avoid them being converted to 
     // accelators, replace them with two ampersands.
     QString name = module->moduleName();
     name.replace("&", "&&");

     int realid = parentMenu->insertItem(KGlobal::iconLoader()->loadIcon(module->icon(), K3Icon::Desktop, K3Icon::SizeSmall)
                                     , name, id);
     _moduleDict.insert(realid, module);

      id++;
  }
  
}

void ModuleMenu::moduleSelected(int id)
{
  kDebug(1208) << "Item " << id << " selected" << endl;
  ConfigModule *module = _moduleDict[id];
  if (module)
    emit moduleActivated(module);
}
