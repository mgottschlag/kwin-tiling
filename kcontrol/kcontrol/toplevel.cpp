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

#include <qstringlist.h>
#include <qpushbutton.h>
#include <kapp.h>
#include <ktoolbar.h>
#include <kglobal.h>
#include <kstddirs.h>
#include <kmenubar.h>
#include <qlineedit.h>
#include <qsplitter.h>
#include <kiconloader.h>
#include <qmenubar.h>
#include <qdialog.h>
#include <kcmodule.h>
#include <klocale.h>
#include <kstdaccel.h>
#include <qtabwidget.h>


#include "toplevel.h"
#include "toplevel.moc"
#include "kdockwidget.h"
#include "aboutwidget.h"
#include "closedlg.h"


TopLevel::TopLevel(const char* name)
  : KTMainWindow( name )
{
  // initialize the entries
  _modules.readDesktopEntries();

  // create the splitter
  QSplitter *splitter = new QSplitter(this);

  // create the left hand side (the tree view)
  _index = new IndexPane(splitter);
  _index->fillIndex(_modules);
  splitter->setResizeMode(_index,QSplitter::KeepSize);
  connect(_index, SIGNAL(moduleActivated(ConfigModule*)),
	  this, SLOT(moduleActivated(ConfigModule*)));

  // set up the right hand side (the docking area)
  _container = new KDockContainer(splitter);
  connect(_container, SIGNAL(newModule(const QString&)), this, SLOT(newModule(const QString&)));

  // insert the about widget
  AboutWidget *aw = new AboutWidget(this);
  _container->addWidget(aw, kapp->miniIcon());
  _container->showPage(aw);

  // set the main view
  setView(splitter);

  // initialize the various *bars
  initMenuBar();
  initToolBars();
  initStatusBar();

  setPlainCaption(i18n("KDE Control Center"));
}


TopLevel::~TopLevel()
{
}


void TopLevel::initMenuBar()
{
  QPopupMenu *file_menu = new QPopupMenu();

  file_menu->insertItem(BarIcon("exit"), i18n("E&xit"), kapp, SLOT(quit()),
                        KStdAccel::quit());

  KMenuBar* menu_bar = new KMenuBar(this);
  menu_bar->insertItem(i18n("&File"), file_menu);
	
  menu_bar->insertItem(i18n("&Help"), helpMenu(
    i18n("The KDE Control Center\n\nFramework: Matthias Hölzer-KlÃ¶pfel "
	 "(hoelzer@kde.org)\n\nModules: almost the whole KDE team!")));

  setMenu(menu_bar);
}


void TopLevel::initToolBars()
{
  /*
  KToolBar* tool_bar_0 = toolBar(0);
  tool_bar_0->setFullWidth(false);
  tool_bar_0->insertButton( BarIcon("fileopen"), 1 );
  tool_bar_0->insertButton( BarIcon("filefloppy"), 2 );
  tool_bar_0->setFullWidth( true );
  */
}


void TopLevel::initStatusBar()
{
  KStatusBar* status_bar = statusBar();
  status_bar->setInsertOrder(KStatusBar::RightToLeft);
  status_bar->insertItem(i18n("Welcome to the KDE Control Center"), 4);
}


void TopLevel::moduleActivated(ConfigModule *module)
{
  QWidget *widget = module->module(_container);
  if (widget)
    {
      _container->showPage(widget);
      newModule(widget->caption());
      widget->show();
      widget->raise();
    }
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
      for (ConfigModule *mod = _modules.first(); mod != 0; mod = _modules.next())
	if (mod->fileName() == *it)
	  {
	    QWidget *widget = mod->module(_container);
	    if (widget)
	      {
		_container->showPage(widget);
	        newModule(widget->caption());
		widget->show();
	      }

	    // tell the index to display the module
	    _index->makeVisible(mod);

	    // tell the index to mark this module as loaded
	    _index->moduleChanged(mod);
	  }
    }
}


bool TopLevel::queryClose()
{
  int cnt=0;

  CloseDialog dlg(0);

  for (ConfigModule *mod = _modules.first(); mod != 0; mod = _modules.next())
    if (mod->isChanged())
      {
	dlg.addUnsaved(mod->icon(), mod->name());
	cnt++;
      }
  
  return (cnt == 0) || (dlg.exec() == QDialog::Accepted);
}


void TopLevel::newModule(const QString &name)
{
  QString cap = i18n("KDE Control Center");
  
  if (!name.isEmpty())
    cap += " - [" + name +"]";

  setPlainCaption(cap);
}
