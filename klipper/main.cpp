/* -------------------------------------------------------------

   Klipper - Cut & paste history for KDE

   (C) by Andrew Stanley-Jones

   Generated with the KDE Application Generator

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

static const char *description =
	I18N_NOOP("KDE Cut & Paste history utility");

int main(int argc, char *argv[])
{
  KAboutData aboutData("klipper", I18N_NOOP("Klipper"),
    klipper_version, description, KAboutData::License_Artistic,
		       "(c) 1998, Andrew Stanley-Jones\n"
		       "1998-2002, Carsten Pfeiffer\n"
		       "2001, Patrick Dubroy");

  aboutData.addAuthor("Carsten Pfeiffer",
                      I18N_NOOP("Author, Maintainer"),
                      "pfeiffer@kde.org");

  aboutData.addAuthor("Andrew Stanley-Jones",
                      I18N_NOOP( "Original Author" ),
                      "asj@cban.com");

  aboutData.addAuthor("Patrick Dubroy",
                      I18N_NOOP("Contributor"),
                      "patrickdu@corel.com");

  KCmdLineArgs::init( argc, argv, &aboutData );
  KUniqueApplication::addCmdLineOptions();

  if (!KUniqueApplication::start()) {
       fprintf(stderr, "%s is already running!\n", aboutData.appName());
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
  return ret;
}
