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

#include <qlayout.h>
#include <qpushbutton.h>

#include <kglobal.h>
#include <klocale.h>
#include <kconfig.h>

#include "indexwidget.h"
#include "indexwidget.moc"
#include "moduletreeview.h"
#include "moduleiconview.h"
#include "modules.h"

IndexWidget::IndexWidget(ConfigModuleList *modules, QWidget *parent ,const char *name)
  : QWidget(parent, name)
  , _modules(modules)
  , viewMode(Icon)
{
  // treeview
  _tree = new ModuleTreeView(_modules, this);
  _tree->fill();
  connect(_tree, SIGNAL(moduleSelected(ConfigModule*)), 
		  this, SLOT(moduleSelected(ConfigModule*)));

  // iconview
  _icon = new ModuleIconView(_modules, this);
  _icon->fill();
  connect(_icon, SIGNAL(moduleSelected(ConfigModule*)), 
		  this, SLOT(moduleSelected(ConfigModule*)));

  // view button
  _viewbtn = new QPushButton(this);
  _viewbtn->setFixedHeight(22);
  connect(_viewbtn, SIGNAL(clicked()), this, SLOT(viewButtonClicked()));

  KConfig *config = KGlobal::config();
  config->setGroup("Index");
  QString defaultview = config->readEntry("DefaultView", "Icon");

  if (defaultview == "Tree")
    activateView(Tree);
  else
    activateView(Icon);
}

IndexWidget::~IndexWidget()
{
  KConfig *config = KGlobal::config();
  config->setGroup("Index");
  if (viewMode == Tree)
    config->writeEntry("DefaultView", "Tree");
  else
    config->writeEntry("DefaultView", "Icon");
  config->sync();
}

void IndexWidget::resizeEvent(QResizeEvent *)
{
  _viewbtn->move(0, height()-22);
  _viewbtn->resize(width(), 22);

  _tree->move(0,0);
  _tree->resize(width(), height()-22);

  _icon->move(0,0);
  _icon->resize(width(), height()-22);
  _icon->setGridX(width()-26);
  _icon->fill();
}

void IndexWidget::moduleSelected(ConfigModule *m)
{
  if(!m) return;

  emit moduleActivated(m);

  if (sender()->inherits("ModuleIconView"))
	{
	  _tree->makeVisible(m);

	  _tree->disconnect(SIGNAL(moduleSelected(ConfigModule*)));
	  _tree->makeSelected(m);
	  connect(_tree, SIGNAL(moduleSelected(ConfigModule*)), 
			  this, SLOT(moduleSelected(ConfigModule*)));
	}
  else if (sender()->inherits("ModuleTreeView"))
	{
	  _icon->makeVisible(m);

	  _icon->disconnect(SIGNAL(moduleSelected(ConfigModule*)));
	  _icon->makeSelected(m);
	  connect(_icon, SIGNAL(moduleSelected(ConfigModule*)), 
			 this, SLOT(moduleSelected(ConfigModule*)));
	}
}

void IndexWidget::makeSelected(ConfigModule *module)
{
  _icon->disconnect(SIGNAL(moduleSelected(ConfigModule*)));
  _tree->disconnect(SIGNAL(moduleSelected(ConfigModule*)));

  _icon->makeSelected(module);
  _tree->makeSelected(module);

  connect(_icon, SIGNAL(moduleSelected(ConfigModule*)), 
		  this, SLOT(moduleSelected(ConfigModule*)));

  connect(_tree, SIGNAL(moduleSelected(ConfigModule*)), 
		  this, SLOT(moduleSelected(ConfigModule*)));
}

void IndexWidget::makeVisible(ConfigModule *module)
{
  _icon->makeVisible(module);
  _tree->makeVisible(module);
}

void IndexWidget::activateView(IndexViewMode mode)
{
  viewMode = mode;

  if (mode == Icon)
    {
      _tree->hide();
      _icon->show();
      _icon->setFocus();
      _viewbtn->setText(i18n("S&witch to treeview."));
    }
  else
    {
      _tree->show();
      _tree->setFocus();
      _icon->hide();
      _viewbtn->setText(i18n("S&witch to iconview."));
    }
}

void IndexWidget::viewButtonClicked()
{
  if (viewMode == Icon)
    activateView(Tree);
  else
    activateView(Icon);
}
