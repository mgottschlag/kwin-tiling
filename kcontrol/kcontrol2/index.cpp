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



#include <qheader.h>
#include <qcolor.h>


#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>


#include "index.h"
#include "index.moc"
#include "utils.h"


IndexPane::IndexPane(QWidget *parent, const char *name)
  : QWidget(parent, name)
{
  _tree = new QListView(this);   
  _tree->setFrameStyle(QFrame::WinPanel | QFrame::Sunken);   
  _tree->addColumn("");   
  _tree->addColumn("");
  _tree->setColumnAlignment(1, AlignRight);
  _tree->setAllColumnsShowFocus(true);
  _tree->setColumnWidth(1, 22);
  _tree->header()->hide();

  connect(_tree, SIGNAL(doubleClicked(QListViewItem*)), 
	  this, SLOT(doubleClicked(QListViewItem*)));
}


void IndexPane::resizeEvent(QResizeEvent *)
{
  if (_tree)
    {
      _tree->setGeometry(0,0,width(),height());
      if (_tree->columnWidth(0) != width() - _tree->columnWidth(1))
	{
	  _tree->setColumnWidth(0, width() - _tree->columnWidth(1));
	  _tree->triggerUpdate();
	}
    }
}


void IndexPane::fillIndex(ConfigModuleList &list)
{
  // add the top level nodes
  localUser = new QListViewItem(_tree, i18n("Local User"));
  localMachine = new QListViewItem(_tree, i18n("Local Computer"));

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

  _tree->setOpen(localUser, true);
}


QListViewItem *IndexPane::getGroupItem(QListViewItem *parent, const QStringList& groups)
{
  QListViewItem *item = parent;

  QStringList::ConstIterator it;
  for (it=groups.begin(); it != groups.end(); it++)
    {
      parent = item;
      item = item->firstChild();
      while (item)
	{
	  if (item->text(0) == *it)
	    break;

	  item = item->nextSibling();
	}
      if (!item)
	item = new QListViewItem(parent, *it);
    }

  return item;
}


void IndexPane::doubleClicked(QListViewItem *item)
{
  IndexListItem *iitem = dynamic_cast<IndexListItem*>(item);
  if (iitem && iitem->_module)
    {
      emit moduleDoubleClicked(iitem->_module);
      moduleChanged(iitem->_module);
    }
}


void IndexPane::updateItem(QListViewItem *item, ConfigModule *module)
{
  while (item)
    {
      updateItem(item->firstChild(), module);

      IndexListItem *iitem = dynamic_cast<IndexListItem*>(item);
      if ((iitem && iitem->getModule()) && (iitem->getModule() == module))
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
	  if (item->text(0) == *it)
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
  : QListViewItem(parent), _module(module)
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
  : QListViewItem(parent), _module(module)
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
      colorgroup.setColor(QColorGroup::Text, Qt::green);
      colorgroup.setColor(QColorGroup::HighlightedText, Qt::green);
    }
  if (_module->isChanged())
    {
      colorgroup.setColor(QColorGroup::Text, Qt::red);
      colorgroup.setColor(QColorGroup::HighlightedText, Qt::red);
    }

  QListViewItem::paintCell(p, colorgroup, column, width, align);
}


