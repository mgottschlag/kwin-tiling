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

#include <iostream.h>
#include <unistd.h>

#include <kapp.h>
#include <kglobal.h>
#include <klocale.h>
#include <kstddirs.h>
#include <kcmodule.h>
#include <kdesktopfile.h>
#include <klibloader.h>


int main(int argc, char *argv[])
{
  KApplication app(argc, argv, "kcminit");

  QStringList libs;
  QString     libname, funcname, call;
  
  // locate the desktop files
  QStringList files;
  files = KGlobal::dirs()->findAllResources("apps", "Settings/*.desktop", true);

  // look for Init=... entries
  QStringList::Iterator it;
  for (it = files.begin(); it != files.end(); ++it) 
    {
      KDesktopFile desktop(*it);

      libname = desktop.readEntry("X-KDE-LibraryName");
      funcname = desktop.readEntry("X-KDE-Init");
      call = funcname+"@"+libname;
      if (!funcname.isEmpty() && !libname.isEmpty() && !libs.contains(call)) 
	libs.append(call);
    }

  // get the library loader instance
  KLibLoader *loader = KLibLoader::self();

  for (it = libs.begin(); it != libs.end(); ++it) 
    {
      int pos = (*it).find("@");
      funcname = (*it).left(pos);
      libname = (*it).mid(pos+1);

      // try to load the library
      QString ln("libkcm_%1");
      KLibrary *lib = loader->library(ln.arg(libname));
      if (lib)
	{
	  // get the init_ function
	  QString factory("init_%1");
	  void *init = lib->symbol(factory.arg(funcname));
	  if (init)
	    {
	      // initialize the module
	      void (*func)() = (void(*)())init;
	      func();
	    }
	}
    }
  
  return 0;
}
