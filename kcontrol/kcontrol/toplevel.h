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

#ifndef __TOPLEVEL_H__
#define __TOPLEVEL_H__

#include <ktmainwindow.h>

class QTabWidget;
class QSplitter;

class DockContainer;
class IndexWidget;
class SearchWidget;
class HelpWidget;
class ConfigModule;
class ConfigModuleList;

class TopLevel : public KTMainWindow
{
  Q_OBJECT

public:
  TopLevel( const char* name=0 );
  ~TopLevel();

  void showModule(QString desktopFile);

protected:
  void setupActions(); 

protected slots:
  void activateModule(const QString& name);
  void moduleActivated(ConfigModule *module);
  void newModule(const QString &name, const QString &quickhelp);
  void activateIconView();
  void activateTreeView();

private:
  QTabWidget     *_tab;
  DockContainer  *_dock;
  QSplitter      *_splitter;

  IndexWidget  *_indextab;
  SearchWidget *_searchtab;
  HelpWidget   *_helptab;

  ConfigModule     *_active;
  ConfigModuleList *_modules;
};

#endif
