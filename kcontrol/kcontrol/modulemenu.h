this file is currently not used.
this message breaks compilation.
that is intentional :-]

/*
  Copyright (c) 2000 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
 
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
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/                                                                            
 
#ifndef __modulemenu_h__
#define __modulemenu_h__


#include <qstringlist.h>
#include <q3intdict.h>
#include <qstring.h>
#include <qwidget.h>

#include <kmenu.h>


class ConfigModule;
class ConfigModuleList;


class ModuleMenu : public KMenu
{
  Q_OBJECT

public:
  ModuleMenu(ConfigModuleList *list, QWidget * parent = 0, const char * name = 0);

Q_SIGNALS:
  void moduleActivated(ConfigModule*);

private Q_SLOTS:
  void moduleSelected(int id);

protected:
  void fill(KMenu *parentMenu, const QString &parentPath);

private:
  int id;

  ConfigModuleList       *_modules;
  Q3IntDict<ConfigModule> _moduleDict;
};


#endif
