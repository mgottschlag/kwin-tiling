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

#include <qheader.h>
#include <qlayout.h>
#include <qpushbutton.h>

#include <kglobal.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kdesktopfile.h>
#include <kstddirs.h>

#include "indexwidget.h"
#include "indexwidget.moc"
#include "modules.h"
#include "global.h"

IndexWidget::IndexWidget(QWidget *parent , const char *name)
  : QWidget(parent, name)
{
  _tree = new QListView(this);
  _tree->setFrameStyle(QFrame::WinPanel | QFrame::Sunken);   
  _tree->addColumn("");   
  _tree->setAllColumnsShowFocus(true);
  _tree->header()->hide();

  connect(_tree, SIGNAL(currentChanged(QListViewItem*)), 
	  this, SLOT(currentChanged(QListViewItem*)));

  _treebtn = new QPushButton(i18n("Tree view"), this);
  _treebtn->setFixedHeight(22);
  connect(_treebtn, SIGNAL(clicked()), this, SLOT(treeButtonClicked()));

  _iconbtn = new QPushButton(i18n("Icon view"), this);
  _iconbtn->setFixedHeight(22);
  connect(_iconbtn, SIGNAL(clicked()), this, SLOT(iconButtonClicked()));

  // activate treeview
  treeButtonClicked();
}

void IndexWidget::resizeEvent(QResizeEvent *)
{
  _treebtn->move(0, height()-22);
  _treebtn->resize(width()/2, 22);
  _iconbtn->move(_treebtn->width(), height()-22);
  _iconbtn->resize(width()/2, 22);

  _tree->move(0,0);
  _tree->resize(width(), height()-22);
}

void IndexWidget::fillIndex(ConfigModuleList *list)
{
  QString rootlabel;

  if (KCGlobal::system())
	rootlabel = i18n("Settings for system %1").arg(KCGlobal::hostName());
  else
	rootlabel = i18n("Settings for user %1").arg(KCGlobal::userName());

  // add the top level nodes
  _root = new QListViewItem(_tree, rootlabel);
  _root->setPixmap(0, KGlobal::iconLoader()->loadIcon("kcontrol", KIconLoader::Small));
  
  ConfigModule *module;
  for (module=list->first(); module != 0; module=list->next())
    {
	  if (module->library().isEmpty())
		continue;

	  if (KCGlobal::system())
		{
		  if (!module->onlyRoot())
			continue;
		}
	  else
		{
		  if (module->onlyRoot() && !KCGlobal::root())
			continue;
		}

      QListViewItem *parent;
	  parent = _root;
      parent = getGroupItem(parent, module->groups());
      new IndexListItem(parent, module);
    }

  _tree->setOpen(_root, true);
  setMinimumWidth(_tree->columnWidth(0)+22);
}

QListViewItem *IndexWidget::getGroupItem(QListViewItem *parent, const QStringList& groups)
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
		  iitem->setPixmap(0, KGlobal::iconLoader()->loadIcon(directory.readEntry("Icon"),
															  KIconLoader::Small));
		  
		  return iitem;
		}
    }
  return item;
}

void IndexWidget::currentChanged(QListViewItem *item)
{
  if (item == 0)
    return;

  if (item->childCount() != 0)
    return;
  IndexListItem *iitem = (IndexListItem*)(item);
  if (iitem && iitem->_module)
    {
      emit moduleActivated(iitem->_module);
    }
}

void IndexWidget::updateItem(QListViewItem *item, ConfigModule *module)
{
  while (item)
    {
      if (item->childCount() != 0)
		updateItem(item->firstChild(), module);
	  
      IndexListItem *iitem = (IndexListItem*)(item);
      if (iitem->getModule() == module)
		{
		  _tree->setSelected(item, true);
		  break;
		}
	  
      item = item->nextSibling();
    }
}

void IndexWidget::moduleChanged(ConfigModule *module)
{
  updateItem(_tree->firstChild(), module);
}

void IndexWidget::makeVisible(ConfigModule *module)
{
  QListViewItem *item;
  
  item = _root;
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

void IndexWidget::iconButtonClicked()
{
  _tree->hide();
  _iconbtn->setEnabled(false);
  _treebtn->setEnabled(true);
}

void IndexWidget::treeButtonClicked()
{
  _tree->show();
  _iconbtn->setEnabled(true);
  _treebtn->setEnabled(false);
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
}

IndexListItem::IndexListItem(QListView *parent, ConfigModule *module)
  : IndexItem(parent), _module(module)
{
  if (!module)
    return;

  setText(0, module->name());
  setPixmap(0, module->icon());
}
