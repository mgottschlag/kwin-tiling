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



#include <kapp.h>
#include <dcopclient.h>


#include "main.h"
#include "main.moc"
#include "toplevel.h"


MyApplication::MyApplication(int argc, char *argv[])
  : KUniqueApplication(argc, argv, "kcontrol"), toplevel(0)
{
  if (isRestored())
    RESTORE(TopLevel)
  else 
    toplevel = new TopLevel();
  
  setMainWidget(toplevel);
  toplevel->resize(640,400);
}


int MyApplication::newInstance(QValueList<QCString> params)
{
  QValueList<QCString>::Iterator it = params.begin();
  it++; // skip program name
  for (; it != params.end(); it++)
    {
      toplevel->showModule(*it);
    }

  toplevel->raise();

  return 0;
}


int main(int argc, char *argv[])
{
  MyApplication app(argc, argv);

  // process command line parameters
  QValueList<QCString> params;
  for (int i=0; i<argc; i++)
    params.append(argv[i]);
  app.newInstance(params);

  // register as kcontrol
  app.dcopClient()->attach();
  app.dcopClient()->registerAs("kcontrol");

  // show the whole stuff
  app.mainWidget()->show();

  return app.exec();
}
