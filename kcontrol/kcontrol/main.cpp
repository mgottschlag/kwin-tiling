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
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
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

//Added by qt3to4:
#include <QByteArray>
#include <QDesktopWidget>

#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kglobalsettings.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include "main.h"
#include "main.moc"
#include "toplevel.h"
#include "global.h"
#include "moduleIface.h"

#include "version.h"

KControlApp::KControlApp()
  : KUniqueApplication()
  , toplevel(0)
{
  toplevel = new TopLevel();

  setMainWidget(toplevel);
  KGlobal::setActiveInstance(this);

  // KUniqueApplication does dcop regitration for us
  ModuleIface *modIface = new ModuleIface(toplevel, "moduleIface");

  connect (modIface, SIGNAL(helpClicked()), toplevel, SLOT(slotHelpRequest()));
  connect (modIface, SIGNAL(handbookClicked()), toplevel, SLOT(slotHandbookRequest()));

  QRect desk = KGlobalSettings::desktopGeometry(toplevel);
  KConfig *config = KGlobal::config();
  config->setGroup("General");
  // Initial size is:
  // never bigger than workspace as reported by desk
  // 940x700 on 96 dpi, 12 pt font
  // 800x600 on 72 dpi, 12 pt font
  // --> 368 + 6 x dpiX, 312 + 4 x dpiY
  // Adjusted for font size
  int fontSize = toplevel->fontInfo().pointSize();
  if (fontSize == 0)
    fontSize = (toplevel->fontInfo().pixelSize() * 72) / toplevel->logicalDpiX();
  int x = config->readEntry(QString::fromLatin1("InitialWidth %1").arg(desk.width()), 
			       qMin( desk.width(), 368 + (6*toplevel->logicalDpiX()*fontSize)/12 ) );
  int y = config->readEntry(QString::fromLatin1("InitialHeight %1").arg(desk.height()), 
			       qMin( desk.height(), 312 + (4*toplevel->logicalDpiX()*fontSize)/12 ) );
  toplevel->resize(x,y);
}

KControlApp::~KControlApp()
{
  if (toplevel)
    {
      KConfig *config = KGlobal::config();
      config->setGroup("General");
      QDesktopWidget *desk = QApplication::desktop();
      config->writeEntry(QString::fromLatin1("InitialWidth %1").arg(desk->width()), toplevel->width());
      config->writeEntry(QString::fromLatin1("InitialHeight %1").arg(desk->height()), toplevel->height());
      config->sync();
    }
  delete toplevel;
}

extern "C" KDE_EXPORT int kdemain(int argc, char *argv[])
{
  KLocale::setMainCatalog("kcontrol");
  KAboutData aboutKControl( "kcontrol", I18N_NOOP("KDE Control Center"),
    KCONTROL_VERSION, I18N_NOOP("The KDE Control Center"), KAboutData::License_GPL,
    I18N_NOOP("(c) 1998-2004, The KDE Control Center Developers"));

  KAboutData aboutKInfoCenter( "kinfocenter", I18N_NOOP("KDE Info Center"),
    KCONTROL_VERSION, I18N_NOOP("The KDE Info Center"), KAboutData::License_GPL,
    I18N_NOOP("(c) 1998-2004, The KDE Control Center Developers"));

  QByteArray argv_0 = argv[0];
  KAboutData *aboutData;
  if (argv_0.right(11) == "kinfocenter")
  {
     aboutData = &aboutKInfoCenter;
     KCGlobal::setIsInfoCenter(true);
     kDebug(1208) << "Running as KInfoCenter!\n" << endl;
  }
  else
  {
     aboutData = &aboutKControl;
     KCGlobal::setIsInfoCenter(false);
  }

  
  if (argv_0.right(11) == "kinfocenter")
    aboutData->addAuthor("Helge Deller", I18N_NOOP("Current Maintainer"), "deller@kde.org");
  else
    aboutData->addAuthor("Daniel Molkentin", I18N_NOOP("Current Maintainer"), "molkentin@kde.org");

  aboutData->addAuthor("Matthias Hoelzer-Kluepfel",0, "hoelzer@kde.org");
  aboutData->addAuthor("Matthias Elter",0, "elter@kde.org");
  aboutData->addAuthor("Matthias Ettrich",0, "ettrich@kde.org");
  aboutData->addAuthor("Waldo Bastian",0, "bastian@kde.org");

  KCmdLineArgs::init( argc, argv, aboutData );
  KUniqueApplication::addCmdLineOptions();

  KCGlobal::init();

  if (!KControlApp::start()) {
	kDebug(1208) << "kcontrol is already running!\n" << endl;
	return (0);
  }

  KControlApp app;

  // show the whole stuff
  app.mainWidget()->show();

  return app.exec();
}
