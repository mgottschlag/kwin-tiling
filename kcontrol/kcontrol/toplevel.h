/*
  toplevel.h - the mainview of the KDE control center

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


#ifndef _TOPLEVEL_H_
#define _TOPLEVEL_H_

#include <qapplication.h>
#include <qmenubar.h>
#include <qlistview.h>
#include <qsplitter.h>
#include <qwidgetstack.h>

#include <kapp.h>
#include <ktoolbar.h>
#include <kstatusbar.h>
#include <ktmainwindow.h>

#include "mainwidget.h"
#include "configlist.h"

class TopLevel : public KTMainWindow
{
  Q_OBJECT

public:

  TopLevel(ConfigList *cl);

private:

  QPopupMenu *file, *hMenu, *options;
  int        helpModuleID, helpID, swallowID;

protected:

  void setupMenuBar();
  void setupStatusBar();

private:

  KMenuBar   *menubar;
  KToolBar   *toolbar;
  QListView  *treelist;
  KStatusBar *statusbar;
  QSplitter *splitter;
  ConfigList *configList;
  QWidgetStack *widgetStack;

  const int ID_GENERAL;

  void updateMenu();

public slots:

  void swallowChanged();

  void itemSelected(QListViewItem*);

  void aboutToShow(QWidget *);

};

#endif


