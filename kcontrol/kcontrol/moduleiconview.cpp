/*
  Copyright (c) 2000 Matthias Elter <elter@kde.org>
 
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

#include "moduleiconview.h"
#include "modules.h"
#include "global.h"

ModuleIconView::ModuleIconView(ConfigModuleList *list, QWidget * parent, const char * name)
  : QIconView(parent, name)
  , _modules(list)
{
  setArrangement(LeftToRight);
  setSelectionMode(Single);
  setItemsMovable(false);
  setSorting(false);
  setWordWrapIconText(true);
  setItemTextPos(Right);
  setResizeMode(Adjust);

  connect(this, SIGNAL(selectionChanged(QIconViewItem*)), 
		  this, SLOT(slotItemSelected(QIconViewItem*)));
}
  
void ModuleIconView::makeSelected(ConfigModule*)
{
}

void ModuleIconView::makeVisible(ConfigModule *)
{
}

void ModuleIconView::fill()
{
  clear();

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
	  
	  (void) new ModuleIconItem(this, module->comment(), module->largeIcon(), module);
    }
}

void ModuleIconView::slotItemSelected(QIconViewItem* item)
{
  if (static_cast<ModuleIconItem*>(item)->module())
	emit moduleSelected(static_cast<ModuleIconItem*>(item)->module());
}
