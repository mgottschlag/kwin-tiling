/*
  toplevel.cpp - the main view of the KDE control center

  written 1997 by Matthias Hoelzer

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
#include <qpixmap.h>
#include <qpushbutton.h>
#include <qheader.h>

#include <kiconloader.h>
#include <klocale.h>
#include <kconfig.h>
#include <kmenubar.h>
#include <kstdaccel.h>

#include "toplevel.moc"

TopLevel::TopLevel (ConfigList *cl)
  : KTMainWindow(), ID_GENERAL(1)
{
  configList = cl;

  setupMenuBar();
  setupStatusBar();

  splitter = new QSplitter(this);

  treelist = new QListView(splitter);
  treelist->setFrameStyle( QFrame::WinPanel | QFrame::Sunken );
  treelist->setRootIsDecorated( TRUE );
  treelist->addColumn("");
  treelist->header()->hide();
  configList->fillTreeList(treelist);
  treelist->setMinimumSize(200, 400);
  splitter->setResizeMode(treelist, QSplitter::KeepSize);

  widgetStack = new QWidgetStack(splitter);
  widgetStack->addWidget(new mainWidget(widgetStack), 1);
  widgetStack->raiseWidget(1);
  widgetStack->setMinimumSize(widgetStack->visibleWidget()->minimumSize());

  connect(treelist, SIGNAL(selectionChanged(QListViewItem*)), 
	  this, SLOT(itemSelected(QListViewItem*)));
  connect(treelist, SIGNAL(returnPressed(QListViewItem*)), 
	  this, SLOT(itemSelected(QListViewItem*)));
  connect(widgetStack, SIGNAL(aboutToShow(QWidget*)), 
	  this, SLOT(aboutToShow(QWidget*)));
  
  setView(splitter);
  
  show();

  KConfig *config = kapp->config();
  config->setGroup("Options");
  KModuleListEntry::swallowingEnabled = 
    config->readNumEntry("SwallowEnabled", TRUE);
  options->setItemChecked(swallowID, KModuleListEntry::swallowingEnabled);
}


void TopLevel::setupMenuBar()
{
    KStdAccel stdAccel;

    file = new QPopupMenu();
    options = new QPopupMenu();

    file->insertItem(i18n("&Quit"),
                     KApplication::kApplication(), SLOT(quit()),
                     stdAccel.quit());

    options->setCheckable(TRUE);
    swallowID = options->insertItem(i18n("&Swallow modules"),
                        this, SLOT(swallowChanged()));

    QPopupMenu *hMenu = helpMenu(i18n("KDE Control Center - "
	     "Version 1.0\n\n"
	     "Written by Matthias Hölzer\n"
	     "(hoelzer@physik.uni-wuerzburg.de)\n\n"
	     "Thanks to:\n"
	     "S. Kulow, P. Dowler, M. Wuebben & M. Jones."));

    menubar = new KMenuBar(this);
    menubar->insertItem(i18n("&File"), file);
    menubar->insertItem(i18n("&Options"), options);
    menubar->insertSeparator(-1);
    menubar->insertItem(i18n("&Help"), hMenu);

    setMenu(menubar);
}


void TopLevel::setupStatusBar()
{
  statusbar = new KStatusBar(this);
  statusbar->insertItem("", ID_GENERAL);
  statusbar->setInsertOrder(KStatusBar::LeftToRight);
  setStatusBar(statusbar);
}


void TopLevel::itemSelected( QListViewItem *listEntry)
{
    if (!listEntry)
	return;
    
    ConfigTreeItem* cti = (ConfigTreeItem*)listEntry;
    cti->moduleListEntry->execute(widgetStack);
    statusbar->changeItem(cti->moduleListEntry->getComment(), ID_GENERAL);
}


void TopLevel::swallowChanged()
{
  KModuleListEntry::swallowingEnabled = !KModuleListEntry::swallowingEnabled;

  KConfig *config = kapp->config();
	
  config->setGroup("Options");
  config->writeEntry("SwallowEnabled", KModuleListEntry::swallowingEnabled);

  options->setItemChecked(swallowID, KModuleListEntry::swallowingEnabled);
  config->sync();
}


void TopLevel::aboutToShow(QWidget *widget)
{
  widgetStack->setMinimumSize(widget->minimumSize());
}
