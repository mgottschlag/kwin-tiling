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
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/                                                                            


#include <qpopupmenu.h>

#include <qheader.h>
#include <qstring.h>
#include <qlist.h>
#include <qpoint.h>
#include <qcursor.h>

#include <klocale.h>
#include <kglobal.h>
#include <kstddirs.h>
#include <kiconloader.h>
#include <kdesktopfile.h>
#include <kdebug.h>

#include "modulemenu.h"
#include "modulemenu.moc"
#include "modules.h"
#include "global.h"


ModuleMenu::ModuleMenu(ConfigModuleList *list, QWidget * parent, const char * name)
  : QPopupMenu(parent, name)
  , _modules(list)
{
  // use large id's to start with...
  id = 10000;

  ConfigModule *module;
  for (module=_modules->first(); module != 0; module=_modules->next())
    {
      if (module->library().isEmpty())
		continue;

      if (!KCGlobal::types().contains(module->type()))
		continue;

	  if (module->onlyRoot() && !KCGlobal::root())
		continue;
         
      QPopupMenu *parent = 0;
      parent = getGroupMenu(module->groups());
      int realid = parent->insertItem(module->smallIcon(), module->name(), id);
      _moduleDict.insert(realid, module);

      id++;
    }  
  
  connect(this, SIGNAL(activated(int)), this, SLOT(moduleSelected(int)));
}


QString menuPath(const QStringList& groups)
{
  QString path;  

  QStringList::ConstIterator it;
  for (it=groups.begin(); it != groups.end(); ++it)
    path += *it + "/";
  
  return path;
}


QPopupMenu *ModuleMenu::getGroupMenu(const QStringList &groups)
{
  // break recursion if path is empty
  if (groups.count() == 0)
    return this;

  // calculate path
  QString path = menuPath(groups);
  kdDebug() << "Path " << path << endl;

  // look if menu already exists
  if (_menuDict[path])
    return _menuDict[path];
  
  // find parent menu
  QStringList parGroup;
  for (unsigned int i=0; i<groups.count()-1; i++)
    parGroup.append(groups[i]);
  QPopupMenu *parent = getGroupMenu(parGroup);

  // create new menu
  QPopupMenu *menu = new QPopupMenu(parent);
  connect(menu, SIGNAL(activated(int)), this, SLOT(moduleSelected(int)));
  KDesktopFile directory(locate("apps", "Settings/"+path+".directory"));
  QString defName = path.left(path.length()-1);
  int pos = defName.findRev('/');
  if (pos >= 0)
    defName = defName.mid(pos+1);
  parent->insertItem(SmallIcon(directory.readEntry("Icon")), directory.readEntry("Name", defName), menu);

  _menuDict.insert(path, menu);

  return menu;
}
  

void ModuleMenu::moduleSelected(int id)
{
  kdDebug() << "Item " << id << " selected" << endl;
  ConfigModule *module = _moduleDict[id];
  if (module)
    emit moduleActivated(module);
}
