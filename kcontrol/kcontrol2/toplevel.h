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



#ifndef __TOPLEVEL_H__
#define __TOPLEVEL_H__


#include <ktmainwindow.h>


class KDockContainer;


#include "index.h"
#include "modules.h"


class TopLevel : public KTMainWindow
{
  Q_OBJECT

public:

  TopLevel( const char* name=0 );
  ~TopLevel();


public slots:

  void showModule(QString desktopFile);


protected:

  void initMenuBar();
  void initToolBars();
  void initStatusBar();

  bool queryClose();


protected slots:

  void moduleDoubleClicked(ConfigModule *module);
  void newModule(const QString &name);
 

private:

  IndexPane        *_index;
  ConfigModuleList _modules;
  KDockContainer   *_container;

};


#endif




















