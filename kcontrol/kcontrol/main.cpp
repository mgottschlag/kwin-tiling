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


/**
 * Howto debug:
 *    start "kcontrol --nofork" in a debugger.
 *    
 * If you want to test with command line arguments you need
 * -after you have started kcontrol in the debugger-
 * open another shell and run kcontrol with the desired
 * command line arguments.
 *
 * The command line arguments will be passed to the version of
 * kcontrol in the debugger via DCOP and will cause a call
 * to newInstance().
 */


#include <kapp.h>
#include <dcopclient.h>


#include "main.h"
#include "main.moc"
#include "toplevel.h"


MyApplication::MyApplication(int argc, char *argv[], const QCString &appName)
  : KUniqueApplication(argc, argv, appName), toplevel(0)
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
  //if (!MyApplication::start(argc, argv, "kcontrol"))
  //    exit(0); // Don't do anything if we are already running

  MyApplication app(argc, argv, "kcontrol");

  // show the whole stuff
  app.mainWidget()->show();

  return app.exec();
}
