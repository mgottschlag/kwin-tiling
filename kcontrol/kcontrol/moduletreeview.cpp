/*
  Copyright (c) 2000 Matthias Elter <elter@kde.org>
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

#include "moduletreeview.h"
#include "moduletreeview.moc"
#include "modules.h"
#include "global.h"

ModuleTreeView::ModuleTreeView(ConfigModuleList *list, QWidget * parent, const char * name)
  : KListView(parent, name)
  , _modules(list)
{
  setFrameStyle(QFrame::WinPanel | QFrame::Sunken);   
  addColumn("");   
  setAllColumnsShowFocus(true);
  header()->hide();

  connect(this, SIGNAL(clicked(QListViewItem*)), 
		  this, SLOT(slotItemSelected(QListViewItem*)));
}

void ModuleTreeView::fill()
{
  clear();
  
  ConfigModule *module;
  for (module=_modules->first(); module != 0; module=_modules->next())
    {
      if (module->library().isEmpty())
		continue;

      if (!KCGlobal::types().contains(module->type()))
		continue;

	  if (module->onlyRoot() && !KCGlobal::root())
		continue;
      
      ModuleTreeItem *parent = 0;
      parent = getGroupItem(parent, module->groups());
      new ModuleTreeItem(parent, module);
    }
  
  //  setOpen(root, true);
  setMinimumWidth(columnWidth(0)+22);
}

void ModuleTreeView::makeSelected(ConfigModule *module)
{
  ModuleTreeItem *item = static_cast<ModuleTreeItem*>(firstChild());
 
  updateItem(item, module);
}

void ModuleTreeView::updateItem(ModuleTreeItem *item, ConfigModule *module)
{
  while (item)
    {
	  if (item->childCount() != 0)
		updateItem(static_cast<ModuleTreeItem*>(item->firstChild()), module);
	  if (item->module() == module)
		{
		  setSelected(item, true);
		  break;
		}
	  item = static_cast<ModuleTreeItem*>(item->nextSibling());
    }
}

void ModuleTreeView::expandItem(QListViewItem *item, QList<QListViewItem> *parentList)
{
  while (item)
    {
      setOpen(item, parentList->contains(item));
      
	  if (item->childCount() != 0)
		expandItem(item->firstChild(), parentList);
      item = item->nextSibling();
    }
}

void ModuleTreeView::makeVisible(ConfigModule *module)
{
  ModuleTreeItem *item;
  
  item = static_cast<ModuleTreeItem*>(firstChild());

  // collapse all
  QList<QListViewItem> parents;
  expandItem(firstChild(), &parents);

  QStringList::ConstIterator it;
  item =static_cast<ModuleTreeItem*>( firstChild());
  for (it=module->groups().begin(); it != module->groups().end(); it++)
    {
      while (item)
		{
		  if (item->tag() == *it)
			{
			  setOpen(item, true);
			  break;
			}
		  
		  item = static_cast<ModuleTreeItem*>(item->nextSibling());
		}
    }
  
  // make the item visible
  if (item)
    ensureItemVisible(item);
}

ModuleTreeItem *ModuleTreeView::getGroupItem(ModuleTreeItem *parent, const QStringList& groups)
{
  QString path;
  
  ModuleTreeItem *item = parent;
  
  QStringList::ConstIterator it;
  for (it=groups.begin(); it != groups.end(); it++)
    {
      path += *it + "/";
	  
      parent = item;
      item = static_cast<ModuleTreeItem*>(firstChild());

      while (item)
		{
		  if (static_cast<ModuleTreeItem*>(item)->tag() == *it)
			break;
		  
		  item = static_cast<ModuleTreeItem*>(item->nextSibling());
		}

      if (!item)
		{
		  // create new branch
          ModuleTreeItem *iitem;
          if (parent == 0)
            iitem = new ModuleTreeItem(this);
          else
            iitem = new ModuleTreeItem(parent);
		  iitem->setTag(*it);
		  
		  // now decorate the branch
		  KDesktopFile directory(locate("apps", "Settings/"+path+".directory"));
		  iitem->setText(0, directory.readEntry("Name", *it));
		  QPixmap icon = SmallIcon(directory.readEntry("Icon"));
		  iitem->setPixmap(0, icon);
		  
		  return iitem;
		}
    }
  return item;
}

void ModuleTreeView::slotItemSelected(QListViewItem* item)
{
  if (!item) return;

  if (static_cast<ModuleTreeItem*>(item)->module())
    {
      emit moduleSelected(static_cast<ModuleTreeItem*>(item)->module());
      return;
    }

  if (item->isOpen())
      setOpen(item, false);
  else
    {
      QList<QListViewItem> parents;
      
      QListViewItem* i = item;
      while(i)
        {
          parents.append(i);
          i = i->parent();
        }

      //int oy1 = item->itemPos();
      //int oy2 = mapFromGlobal(QCursor::pos()).y();
      //int offset = oy2 - oy1;
      
      expandItem(firstChild(), &parents);

      //int x =mapFromGlobal(QCursor::pos()).x();
      //int y = item->itemPos() + offset;
      //QCursor::setPos(mapToGlobal(QPoint(x, y)));
    }
}

void ModuleTreeView::keyPressEvent(QKeyEvent *e)
{
  if (!currentItem()) return;
  
  if(e->key() == Key_Return
     || e->key() == Key_Enter
        || e->key() == Key_Space)
    {
      //QCursor::setPos(mapToGlobal(QPoint(10, currentItem()->itemPos()+5)));
      slotItemSelected(currentItem());
    }
  else
    KListView::keyPressEvent(e);
}

ModuleTreeItem::ModuleTreeItem(QListViewItem *parent, ConfigModule *module)
  : QListViewItem(parent)
  , _module(module)
  , _tag(QString::null)
{
  if (_module)
	{
	  setText(0, module->name());
	  setPixmap(0, module->smallIcon());
	}
}

ModuleTreeItem::ModuleTreeItem(QListView *parent, ConfigModule *module)
  : QListViewItem(parent)
  , _module(module)
  , _tag(QString::null)
{
  if (_module)
	{
	  setText(0, module->name());
	  setPixmap(0, module->smallIcon());
	}
}

ModuleTreeItem::ModuleTreeItem(QListViewItem *parent, const QString& text)
  : QListViewItem(parent, text)
  , _module(0)
  , _tag(QString::null) {}

ModuleTreeItem::ModuleTreeItem(QListView *parent, const QString& text)
  : QListViewItem(parent, text)
  , _module(0)
  , _tag(QString::null) {}
