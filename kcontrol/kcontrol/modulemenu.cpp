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


#define INCLUDE_MENUITEM_DEF 1
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
      if (KCGlobal::system()) {
	if (!module->onlyRoot())
	  continue;
      }
      else {
	if (module->onlyRoot() && !KCGlobal::root())
	  continue;
      }
      
      QPopupMenu *parent = 0;
      parent = getGroupMenu(module->groups());
      int realid = parent->insertItem(module->smallIcon(), module->name(), id);
      _moduleDict.insert(realid, module);

      id++;
    }  
  
  connect(this, SIGNAL(activated(int)), this, SLOT(moduleSelected(int)));
}


QPopupMenu *ModuleMenu::getGroupMenu(const QStringList &groups)
{
  QString path;  
  QPopupMenu *parent, *iitem, *item=this;

  QStringList::ConstIterator it;
  for (it=groups.begin(); it != groups.end(); it++)
    {      
      path += *it + "/";
      parent = item;
      item = 0;

      // look if there is already a menu
      for (unsigned int i=0; i<parent->count(); i++)
	{
	  iitem = 0;
	  QMenuItem *mitem = parent->findItem(idAt(i));
	  if (mitem)
	    iitem = mitem->popup();
	  if (!iitem)
	    continue;

	  if (iitem->isA("ModuleMenuItem") && static_cast<ModuleMenuItem*>(iitem)->tag() == *it)
	    {
	      item = iitem;
	      break;
	    }
	}

      if (!item)
	{
	  // create new branch
          ModuleMenuItem *menu = new ModuleMenuItem(parent);
	  connect(menu, SIGNAL(activated(int)), this, SLOT(moduleSelected(int)));
	  menu->setTag(*it);

	  KDesktopFile directory(locate("apps", "Settings/"+path+".directory"));

	  static_cast<QPopupMenu*>(parent)->insertItem(SmallIcon(directory.readEntry("Icon")), directory.readEntry("Name", *it), menu);
	  item = menu;
	}
    }

  // just in case...
  if (!item)
    item = this;

  return item;
}
  

void ModuleMenu::moduleSelected(int id)
{
  kdDebug() << "Item " << id << " selected" << endl;
  ConfigModule *module = _moduleDict[id];
  if (module)
    emit moduleActivated(module);
}
