/* -------------------------------------------------------------

   Klipper - Cut & paste history for KDE

   (C) by Andrew Stanley-Jones

   Generated with the KDE Application Generator

   Licensed under the GNU GPL Version 2

 ------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>

#include <klocale.h>
#include <kcmdlineargs.h>
#include <kwin.h>
#include <kaboutdata.h>
#include <kuniqueapplication.h>

#include "toplevel.h"
#include "version.h"

extern "C" int kdemain(int argc, char *argv[])
{
  Klipper::createAboutData();
  KCmdLineArgs::init( argc, argv, Klipper::aboutData());
  KUniqueApplication::addCmdLineOptions();

  if (!KUniqueApplication::start()) {
       fprintf(stderr, "Klipper is already running!\n");
       exit(0);
  }
  KUniqueApplication app;
  app.disableSessionManagement();

  Klipper *toplevel = new Klipper();

  KWin::setSystemTrayWindowFor( toplevel->winId(), 0 );
  toplevel->setGeometry(-100, -100, 42, 42 );
  toplevel->show();

  int ret = app.exec();
  delete toplevel;
  Klipper::destroyAboutData();
  return ret;
}
