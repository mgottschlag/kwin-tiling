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



#include <stream.h>
#include <unistd.h>
#include <sys/types.h>                                                   
#include <stdlib.h>


#include <qmessagebox.h>


#include <kapp.h>
#include <kglobal.h>
#include <klocale.h>
#include <kstddirs.h>
#include <dcopclient.h>


#include "kcmroot.h"
#include "kcmroot.moc"
#include "moduleinfo.h"
#include "modloader.h"


RemoteProxy::RemoteProxy(KCModule *module)
  : DCOPObject("KCModuleIface"), _module(module)
{
}


int main(int argc, char *argv[])
{
  KApplication app(argc, argv, "kcmroot");
  app.connect(&app, SIGNAL(lastWindowClosed()), &app, SLOT(quit()));

  // see if we really run as root
  if (getuid() != 0)
    {
      cerr << "Only root will run this program!" << endl;
      exit(-1);
    }

  // check parameters
  if ((argc < 2) || (argc > 3))
    {
      cerr << i18n("usage: kcmroot module.desktop [dcopserver]") << endl;
      exit(-1);
    }

  // load the module
  ModuleInfo info(argv[1]);
  KCModule *module = ModuleLoader::module(info, 0);

  if (module)
    {
      RemoteProxy proxy(module);

      if (!app.dcopClient()->attach())
	return -1;
      QCString id = info.moduleId();
      if (id != app.dcopClient()->registerAs(id))
	cerr << "Warning: multiple instances registered" << endl;

      // show the proxy
      module->move(10000, 10000);

      return app.exec();
    }
  
  return -1;
}
