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


#include <qevent.h>


#include <kglobal.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <kstddirs.h>                                                                
#include <kdesktopfile.h>
#include <kiconloader.h>

#include "moduleiconview.h"
#include "modules.h"
#include "global.h"


ModuleIconView::ModuleIconView(ConfigModuleList *list, QWidget * parent, const char * name)
  : QIconView(parent, name)
  , _path(QString::null)
  , _modules(list)
{
  setArrangement(LeftToRight);
  setSelectionMode(Single);
  setItemsMovable(false);
  setSorting(false);
  setWordWrapIconText(true);
  setItemTextPos(Right);
  setResizeMode(Adjust);

  connect(this, SIGNAL(clicked(QIconViewItem*)), 
		  this, SLOT(slotItemSelected(QIconViewItem*)));
}
  
void ModuleIconView::makeSelected(ConfigModule *m)
{
  if (!m) return;

  for (QIconViewItem *i = firstItem(); i; i = i->nextItem())
	{
      if(static_cast<ModuleIconItem*>(i)->module() == m)
		{
		  setSelected(i, true);
		  break;
		}
	}

}

void ModuleIconView::makeVisible(ConfigModule *m)
{
  if (!m) return;
  _path = m->groups().join(QString::null);
  fill();
}

void ModuleIconView::fill()
{
  clear();

  QStringList subdirs;
  
  // build a list of subdirs
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

	  QString path = module->groups().join(QString::null);

	  if(!subdirs.contains(path))
		subdirs.append(path);
	}
  subdirs.sort();

  qDebug("path = %s", _path.latin1());

  if (_path == QString::null)
	{
	  for (QStringList::Iterator it = subdirs.begin(); it != subdirs.end(); ++it )
		{
		  QString subdir = (*it);
		  
 		  KDesktopFile directory(locate("apps", "Settings/"
										+ subdir
										+ "/.directory"));



		  QPixmap icon = KGlobal::iconLoader()->loadIcon(directory.readEntry("Icon"),
														 KIconLoader::Medium);

		  if(icon.isNull())
		  	icon = KGlobal::iconLoader()->loadIcon("folder", KIconLoader::Medium);

		  ModuleIconItem *i = new ModuleIconItem(this, directory.readEntry("Name", subdir), icon);
		  i->setTag(subdir);
		}
	}
  else
	{
	  QPixmap icon = KGlobal::iconLoader()->loadIcon("up", KIconLoader::Medium);
	  
	  if(icon.isNull())
		icon = KGlobal::iconLoader()->loadIcon("folder", KIconLoader::Medium);

	  // go-up node
	  ModuleIconItem *i = new ModuleIconItem(this, i18n("Go up"), icon);
	  i->setTag(QString::null);

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
		  
		  QString path = module->groups().join(QString::null);
		  if(path == _path)
            (void) new ModuleIconItem(this, module->name(), module->largeIcon(), module);
		}
    }

}

void ModuleIconView::slotItemSelected(QIconViewItem* item)
{
  if (!item) return;

  if (static_cast<ModuleIconItem*>(item)->module())
	emit moduleSelected(static_cast<ModuleIconItem*>(item)->module());
  else
	{
	  _path = static_cast<ModuleIconItem*>(item)->tag();
	  fill();
	}
}


QDragObject *ModuleIconView::dragObject()
{
  QDragObject *icondrag = QIconView::dragObject();
  QUriDrag *drag = new QUriDrag(this);

  QPixmap pm = icondrag->pixmap();
  drag->setPixmap(pm, QPoint(pm.width()/2,pm.height()/2));


  QPoint orig = viewportToContents(viewport()->mapFromGlobal(QCursor::pos()));
  
  QStringList l;	
  ModuleIconItem *item = (ModuleIconItem*) findItem(orig);
  if (item)
    {
      if (item->module())
	l.append(item->module()->fileName());
      else
	if (!item->tag().isEmpty())
	  {
	    QString dir = _path + "/" + item->tag();
	    dir = locate("apps", "Settings/"+dir+"/.directory");
	    int pos = dir.findRev("/.directory");
	    if (pos > 0)
	      {
		dir = dir.left(pos);
		l.append(dir);
	      }
	  }
      
      drag->setFilenames(l); 
    }

  delete icondrag;

  if (l.count() == 0)
    return 0;

  return drag;
}


