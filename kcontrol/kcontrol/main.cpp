/*
  Copyright (c) 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
  Copyright (c) 2000 Matthias Elter <elter@kde.org> 
 
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
#include <kmessagebox.h>
#include <kconfig.h>

#include "main.h"
#include "main.moc"
#include "toplevel.h"
#include "global.h"

static KCmdLineOptions options[] =
{
   { "t", 0, 0 },
   { "type <t>", I18N_NOOP("Show control modules of type \"t\"."), "User" },
   { 0,0,0 }
};

MyApplication::MyApplication()
  : KApplication()
  , toplevel(0)
{
  toplevel = new TopLevel();
  
  setMainWidget(toplevel);

  KConfig *config = KGlobal::config();
  config->setGroup("General");
  int x = config->readNumEntry("InitialWidth", 730);
  int y = config->readNumEntry("InitialHeight", 530);
  toplevel->resize(x,y);
}

MyApplication::~MyApplication()
{
  if (toplevel)
    {
      KConfig *config = KGlobal::config();
      config->setGroup("General");
      config->writeEntry("InitialWidth", toplevel->width());
      config->writeEntry("InitialHeight", toplevel->height());
      config->sync();
    }
}

int main(int argc, char *argv[])
{
  KAboutData aboutData( "kcontrol", I18N_NOOP("KDE Control Centre"), 
    "v2.0pre", "The KDE Control Centre", KAboutData::License_GPL, 
    "(c) 1998-2000, The KDE Control Centre Developers");
  aboutData.addAuthor("Matthias Hoelzer-Kluepfel",0, "hoelzer@kde.org");
  aboutData.addAuthor("Matthias Elter",0, "elter@kde.org");

  KCmdLineArgs::init( argc, argv, &aboutData );
  KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.
  KApplication::addCmdLineOptions();

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  KCGlobal::init();
  KCGlobal::setType(args->getOption("type"));

  MyApplication app;

  if (KCGlobal::types().contains("system") && !KCGlobal::root())
	{
	  KMessageBox::error(0, I18N_NOOP("Only the root user can edit system global settings!")
						 , I18N_NOOP("Error!"));
	  exit(0);
	}

  // show the whole stuff
  app.mainWidget()->show();

  return app.exec();
}
