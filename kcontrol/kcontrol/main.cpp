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
#include <kcmdlineargs.h>
#include <dcopclient.h>
#include <klocale.h>

#include <kaboutdata.h>


#include "main.h"
#include "main.moc"
#include "toplevel.h"

static KCmdLineOptions options[] =
{
   { "+module", I18N_NOOP("Configuration module to open."), 0 },
   { 0,0,0 }
};

MyApplication::MyApplication()
  : KUniqueApplication(), toplevel(0)
{
  if (isRestored())
    RESTORE(TopLevel)
  else 
    toplevel = new TopLevel();
  
  setMainWidget(toplevel);
  toplevel->resize(640,400);
}


int MyApplication::newInstance()
{
  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
  for (int i=0; i < args->count(); i++)
  {
     toplevel->showModule(args->arg(i));
  }
  toplevel->raise();

  return 0;
}


int main(int argc, char *argv[])
{
  KAboutData aboutData( "kcontrol", I18N_NOOP("KDE Control Centre"), 
    "v2.0pre", "The KDE Control Centre", KAboutData::License_GPL, 
    "(c) 1998-2000, The KDE Control Centre Developers");
  aboutData.addAuthor("Matthias Hoelzer-Kluepfel",0, "hoelzer@kde.org");
  KCmdLineArgs::init( argc, argv, &aboutData );
  KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.
 
  
  if (!MyApplication::start())
    exit(0); // Don't do anything if we are already running
  
  MyApplication app; //&aboutData
  // TODO: MyApplication (a KUniqueApplication) doesn't 
  // support KApplication::KApplication(KAboutData *)
  // So, I'm leaving this alone, and will let the 
  // maintainter of kcontrol finish it up!

  // show the whole stuff
  app.mainWidget()->show();

  return app.exec();
}

