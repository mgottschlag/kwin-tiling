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



#include <stdlib.h> 
#include <unistd.h>
#include <sys/types.h>


#include <qstringlist.h>


#include <kglobal.h>
#include <kstddirs.h>
#include <klibloader.h>


#include "modloader.h"
#include "proxymodule.h"


KCModule *ModuleLoader::module(const ModuleInfo &mod, QWidget *parent)
{
  // check, if we require root privileges we don't have
  bool needRoot = mod.onlyRoot() && (getuid() != 0);

  /*
   * Simple libraries as modules are the easiest case: 
   *  We just have to load the library and get the module
   *  from the factory.
   */

  if (mod.type() == ModuleInfo::Library && !needRoot)
    {
      // get the library loader instance
      KLibLoader *loader = KLibLoader::self();

      // try to load the library
      QString libname("libkcm_%1");
      KLibrary *lib = loader->library(libname.arg(mod.library()));
      if (!lib)
	return 0;

      // get the create_ function
      QString factory("create_%1");
      void *create = lib->symbol(factory.arg(mod.handle()));
      if (!create)
	return 0;

      // create the module
      KCModule* (*func)(QWidget *, const char *);
      func = (KCModule* (*)(QWidget *, const char *)) create;
      return  func(parent, "");
    }

  /*
   * Libraries that require root privileges are very tricky:
   *  We have to call an external process with kdesu, provide 
   *  a proxy in this process and have both communicate via
   *  DCOP.
   */

  if (mod.type() == ModuleInfo::Library && needRoot)
    {      
      DCOPProxy *p = new DCOPProxy(parent, mod);
      if (p->running())
	return p;
      else
	return 0;
    }

  /*
   * What remains are simple processes that are run via the
   * control center just for convenience. 
   * These processes are created in ConfigModules::process, so
   * we just return 0 to indicate that no module is available.
   */  

  return 0;
}
