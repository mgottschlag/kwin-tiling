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



#ifndef __KCMROOT_H__
#define __KCMROOT_H__


#include <qobject.h>


#include <kcmodule.h>


#include "KCModuleIface.h"


class RemoteProxy : public QObject, virtual public KCModuleIface
{
  Q_OBJECT

public:

  RemoteProxy(KCModule *module);

  int winId() { return _module->winId(); };

  void load() { _module->load(); };
  void save() { _module->save(); };
  void defaults() { _module->defaults(); };
 
  void init() { _module->init(); };
 
  int buttons() { return _module->buttons(); };

private:

  KCModule *_module;

};


#endif
