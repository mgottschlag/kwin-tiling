/*
  Copyright (c) 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
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
                                          
#include <kapp.h>
#include <kglobal.h>
#include <kstddirs.h>

#include <qsplitter.h>
#include <qtabwidget.h>

#include <kaction.h>
#include <kstdaction.h>
#include <klocale.h>

#include "indexwidget.h"
#include "searchwidget.h"
#include "helpwidget.h"
#include "dockcontainer.h"
#include "aboutwidget.h"
#include "modules.h"
#include "proxywidget.h"

#include "toplevel.h"
#include "toplevel.moc"


TopLevel::TopLevel(const char* name)
  : KTMainWindow( name )
  , _active(0)
{
  // initialize the entries
  _modules = new ConfigModuleList();
  _modules->readDesktopEntries();

  // create the splitter
  _splitter = new QSplitter(this);

  // create the left hand side (the tab view)
  _tab = new QTabWidget(_splitter);

  // index tab
  _indextab = new IndexWidget(_modules, _tab);
  connect(_indextab, SIGNAL(moduleActivated(ConfigModule*)),
		  this, SLOT(moduleActivated(ConfigModule*)));
  _tab->addTab(_indextab, "In&dex");

  // search tab
  _searchtab = new SearchWidget(_tab);
  _searchtab->populateKeywordList(_modules);
  connect(_searchtab, SIGNAL(moduleSelected(const QString&)),
		  this, SLOT(activateModule(const QString&)));

  _tab->addTab(_searchtab, "S&earch");

  // help tab
  _helptab = new HelpWidget(_tab);
  _tab->addTab(_helptab, "Hel&p");
  
  // set a reasonable resize mode
  _splitter->setResizeMode(_tab, QSplitter::KeepSize);

  // set up the right hand side (the docking area)
  _dock = new DockContainer(_splitter);
  connect(_dock, SIGNAL(newModule(const QString&, const QString&)),
		  this, SLOT(newModule(const QString&, const QString&)));

  // insert the about widget
  AboutWidget *aw = new AboutWidget(this);
  _dock->setBaseWidget(aw);

  // set the main view
  setView(_splitter);

  // initialize the GUI actions
  setupActions();

  setPlainCaption(i18n("KDE Control Center"));
}

TopLevel::~TopLevel() {}

void TopLevel::setupActions()
{
  KStdAction::quit(kapp, SLOT(quit()), actionCollection());
  (void)new KAction(i18n("&Icon View"), 0, this, SLOT(activateIconView()),
                    actionCollection(), "activate_iconview");
  (void)new KAction(i18n("&Tree View"), 0, this, SLOT(activateTreeView()),
                    actionCollection(), "activate_treeview");
  createGUI("kcontrolui.rc");
}

void TopLevel::activateIconView()
{
  _indextab->activateView(Icon);
}

void TopLevel::activateTreeView()
{
  _indextab->activateView(Tree);
}

void TopLevel::newModule(const QString &name, const QString &quickhelp)
{
  QString cap = i18n("KDE Control Center");
  
  if (!name.isEmpty())
    cap += " - [" + name +"]";

  setPlainCaption(cap);

  _helptab->setText(quickhelp);
}

void TopLevel::moduleActivated(ConfigModule *module)
{
  _dock->dockModule(module);
}

void TopLevel::showModule(QString desktopFile)
{
  // strip trailing ".desktop"
  int pos = desktopFile.find(".desktop");
  if (pos > 0)
    desktopFile = desktopFile.left(pos);

  // locate the desktop file
  QStringList files;
  files = KGlobal::dirs()->findAllResources("apps", "Settings/"+desktopFile+".desktop", TRUE);
  
  // show all matches
  QStringList::Iterator it;
  for (it = files.begin(); it != files.end(); ++it)
    {
      for (ConfigModule *mod = _modules->first(); mod != 0; mod = _modules->next())
		if (mod->fileName() == *it && mod != _active)
		  {
			// tell the index to display the module
			_indextab->makeVisible(mod);
			
			// tell the index to mark this module as loaded
			_indextab->makeSelected(mod);

			// dock it
            _dock->dockModule(mod);
            break;
	  }
    }
}

void TopLevel::activateModule(const QString& name)
{
  qDebug("activate: %s", name.latin1());
  for (ConfigModule *mod = _modules->first(); mod != 0; mod = _modules->next())
	{
	  if (mod->name() == name)
		{
		  // tell the index to display the module
		  _indextab->makeVisible(mod);

		  // tell the index to mark this module as loaded
		  _indextab->makeSelected(mod);
		  
		  // dock it
          _dock->dockModule(mod);
		  break;
		}
	}
}
