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
#include <sys/utsname.h>
#include <stdlib.h>

#include <qheader.h>
#include <qcolor.h>

#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kdesktopfile.h>
#include <kstddirs.h>


#include "index.h"
#include "index.moc"
#include "utils.h"


IndexPane::IndexPane(QWidget *parent, const char *name)
  : QWidget(parent, name)
{
  _tree = new QListView(this);   
  _tree->setFrameStyle(QFrame::WinPanel | QFrame::Sunken);   
  _tree->addColumn("",0);   
  _tree->addColumn("",22);
  _tree->setColumnAlignment(1, AlignRight);
  _tree->setAllColumnsShowFocus(true);
  _tree->header()->hide();

  connect(_tree, SIGNAL(currentChanged(QListViewItem*)), 
	  this, SLOT(currentChanged(QListViewItem*)));
}

void IndexPane::resizeEvent(QResizeEvent *)
{
  if (_tree)
    {
      _tree->setGeometry(0,0,width(),height());
      if (_tree->columnWidth(0) != _tree->visibleWidth() - _tree->columnWidth(1))
	{
	  _tree->setColumnWidth(0, _tree->visibleWidth() - _tree->columnWidth(1));
	  _tree->triggerUpdate();
	}
    }
}


/* ICON FILENAMES */
#define ICON_LOCALUSER    "kcontrol_user"
#define ICON_LOCALMACHINE "kcontrol_system"

void IndexPane::fillIndex(ConfigModuleList &list)
{
  QString username;
  char *user = getlogin();
  if (!user) user = getenv("LOGNAME");
  if (!user) username = ""; else username = QString("(%1)").arg(user);
  // add the top level nodes
  localUser = new QListViewItem(_tree, i18n("Local User %1").arg(username));
  localUser->setPixmap(0, KGlobal::iconLoader()->loadIcon(
        	ICON_LOCALUSER, KIconLoader::Small));
  localMachine = new QListViewItem(_tree, i18n("Local Computer"));
  localMachine->setPixmap(0, KGlobal::iconLoader()->loadIcon(
        	ICON_LOCALMACHINE, KIconLoader::Small));

  ConfigModule *module;
  for (module=list.first(); module != 0; module=list.next())
    {
      QListViewItem *parent;
      if (module->localUser())
	parent = localUser;
      else
	parent = localMachine;
      parent = getGroupItem(parent, module->groups());
      new IndexListItem(parent, module);
    }

  // _tree->setOpen(localMachine, true);
  _tree->setOpen(localUser, true);
}


QListViewItem *IndexPane::getGroupItem(QListViewItem *parent, const QStringList& groups)
{
  QString path;

  QListViewItem *item = parent;

  QStringList::ConstIterator it;
  for (it=groups.begin(); it != groups.end(); it++)
    {
      path += *it + "/";

      parent = item;
      item = item->firstChild();
      while (item)
	{
	  if (((IndexItem*)item)->tag() == *it)
	    break;

	  item = item->nextSibling();
	}
      if (!item)
	{
	  // create new branch
	  IndexItem *iitem = new IndexItem(parent);
	  iitem->setTag(*it);

	  // now decorate the branch
	  KDesktopFile directory(locate("apps", "Settings/"+path+".directory"));
	  
	  iitem->setText(0, directory.readEntry("Name", *it));
	  iitem->setPixmap(0, KGlobal::iconLoader()->loadIcon(
            directory.readEntry("Icon"), KIconLoader::Small));

	  return iitem;
	}
    }

  return item;
}


void IndexPane::currentChanged(QListViewItem *item)
{
  if (item == 0)
    return;

  if (item->childCount() != 0)
    return;
  IndexListItem *iitem = (IndexListItem*)(item);
  if (iitem && iitem->_module)
    {
      emit moduleActivated(iitem->_module);
      moduleChanged(iitem->_module);
    }
}


void IndexPane::updateItem(QListViewItem *item, ConfigModule *module)
{
  while (item)
    {
      if (item->childCount() != 0)
	updateItem(item->firstChild(), module);
	  
      IndexListItem *iitem = (IndexListItem*)(item);
      if (iitem->getModule() == module)
	iitem->repaint();

      item = item->nextSibling();
    }
}


void IndexPane::moduleChanged(ConfigModule *module)
{
  updateItem(_tree->firstChild(), module);
}


void IndexPane::makeVisible(ConfigModule *module)
{
  QListViewItem *item=0;

  // search the right tree
  if (module->localUser())
    item = localUser;
  else
    item = localMachine;
  _tree->setOpen(item, true);

  QStringList::ConstIterator it;
  for (it=module->groups().begin(); it != module->groups().end(); it++)
    {
      item = item->firstChild();
      while (item)
	{
	  if (((IndexItem*)item)->tag() == *it)
	    {
	      _tree->setOpen(item, true);
	      break;
	    }
	  
	  item = item->nextSibling();
	}
    }

  // make the item visible
  if (item)
    _tree->ensureItemVisible(item);
}


IndexListItem::IndexListItem(QListViewItem *parent, ConfigModule *module)
  : IndexItem(parent), _module(module)
{
  if (!module)
    return;

  setText(0, module->name());
  setPixmap(0, module->icon());

  if (module->onlyRoot())
    setPixmap(1, BarIcon("lock"));

  QObject::connect(module, SIGNAL(changed(ConfigModule*)), 
		   listView()->parent(), SLOT(moduleChanged(ConfigModule*)));
}


IndexListItem::IndexListItem(QListView *parent, ConfigModule *module)
  : IndexItem(parent), _module(module)
{
  if (!module)
    return;

  setText(0, module->name());
  setPixmap(0, module->icon());

  if (module->onlyRoot())
    setPixmap(1, BarIcon("lock"));

  QObject::connect(module, SIGNAL(changed(ConfigModule*)), 
		   listView()->parent(), SLOT(moduleChanged(ConfigModule*)));
}


void IndexListItem::paintCell(QPainter *p, const QColorGroup &cg, int column, int width, int align)
{
  QColorGroup colorgroup(cg);

  if (_module->isActive())
    {
      colorgroup.setColor(QColorGroup::Text, Qt::darkGreen);
      colorgroup.setColor(QColorGroup::HighlightedText, Qt::darkGreen);
    }
  if (_module->isChanged())
    {
      colorgroup.setColor(QColorGroup::Text, Qt::red);
      colorgroup.setColor(QColorGroup::HighlightedText, Qt::red);
    }

  QListViewItem::paintCell(p, colorgroup, column, width, align);
}


