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

#include "toplevel.moc"

void MySplitter::resizeEvent(QResizeEvent *event)
{
  QSplitter::resizeEvent(event);

  emit resized();
}


TopLevel::TopLevel (ConfigList *cl)
  : KTopLevelWidget(), ID_GENERAL(1)
{
  configList = cl;
  current = 0;

  setupMenuBar();
  setupStatusBar();

  splitter = new MySplitter(this);
  connect(splitter, SIGNAL(resized()), this, SLOT(doResize()));

  treelist = new QListView(splitter);
  treelist->setFrameStyle( QFrame::WinPanel | QFrame::Sunken );
  treelist->setRootIsDecorated( TRUE );
  treelist->addColumn("");
  treelist->header()->hide();
  configList->fillTreeList(treelist);
  treelist->setMinimumSize(200, 400);
  splitter->setResizeMode(treelist,QSplitter::KeepSize);

  mwidget = new mainWidget(splitter);
  mwidget->setMinimumSize(450, 400);

  connect(mwidget, SIGNAL(resized()), this, SLOT(doResize()));

  connect(treelist, SIGNAL(selectionChanged(QListViewItem*)), this, SLOT(itemSelected(QListViewItem*)));
  connect(treelist, SIGNAL(returnPressed(QListViewItem*)), this, SLOT(itemSelected(QListViewItem*)));

  setView(splitter);

  show();
  resizeEvent(NULL);

  KConfig *config = kapp->getConfig();
  config->setGroup("Options");
  KModuleListEntry::swallowingEnabled = config->readNumEntry("SwallowEnabled", TRUE);
  options->setItemChecked(swallowID, KModuleListEntry::swallowingEnabled);
}


void TopLevel::setupMenuBar()
{
    file = new QPopupMenu();
    options = new QPopupMenu();

    file->insertItem(i18n("E&xit"),
		     KApplication::getKApplication(), SLOT(quit()));

    options->setCheckable(TRUE);
    swallowID = options->insertItem(i18n("&Swallow modules"),
                        this, SLOT(swallowChanged()));

    QPopupMenu *helpMenu = kapp->getHelpMenu(true, i18n("KDE Control Center - "
					"Version 1.0\n\n"
					"Written by Matthias Hölzer\n"
					"(hoelzer@physik.uni-wuerzburg.de)\n\n"
					"Thanks to:\n"
					"S. Kulow, P. Dowler, M. Wuebben & M. Jones."));

    menubar = new KMenuBar(this);
    menubar->insertItem(i18n("&File"), file);
    menubar->insertItem(i18n("&Options"), options);
    menubar->insertSeparator(-1);
    menubar->insertItem(i18n("&Help"), helpMenu);

    setMenu(menubar);
}


void TopLevel::setupStatusBar()
{
  statusbar = new KStatusBar(this);
  statusbar->insertItem("", ID_GENERAL);
  statusbar->setInsertOrder(KStatusBar::LeftToRight);
  setStatusBar(statusbar);
}


void TopLevel::resizeEvent(QResizeEvent *)
{
  updateRects();

  doResize();
}


void TopLevel::doResize()
{
  if (KModuleListEntry::visibleWidget)
    KModuleListEntry::visibleWidget->resize(mwidget->width(), mwidget->height());
}


void TopLevel::itemSelected( QListViewItem *listEntry)
{
    if (!listEntry)
	return;
    ConfigTreeItem* cti = (ConfigTreeItem*)listEntry;
    cti->moduleListEntry->execute(mwidget);
}




void TopLevel::swallowChanged()
{
  KModuleListEntry::swallowingEnabled = !KModuleListEntry::swallowingEnabled;

  KConfig *config = kapp->getConfig();
	
  config->setGroup("Options");
  config->writeEntry("SwallowEnabled", KModuleListEntry::swallowingEnabled);

  options->setItemChecked(swallowID, KModuleListEntry::swallowingEnabled);
  config->sync();
}


void TopLevel::ensureSize(int w, int h)
{
  int width=w, height=h;

  if (w < mwidget->width())
    width = mwidget->width();
  if (h < mwidget->height())
    height = mwidget->height();

  width += treelist->width() + 6; // 6 ~= width of QSplitter slider
  // Markus W. : If we are in macStyle we dont need to add the height of
  // the menubar
  KConfig* config = kapp->getConfig();
  config->setGroup("KDE");
  if (config->readEntry("macStyle") == "on")
     height += statusbar->height();
  else
    height += menubar->height()+statusbar->height();
  if (width < this->width())
    width = this->width();
  if (height < this->height())
    height = this->height();

  resize(width, height);
}
