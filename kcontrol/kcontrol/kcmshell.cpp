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

#include <stdlib.h>
#include <kapp.h>
#include <kglobal.h>
#include <klocale.h>
#include <kstddirs.h>
#include <kcmodule.h>

#include "moduleinfo.h"
#include "modloader.h"


int main(int argc, char *argv[])
{
  KApplication app(argc, argv, "kcmshell");

  // check parameters
  if (argc != 2)
    {
      cerr << i18n("usage: kcmshell module") << endl;
      exit(-1);
    }

  // locate the desktop file
  QStringList files;
  files = KGlobal::dirs()->findAllResources("apps", QString("Settings/%1.desktop").arg(argv[1]), true);

  // check the matches
  if (files.count() > 1)
    cerr << i18n("Module name not unique. Taking the first match.") << endl;
  if (files.count() <= 0)
    {
      cerr << i18n("Module not found!") << endl;
      exit(-1);
    }

  // load the module
  ModuleInfo info(files[0]);
  KCModule *module = ModuleLoader::module(info, 0);
  
  if (module)
    {
      // create the dialog
      KCDialog dlg(module, 0, 0, true);
      dlg.setCaption(info.name());

      // run the dialog
      return dlg.exec();
    }

  return 0;
}
